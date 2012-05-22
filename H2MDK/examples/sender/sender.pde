/*
remoteMonitor

example program for a remote temperature monitor. We use:

* h2mdk for managing fuel cell
* JeeLib and RF12 radio modules for tx/rx
* Dallas DS18B20 one wire temperature sensor

for more info see the website:
http://www.arcolaenergy.com/h2mdk
*/

//for fuel cell power
#include <h2mdk.h>
h2mdk fuelcell(V3W); 

//for radio
#include <JeeLib.h>
MilliTimer sendTimer;
byte needToSend;

//for one wire temperature
#include <OneWire.h>
OneWire  ds(8);  // on pin 8

#define LED 9

//payload def
typedef struct {
  float celsius;
  float voltage;
  float current;
} 
Payload;

Payload payload;

void setup()
{
  Serial.begin(57600);
  pinMode(LED,OUTPUT);
  //this blocks until caps are charged
  fuelcell.start();

  Serial.println( "transmitter start");
  delay(100);
  rf12_initialize(1, RF12_433MHZ,212);
  Serial.println( "rf12 setup done" );
}

void loop()
{
  fuelcell.poll();
  rf12_recvDone(); //needs to be called often

  //send data once 
  if (sendTimer.poll(5000))
  {
    checkTemp();
    payload.voltage = fuelcell.getVoltage();
    payload.current = fuelcell.getCurrent();
    needToSend = 1;
   
    Serial.println( "data ready to send:" );
   
    Serial.print( "temp C: " );
    Serial.println(payload.celsius);
    Serial.print( "stack V: " );
    Serial.println(payload.voltage);
    Serial.print( "stack I: " );
    Serial.println(payload.current);  
  }

  if (needToSend && rf12_canSend())
  {
    digitalWrite(LED,HIGH);
    needToSend = 0;
    //broadcast
    rf12_sendStart(0, &payload, sizeof payload);
    Serial.println("sent");
    delay(100);
    digitalWrite(LED,LOW);
  }
}


