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
#define V1_5W 1
#define V3W 2
#define V12W 3
#define V30W 4

//timing defs
#define PREPURGE 200 //to ensure we don't purge while shorting!
#define ELECT_INTERVAL 100 //how often to sample electrical 
#define BLINK_INTERVAL 500 //how often to sample electrical 

#define FILTER 0.8 //coefficient for LPF on current sense
//band gap can be measured like this: http://bit.ly/KQmkma
#define DEFAULTBANDGAP 1100 //for better ADC accuracy, pass your bandgap to constructor
#define CAP_V 2900 //this will need a potential divider

//digital IO pins
#define PURGE 3
#define LOAD 4  // 3w can disconnect load
#define OSC 4   // 12-30w needs oscillator for mosfet charge pump
#define SHORT 5
#define STATUS_LED 6

//analog pins
#define VOLTAGE_SENSE A1
#define CURRENT_SENSE A2
#define CAP_V_SENSE A3

const float _aRef = 3300.0;
class h2mdk
{
  public:
    h2mdk(int version, int bandGap);
    h2mdk(int version);
    void poll();
    void status();
    float getVoltage();
    float getSupplyMV();
    float getCurrent();
    void start();
  private:
    void _init(int version, int bandGap );
    int _vccRead();
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
    void _overloadCutout();
    //time vars
    unsigned int _shortCircuitInterval;
    unsigned int _shortTime;
    unsigned long _purgeInterval;
    unsigned int _purgeTime;
    //for doing the timing
    unsigned int _shortCircuitTimer;
    unsigned int _electTimer;
    unsigned long _purgeTimer;
    unsigned int _statusTimer;
    unsigned long _lastPoll;
    float _filteredRawCurrent;
    int _bandGap;
    float _cutoutVoltage;
    float _supplyMV;
};

#endif
