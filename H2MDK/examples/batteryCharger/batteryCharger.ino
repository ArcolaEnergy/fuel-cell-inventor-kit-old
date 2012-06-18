/*
example program for charging a lead acid battery with a fuel cell stack
 for more info see the website:
 http://www.arcolaenergy.com/h2mdk
 */

//todo - fix current sense -it's way off.

#define SETUP

const int knobPin = A0;    // select the input pin for the potentiometer
const int batteryVPin = A4; // A1-A3 are used by shield
#define mosfetPin 6 //2 - 5 are used by the library
#include <h2mdk.h>
const int chargeLimit = 100; 
float batteryV;
const int batteryStartChargeV = 11500; //mv
const int batteryEndChargeV = 14500; //mv
boolean charge = false;
long startChargeTime = 0;
long maxChargeTime = 600000; //10 minutes expressed in ms
long timer;

//pass V1_5W, V3W, V12W or V30W depending on your board and fuelcell
//for greater accuracy set second argument to arduino's internal band gap voltage in mv
//see http://bit.ly/KQmkma on how to measure this
h2mdk fuelcell(V30W); 

void setup()
{
  pinMode(mosfetPin, OUTPUT);

  analogWrite(mosfetPin, 0 ); //don't charge to start with
  Serial.begin(9600);
  //this blocks until caps are charged
  fuelcell.start();
}

void loop()
{
  //this takes care of short circuit, purging and updating electrical values
  fuelcell.poll();

#ifndef SETUP
  //once per second
  if( millis() - timer > 1000 )
  {
    timer = millis();
    int rawBatteryV = analogRead( batteryVPin );
    //these first 2 numbers are total R (in kohms) and lower R of potential divider
    batteryV = (299.00/80.00)*(fuelcell.getSupplyMV()/1024*rawBatteryV); // R1=226k R2=100k
    Serial.println( charge ? "charging" : "waiting" );
    Serial.print( "battery voltage: " );
    Serial.println( batteryV );
    if( charge == false && batteryV < batteryStartChargeV )
    {
      charge = true;
      startChargeTime = millis();
      analogWrite( mosfetPin, chargeLimit );
    }
    if( charge == true && batteryV > batteryEndChargeV )
    {
      Serial.println( "finished charging battery: battery level OK" );
      charge = false;
      analogWrite( mosfetPin, 0 );
    }
    /* Serial.print( millis() - startChargeTime );
     Serial.print( " " );
     Serial.println( maxChargeTime );*/
    if( charge == true && ( millis() - startChargeTime ) > maxChargeTime )
    {
      Serial.println( "finished charging battery: max charge time reached" );
      charge = false;
      analogWrite( mosfetPin, 0 );
    }
  }
#endif
#ifdef SETUP
  if( millis() - timer > 1000 )
  {
    timer = millis();
    //use a knob to set charge current
    int knobPos = analogRead(knobPin);
    int pwm = map( knobPos, 0, 1023, 0, 255 );
    analogWrite( mosfetPin, pwm );
    Serial.print( "charge PWM: " ); 
    Serial.println( pwm, DEC );
  }
#endif

}




