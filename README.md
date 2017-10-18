# Controlling the Skoda signal tower
This hobby project consists of a three-light and buzzer, 12V Skoda signal tower mounted in a standalone box. The signal tower is controlled by a Nodemcu microcontroller, with software written in the Arduino IDE.

The lights on the signal tower can be controlled either by:
- a fitted push button; pushing it briefly switches to the next light, pushing it longer switches to the previous light like so: green, green+yellow, yellow, yellow+red, red. And then back to green.
- a JSON message published on a specific Mosquitto (MQTT) topic.

The buzzer can only be controlled by the JSON message.

# Target systems
The code is written for the NodeMCU. It should be possible to run this on many other devices as well provided the libraries that it uses are supported:

- ArduinoJSON
- ESP8266WiFi
- FSM
- PubSubClient

# License
This code is released under the GPL v3.
