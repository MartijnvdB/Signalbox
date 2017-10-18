/*
   Name:    signalbox
   Purpose: operate lights of the flexible signal tower with one button or via MQTT.
            implemented with a state machine with the following states:
            - button_up
            - button_down
            - message received
   Author:  MBUR
   Github:
   Date:    started Jul 2017
            Oct 2017, complete rewrite, added MQTT and split of the signal tower hardware
                      in a separate library.

   Uses arduino-fsm to implement a finite state machine.
   Based on arduino-fsm 2.2.0 with pull request #5 9https://github.com/jonblack/arduino-fsm/pull/5)

*/

#include <ArduinoJson.h>
#include <SignalTower.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// http://www.humblecoder.com/arduino-finite-state-machine-library/
// https://github.com/jonblack/arduino-fsm
#include <Fsm.h>
#include "./credentials.h" // Sensitive WiFi and MQTT credentials not stored in GIT.



/*
  To be able to use ESP8266's API, e.g. for SLEEP modes.
  See TestEspApi example sketch in .arduino15
  SDK headers are in C, include them extern or else they will throw an error compiling/linking
  all SDK .c files required can be added between the { }
*/
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


/* Global debug flag */
#define DEBUG 1


/* Digital pin configuration on the NodeMCU */
#define PUSHBUTTON D1   // pushbutton for lamp control connected to D2
#define GREEN_LIGHT D2
#define YELLOW_LIGHT D5
#define RED_LIGHT D6
#define BUZZER D7

/* Button press timimgs */
#define MIN_PRESS_MS 10     // do nothing when pressed less than this (debounce)
#define FORWARD_MS 500      // move light up when pressed 0 < delta <= FORWARD_MS


/* Wireless variables */
int waitCount = 0;      // reconnect logic
int maxWaitCount = 30;  // reconnect logic


/* MQTT/WiFi configurations */
const char* mqtt_server = "192.168.178.190";    // raspberrypi, Domoticz
const unsigned int mqtt_port = 11883;
const char* ssid     = SSID;
const char* password = PASSWORD;

const char* connection_id = CONNECTION_ID;
const char* client_name = CLIENT_NAME;
const char* client_password = CLIENT_PASSWORD;
const char* inTopic = "signaltower/in";
const char* outTopic = "signaltower/out";

const unsigned int reconnect_interval_msec = 5000;
unsigned int prev_connect_attempt = 0;

/*  JSON variables for messages received over MQTT.
    Calculate buffer size: https://bblanchon.github.io/ArduinoJson/assistant/
*/
const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 50;
DynamicJsonBuffer jsonBuffer(bufferSize);

/* FSM variables */
bool curState, prevState;
unsigned long eventStartTime;
unsigned long actionTime = 0;


int lightCode = 0;  // which light(s) are on
byte codeToByteValue[5] = {B01, B11, B10, B110, B100}; // for mapping an integer lightCode value to a bit/byte value
/* lightCode decoding:
    INT BIN colour
    1   001 green
    2   011 green+yellow
    3   010 yellow
    4   011 yellow+red
    5   100 red

   press up:
   0 > 1 > 2 > 3 > 4 > 0

   press down:
   0 > 4 > 3 > 2 > 1 > 0
*/

/*
  State definitions.
  A state has two callback functions associated with it: on_enter and on_exit, which are called when the state is entered into and exited from, respectively.
  Callbacks can be NULL (at least, the second one)
*/
State state_button_up(&on_button_up_enter, &on_button_up_exit);
State state_button_down(&on_button_down_enter, &on_button_down_exit);



/*
   FSM on-enter and on-exit functions.
*/
void on_button_up_enter() {
  if (DEBUG) {
    Serial.println("Entering button UP");
    Serial.print("Time pressed: ");
    Serial.println(actionTime);
  }

  // Determine what to do based on length of button press
  if ( (actionTime > MIN_PRESS_MS) && (actionTime <= FORWARD_MS) ) {
    lightCode = lightUp(lightCode);
  }
  else if ( actionTime > FORWARD_MS ) {
    lightCode = lightDown(lightCode);
  }
  else {
    // debounce, do nothing
  }
  switchLamp(lightCode);
} // on_button_up_enter
void on_button_up_exit() {
}
void on_button_down_enter() {
}
void on_button_down_exit() {
}


/* State transition events */
#define PRESS 10
#define RELEASE 20


Fsm fsm(&state_button_up);
WiFiClient espClient;
PubSubClient client(espClient);


/* Other functions */

void setup() {

  if (DEBUG) {
    Serial.begin(9600);
  }

  pinMode(PUSHBUTTON, INPUT);
  pinMode(GREEN_LIGHT, OUTPUT);
  pinMode(YELLOW_LIGHT, OUTPUT);
  pinMode(RED_LIGHT, OUTPUT);

  prevState = digitalRead(PUSHBUTTON);

  /* Connect to wireless */
  WiFi.begin(ssid, password);




  // This while() makes no sense?
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }




  if (DEBUG) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }


  // Set MQTT parameters
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(parseMQTTMessage);   // callback executed when message received


  /* Define state transitions
     void add_transition(State* state_from, State* state_to, int event, void (*on_transition)());
     void add_timed_transition(State* state_from, State* state_to, unsigned long interval, void (*on_transition)());
     --> Last argument may be NULL
  */
  fsm.add_transition(&state_button_up, &state_button_down, PRESS, resetCounter);
  fsm.add_transition(&state_button_down, &state_button_up, RELEASE, setCounter);

  switchLamp(lightCode);  // switch on green light

} // setup


/* reset the counter that we use to keep track of how long an press event lasts. */
void resetCounter() {
  actionTime = millis();
}

/* set the counter that we use to keep track of how long an press event lasts. */
void setCounter() {
  actionTime = abs(millis() - actionTime);  // abs() protects against neg. numbers in case of counter rollover during press
}

/* return next light code, moving up the tower. */
int lightUp(int curlightCode) {
  if (curlightCode == 4) {
    return 0;
  }
  return (++curlightCode);
}

/* return next light code, moving down the tower. */
int lightDown(int curlightCode) {
  if (curlightCode == 0) {
    return 4;
  }
  return (--curlightCode);
}


/* Parse the incoming message to determine light/buzzer commands.
   Structure should be like this:
  {
  "colors":{
   "green":"on",
   "yellow":"on",
   "red":"on"
   },
  "buzzer":"on"
  }
*/
void parseMQTTMessage(char* topic, byte* payload, unsigned int length) {
  char json[80];
  int locallightCode = 0;

  DEBUG && Serial.println("Message received.");
  
  // Ready input byte array, store in char array.
  for (int i = 0; i < length; i++) {
    json[i] = (char)payload[i];
  }
  JsonObject& root = jsonBuffer.parseObject(json);

  // Check the root element.
  if (!root.success()) {
    // ParseObject() failed
    return;
  }

  DEBUG && Serial.println("Parsing succeeded.");

  JsonArray& colors = root["colors"];
  const char* colors_green = colors[0]; // "on"
  const char* colors_yellow = colors[1]; // "on"
  const char* colors_red = colors[2]; // "on"
  const char* buzzer = root["buzzer"]; // "on"

  DEBUG && Serial.println("Variable assignment succeeded.");

  // Set the value of 'lightcode', then switch the lamps with it.
  if ( strcmp(colors_green, "on") == 0) {
//    locallightCode |= B00000001;
  }
  if ( strcmp(colors_yellow, "on") == 0) {
//    locallightCode |= B10;
  }
  if ( strcmp(colors_red, "on") == 0) {
//    locallightCode |= B100;
  }

  DEBUG && Serial.print("Going to switch the lamp now with code ");
  DEBUG && Serial.println(locallightCode);

//  switchLamp(locallightCode);  // switch lights


} // parseMQTTMessage



16 Oct 2017
switchlamp() is niet compatible met de JSON input manier. Hier moet iets aan gedaan worden.




/* Switch the relevant lamp(s) on by bitmasking the value of codeToByteValue[] */
void switchLamp(int position) {
  digitalWrite(RED_LIGHT, (codeToByteValue[position] & B00000100) );
  digitalWrite(YELLOW_LIGHT, (codeToByteValue[position] & B00000010) );
  digitalWrite(GREEN_LIGHT, (codeToByteValue[position] & B00000001) );
}



/* Connect to MQTT
  Returns 1 for success, 0 for failure.
*/
int reconnect() {
  // Attempt to connect
  if (client.connect(connection_id, client_name, client_password)) {
    // Once connected, publish an announcement...
    client.publish(outTopic, "Signaltower reconnected");
    // ... and resubscribe to topic where commands are published.
    client.subscribe(inTopic);
    return 1;
  }
  return 0;
}




void loop() {
  /* The program needs to do nothing except controlling the lamp,
      so instead of using an external interrupt to control the FSM,
      just keep reading the input.
  */
  curState = digitalRead(PUSHBUTTON);
  if (curState != prevState) {
    prevState = curState;
    curState == 1 ? fsm.trigger(PRESS) : fsm.trigger(RELEASE);   // switch is NC, opening it makes PUSHBUTTON go high
  }

  // Connect to MQTT topic, but only once every reconnect_interval_msec.
  if (!client.connected() && (millis() - prev_connect_attempt > reconnect_interval_msec) ) {
    prev_connect_attempt = millis();
    DEBUG && Serial.print(client_name);
    DEBUG && Serial.println(" not connected");
    reconnect();
  }
  client.loop();    // start listening to messages


} // loop


/* END */

