/*
 SignalTower.h 
 
 Copyright 2016 Martijn van den Burg, martijn@[remove-me-first]palebluedot . nl
 
 This file is part of the SignalTower library for controlling a Skoda signal
 tower with Digistump Oak.
 
 SignalTower is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.
 
 This software is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
   
   You should have received a copy of the GNU General Public License
   along with SignalTower.  If not, see <http://www.gnu.org/licenses/>.
   
*/


/* This library is written for the Digistump Oak and Arduino software version 1.6.8.
 * 
 * This library may work with other hardware and/or software. YMMV.
 */

#include "Arduino.h"

#ifndef SignalTower_h
#define SignalTower_h

#define VERSION 1.0


class SignalTower {
  public:
    SignalTower(int, int, int, int);
    ~SignalTower();

    void init();

    void redOn();
    void yellowOn();
    void greenOn();
    void buzzerOn();
    void allLightsOn();

    void redOff();
    void yellowOff();
    void greenOff();
    void buzzerOff();
    void allLightsOff();

    int redStatus();
    int yellowStatus();
    int greenStatus();
    int buzzerStatus();

  private:
    int redLight;
    int yellowLight;
    int greenLight;
    int buzzer;
};


#endif
