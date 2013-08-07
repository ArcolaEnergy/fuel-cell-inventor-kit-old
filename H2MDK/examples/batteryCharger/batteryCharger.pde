/*
example program for charging a lead acid battery with a fuel cell stack
 for more info see the website:
 http://www.arcolaenergy.com/h2mdk
 */

//#define SETUP

//user variables
const int chargeLimit = 100; 
const int batteryStartChargeV = 11500; //mv
const int batteryEndChargeV = 13800; //mv
long maxChargeTime = 7200000; //2 hrs expressed in ms. 600000 is 10 minutes expressed in ms



const int knobPin = A0;    // select the input pin for the potentiometer
const int batteryVPin = A4; // A1-A3 are used by shield
#define mosfetPin 9 //3,5 & 6 are used by the library
float batteryV;
boolean charge = false;
long startChargeTime = 0;
long timer;

//_stacksize V1_5W, V3W, V12W or V30W depending on fuelcell
//_shield is the version number printed on the reverse of the shield
#define _stacksize V1_5W
#define _shield V1_2
#include <h2mdk.h>
h2mdk fuelcell; 

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


#ifdef SETUP
  //potentiometer adjustment of current
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

#ifndef SETUP
  //once per second, we check battery status and make decisions about charging it
  if( millis() - timer > 1000 )
  {
    timer = millis();
    int rawBatteryV = analogRead( batteryVPin );
    //these first 2 numbers are total R (in kohms) and lower R of potential divider
    batteryV = (300.00/80.00)*(5000/1024)*rawBatteryV; // R1=226k R2=100k
    Serial.println( charge ? "charging" : "waiting" );
    Serial.print( "battery voltage: " );
    Serial.println( batteryV );

    //logic
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
}
