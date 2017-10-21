/* Name:    SignalTower
   Purpose: Object for Skoda SignalTower control.
   Author:  Martijn van den Burg, martijn@[remove-me-first]palebluedot . nl

   Copyright (C) 2016 Martijn van den Burg. All right reserved.

   This program is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Arduino.h"
#include "SignalTower.h"



/*
   Constructor.
   Maps pin numbers to light colour/buzzer connections.
*/
SignalTower::SignalTower(int green, int yellow, int red, int bz) {
  greenLight = green;
  yellowLight = yellow;
  redLight = red;
  buzzer = bz;
  
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(buzzer, OUTPUT);
}


/*
   Destructor
*/
SignalTower::~SignalTower() {
  /* nothing to do */
}


void SignalTower::init() {
  allLightsOff();
  buzzerOff();
}


/* Switch on light/buzzer. */
void SignalTower::redOn() {
  digitalWrite(redLight, HIGH);
}
void SignalTower::yellowOn() {
  digitalWrite(yellowLight, HIGH);
}
void SignalTower::greenOn() {
  digitalWrite(greenLight, HIGH);
}
void SignalTower::buzzerOn() {
  digitalWrite(buzzer, HIGH);
}
void SignalTower::allLightsOn() {
    redOn();
    yellowOn();
    greenOn();
}

/* Toggle light, going up or down on the tower. */
void SignalTower::lightUp() {
  if (greenStatus()) {
    if (yellowStatus()) {
      greenOff();
    }
    else {
      yellowOn();
    }
  }
  else if (yellowStatus()) {
    if (redStatus()) {
      yellowOff();
    }
    else {
      redOn();
    }
  }
  else if (redStatus()) {
    if (yellowStatus()) {
      yellowOff();
    }
    else {
      redOff();
      greenOn();
    }
  }
} // lightUp
void SignalTower::lightDown() {
  if (greenStatus()) {
    if (yellowStatus()) {
      yellowOff();
    }
    else {
      greenOff();
      redOn();
    }
  }
  else if (yellowStatus()) {
    if (greenStatus()) {
      yellowOff();
    }
    else if (redStatus()) {
      redOff();
    }
    else {
      greenOn();
    }
  }
  else if (redStatus()) {
    if (yellowStatus()) {
      redOff();
    }
    else {
      yellowOn();
    }
  }   
} // lightDown

/* Switch off light/buzzer. */
void SignalTower::redOff() {
  digitalWrite(redLight, LOW);
}
void SignalTower::yellowOff() {
  digitalWrite(yellowLight, LOW);
}
void SignalTower::greenOff() {
  digitalWrite(greenLight, LOW);
}
void SignalTower::buzzerOff() {
  digitalWrite(buzzer, LOW);
}
void SignalTower::allLightsOff() {
    redOff();
    yellowOff();
    greenOff();
}


/* Return status of a single light/buzzer */
int SignalTower::redStatus() {
  return digitalRead(redLight);
}
int SignalTower::yellowStatus() {
  return digitalRead(yellowLight);
}
int SignalTower::greenStatus() {
  return digitalRead(greenLight);
}
int SignalTower::buzzerStatus() {
  return digitalRead(buzzer);
}




/* End */
