# Controlling the Skoda signal tower
This hobby project consists of a three-light and buzzer, 12V Skoda signal tower mounted in a standalone box. The signal tower is controlled by a Nodemcu microcontroller, with software written in the Arduino IDE.

The lights on the signal tower can be controlled either by:
- a fitted push button; pushing it briefly switches to the next light, pushing it longer switches to the previous light like so: green, green+yellow, yellow, yellow+red, red. And then back to green.
- a JSON message published on a specific Mosquitto (MQTT) topic.

The buzzer can only be controlled by the same JSON message.

The format of the JSON message is:

> { "colors": {"green":"[on|x]", "yellow":"[on|x], "red":"[on|x"]}, "buzzer":"[on|x]" }

The meaning of "[on|x]" is really "the string 'on', or any other string". Meaning that code only checks for the string "on", anything else is considered "off".

Network credentials should be made available in a file 'credentials.h', looking like this:

> #define SSID "yourssid"
> #define PASSWORD "somepassword"
> #define CONNECTION_ID "MQTTclientID"
> #define CLIENT_NAME "MQTTclientname"
> #define CLIENT_PASSWORD "MQTT password"

## To do
Use an external interrupt with the pushbutton.

# Target systems
The code is written for the NodeMCU (ESP8266). It should be easy to run this on many other devices, provided the libraries that it uses are supported:

- ArduinoJSON
- ESP8266WiFi
- FSM
- PubSubClient

# License
This code is released under the GPL v3.




/* END */

