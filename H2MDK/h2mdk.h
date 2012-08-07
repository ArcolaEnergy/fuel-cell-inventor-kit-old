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

#if ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

//stack version types
#define V1_5W 1
#define V3W 2
#define V12W 3
#define V30W 4
//hardware version types
#define V1 1
#define V2 2

class h2mdk
{
  public:
    void poll();
    void status();
    float getVoltage();
    float getCurrent();
    void start();
  private:
    int _vccRead();
    bool _ledstate;
    bool _ni(bool);
    float _current;
    float _voltage;
    void _shortCircuit();
    void _purge();
    void _updateElect();
    void _blink();
    void _checkCaps();

//timing defs
    static const int PREPURGE = 200; //to ensure we don't purge while shorting!
    static const int ELECT_INTERVAL = 400; //how often to sample electrical
    static const int BLINK_INTERVAL = 500; //how often to sample electrical

    static const float FILTER = 0.9; //coefficient for LPF on current sense
//band gap can be measured like this: http://bit.ly/KQmkma
    static const int DEFAULTBANDGAP = 1100; //for better ADC accuracy, pass your bandgap to constructor
    static const int CAP_V = 800;

//digital IO pins
  #if( _hardware == V1 )
    static const int PURGE = 2;
    static const int LOAD = 3;  // 3w can disconnect load
    static const int OSC = 3;   // 12-30w needs oscillator for mosfet charge pump
    static const int SHORT = 4;
    static const int STATUS_LED = 5;
  #elif( _hardware == V2 )

  #endif
//analog pins
    static const int VOLTAGE_SENSE = A1;
    static const int CURRENT_SENSE = A2;
    static const int CAP_V_SENSE = A3;

/*
1.5W stack (info from Horizon)
Purge: 100ms every 4 mins
Short circuit: 100ms every 10s

H-12 (no info from Horizon as they don't have H-12 controller)
Purge: Shall we assume 50ms every 25s (because H2 consumption is less)
Short circuit: 100ms every 10s (assuming same as other stacks)

H-30 (info from Horizon)
Purge: 50ms every 10s
Short circuit: 100ms every 10s
*/
//time vars
  // all in ms
  #if( _version == V1_5W )
    static const unsigned int _shortCircuitInterval = 10000;
    static const unsigned int _shortTime = 100;
    static const unsigned long _purgeInterval = 60000; //240000;
    static const unsigned int _purgeTime = 100;

  #elif( _version == V3W )
    static const unsigned int _shortCircuitInterval = 10000;
    static const unsigned int _shortTime = 100;
    static const unsigned long _purgeInterval = 60000; //240000;
    static const unsigned int _purgeTime = 100;

  #elif( _version == V12W )
    static const unsigned int _shortCircuitInterval = 10000;
    static const unsigned int _shortTime = 100;
    static const unsigned long _purgeInterval = 25000;
    static const unsigned int _purgeTime = 50;

  #elif( _version == V30W )
    static const unsigned int _shortCircuitInterval = 10000;
    static const unsigned int _shortTime = 100;
    static const unsigned long _purgeInterval = 10000;
    static const unsigned int _purgeTime = 50;
  #else
  #endif

//for doing the timing
    unsigned int _shortCircuitTimer;
    unsigned int _electTimer;
    unsigned long _purgeTimer;
    unsigned int _statusTimer;
    unsigned long _lastPoll;
    float _filteredRawCurrent;
  #ifdef BANDGAP
    static const int _bandGap = BANDGAP;
  #else
    static const int _bandGap = DEFAULTBANDGAP;
  #endif
};





void h2mdk::start()
{
  _shortCircuitTimer = 0;
  _purgeTimer = 0;
  _electTimer = 0;
  _statusTimer = 0;

  _filteredRawCurrent = 512; //should be at zero A (ie 2.5v) to start with

  pinMode(STATUS_LED, OUTPUT);

  if( _version == V3W || _version == V1_5W )
  {
    pinMode( LOAD, OUTPUT );
    //connect the load to charge the caps
    digitalWrite( LOAD, _ni(HIGH) );
  }
  else if( _version == V12W || _version == V30W )
  {
  //charge pump waveform
    analogWrite( OSC, 128 );
  }

  pinMode( SHORT, OUTPUT );
  digitalWrite( SHORT, _ni(LOW) );
  pinMode( PURGE, OUTPUT );
  digitalWrite( PURGE, _ni(LOW) );

  Serial.println( "ArcolaEnergy fuel cell controller for "
  #if( _version == V1_5W )
    "1.5W"
  #elif( _version == V3W )
    "3W"
  #elif( _version == V12W )
    "12W"
  #elif( _version == V30W )
    "30W"
  #else
  #endif
  );

  Serial.print("Short-circuit: ");
  Serial.print(_shortTime);
  Serial.print("ms every ");
  Serial.print(_shortCircuitInterval / 1000);
  Serial.println(" s");

  Serial.print("Purge: ");
  Serial.print(_purgeTime);
  Serial.print("ms every ");
  Serial.print( _purgeInterval / 1000);
  Serial.println(" s");

  //wait for cap to charge
  Serial.println( "waiting for caps to charge" );
  _checkCaps();
}
float h2mdk::getVoltage()
{
  return _voltage;
}

float h2mdk::getCurrent()
{
  return _current;
}

void h2mdk::status()
{
  Serial.print( _voltage );
  Serial.print( "V, " );
  Serial.print( _current );
  Serial.println( "A" );
}

void h2mdk::poll()
{
  int interval = millis() - _lastPoll;
  _lastPoll = millis();

  _electTimer += interval;
  _statusTimer += interval;
  _purgeTimer += interval;
  _shortCircuitTimer += interval;

  _lastPoll = millis();

  if( _statusTimer > BLINK_INTERVAL )
  {
    _blink();
    status();
    _statusTimer = 0;
  }
  if( _electTimer > ELECT_INTERVAL )
  {
    _updateElect();
    _electTimer = 0;
  }
  if( _shortCircuitTimer > _shortCircuitInterval )
  {
    _shortCircuit();
    _shortCircuitTimer = 0;
  }
  if( _purgeTimer > _purgeInterval )
  {
    _purge();
    _purgeTimer = 0;
  }

}

void h2mdk::_checkCaps()
{
  while( analogRead( CAP_V_SENSE ) < CAP_V )
  {
    Serial.println( analogRead( CAP_V_SENSE ) );
    _blink();
    delay(200);
  }
  Serial.println( "CHARGED" );
}

void h2mdk::_updateElect()
{
  //read this to get more accurate ADC readings. As load increases, supplyMV drops. So we read the supplyMV before doing our other measurements.
  float supplyMV = _vccRead();

  //voltage
  float rawStackV = analogRead(VOLTAGE_SENSE );
  float stackVoltage;
  if( _version == V3W || _version == V1_5W )
    stackVoltage = (supplyMV/1024*rawStackV);
  //for 12 and 30W
  else if( _version == V12W || _version == V30W )
    stackVoltage = (326.00/100.00)*(supplyMV/1024*rawStackV); // R1=226k R2=100k

  _voltage = stackVoltage/1000;

  //current
  //100 times average of current.
  _filteredRawCurrent = _filteredRawCurrent * FILTER  + ( 1 - FILTER ) * analogRead(CURRENT_SENSE);
  float currentMV = (supplyMV / 1024 ) * _filteredRawCurrent;
  if( _version == V3W || _version == V1_5W )
    //current sense chip is powered from 5v regulator
    _current = ( currentMV - 5040 / 2 ) / 185; //185mv per amp
  else if( _version == V12W || _version == V30W )
    //current sense chip is powered by arduino supply
    _current = ( currentMV - supplyMV / 2 ) / 185; //185mv per amp

}


void h2mdk::_purge()
{
  delay( PREPURGE );
  Serial.println("PURGE");
  digitalWrite( PURGE, _ni(HIGH) );
  delay( _purgeTime);
  digitalWrite( PURGE, _ni(LOW) );
}

void h2mdk::_shortCircuit()
{
  if (analogRead( CAP_V_SENSE ) < CAP_V)
  {
    Serial.println("SKIPPING SHORT-CIRCUIT AS SUPERCAP VOLTAGE TOO LOW");
    return;
  }

  if( _version == V3W || _version == V1_5W )
    //disconnect load
    digitalWrite( LOAD, _ni(LOW) );

  //short circuit
  digitalWrite( SHORT, _ni(HIGH) );
  Serial.println("SHORT-CIRCUIT");
  delay(_shortTime);
  digitalWrite( SHORT, _ni(LOW) );

  if( _version == V3W || _version == V1_5W )
    //reconnect
    digitalWrite( LOAD, _ni(HIGH) );
}

//utility to invert the mosfet pins for the 12 and 30W control boards
inline bool h2mdk::_ni(bool state)
{
  if( _version == V3W || _version == V1_5W )
    return state;
  if( _version == V12W || _version == V30W )
    return ! state;
}

//blink the status led
void h2mdk::_blink()
{
  digitalWrite(STATUS_LED, _ledstate );
  _ledstate = ! _ledstate;
}


//measure VCC
//needs accurate bandgap
//thanks to JeeLabs for this
//http://jeelabs.org/2012/05/04/measuring-vcc-via-the-bandgap/
int h2mdk::_vccRead()
{
  analogRead(6);
  bitSet(ADMUX,3);
  delayMicroseconds(550);
  bitSet(ADCSRA, ADSC );
  while( bit_is_set(ADCSRA,ADSC));
  word x = ADC;
  return x ? (_bandGap * 1023L) / x : -1;
}

#endif

