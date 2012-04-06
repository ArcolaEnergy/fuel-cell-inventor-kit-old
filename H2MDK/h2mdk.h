/*
  ArcolaEnergy h2mdk fuelcell controller for 3, 12 and 30W stacks.
  http://www.arcolaenergy.com/h2mdk

  Copyright (C) 2012  ArcolaEnergy

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef h2mdk_h
#define h2mdk_h

//version types
#define V3W 1
#define V12W 2
#define V30W 3

//timing defs
#define PREPURGE 200 //to ensure we don't purge while shorting!
#define ELECT_INTERVAL 500 //how often to blink the led

#define SUPPLYMV 5040.0 //for better ADC accuracy, measure your 5v and put here in millivolts (.0 is important at end)
#define CAP_V 800

//digital IO pins
#define PURGE 2
#define LOAD 3  // 3w can disconnect load
#define OSC 3   // 12-30w needs oscillator for mosfet charge pump
#define SHORT 4
#define STATUS_LED 5

//analog pins
#define VOLTAGE_SENSE A1
#define CURRENT_SENSE A2
#define CAP_V_SENSE A3

class h2mdk
{
  public:
    h2mdk(int version);
    void poll();
    void status();
    float getVoltage();
    float getCurrent();
    void start();
  private:
    int _version;
    bool _ledstate;
    bool _ni(bool);
    float _current;
    float _voltage;
    void _shortCircuit();
    void _purge();
    void _updateElect();
    void _blink();
    void _checkCaps();
    void _setupTimings(int version);
    //time vars
    unsigned int _shortCircuitInterval;
    unsigned int _shortTime;
    unsigned long _purgeInterval;
    unsigned int _purgeTime;
    //for doing the timing
    unsigned int _shortCircuitTimer;
    unsigned long _purgeTimer;
    unsigned int _statusTimer;
    unsigned int _lastPoll;
};

#endif
