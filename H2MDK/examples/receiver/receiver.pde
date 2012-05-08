/*
remoteMonitor

example program for a remote temperature monitor. We use:

* JeeLib and RF12 radio modules for tx/rx

for more info see the website:
http://www.arcolaenergy.com/h2mdk
*/

#include <JeeLib.h>

typedef struct {
  float celsius;
  float voltage;
  float current;
} 
Payload;

void setup()
{
  Serial.begin(57600);
  Serial.println( "receiver start");
  delay(100);
  rf12_initialize(2, RF12_433MHZ,212);
  Serial.println( "rf12 setup done" );
}

void loop()
{
  if (rf12_recvDone() && rf12_crc == 0 and rf12_len == sizeof(Payload))
  {
    const Payload* p = (const Payload*) rf12_data;
    Serial.println( "got data:" );
    Serial.print( "temp C: " );
    Serial.println(p->celsius);
    Serial.print( "stack V: " );
    Serial.println(p->voltage);
    Serial.print( "stack I: " );
    Serial.println(p->current);    
  }
}




