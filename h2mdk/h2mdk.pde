/* Horizon suggested short/purge schedule:

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

#include <TimedAction.h>

//version types
#define V3W 1
#define V12W 2
#define V30W 3
//set your version here:
#define VERSION V30W

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
//#define CAP_V_SENSE A4 //test board
//#define VOLTAGE_SENSE A5 //test board

//timing defs
#define SHORTDEL 100
#define PURGEDEL 100
#define PREPURGE 200 //to ensure we don't purge while shorting!

const int shortCircuitInterval = 10;  // seconds
const int purgeInterval = 15; // seconds

boolean LEDSTATE = true;

TimedAction statusAction = TimedAction(500,status);
TimedAction shortAction = TimedAction(shortCircuitInterval*1000,shortC);
TimedAction purgeAction = TimedAction(purgeInterval*1000,purge);


void setup()
{
  Serial.begin(9600);
  Serial.print( "ArcolaEnergy fuel cell controller for " );
  #if VERSION == V3W
    Serial.println( "3W" );
  #elif VERSION == V12W
    Serial.println( "12W" );
  #elif VERSION == V30W
    Serial.println( "30W" );
  #endif

  Serial.print("Short-circuit: ");
  Serial.print(SHORTDEL);
  Serial.print("ms every ");
  Serial.print(shortCircuitInterval);
  Serial.println(" seconds");
  
  Serial.print("Purge: ");
  Serial.print(PURGEDEL);
  Serial.print("ms every ");
  Serial.print(purgeInterval);
  Serial.println(" seconds");
  
  pinMode(STATUS_LED, OUTPUT);

  #if VERSION == V3W
    pinMode( LOAD, OUTPUT );
    digitalWrite( LOAD, ni(LOW) );
  #elif VERSION == V12W 
  //charge pump waveform
    analogWrite( OSC, 128 );
  #elif VERSION == V30W
    //charge pump waveform
    analogWrite( OSC, 128 );
  #endif
  
  pinMode( SHORT, OUTPUT );
  digitalWrite( SHORT, ni(LOW) ); 
  pinMode( PURGE, OUTPUT );
  digitalWrite( PURGE, ni(LOW) );

  //wait for cap to charge
  Serial.println( "waiting for caps to charge" );
  checkCaps();

}

//utility to invert the mosfet pins for the 12 and 30W control boards
boolean ni(boolean state)
{
  #if VERSION == V3W
    return state;
  #elif VERSION == V12W
    return ! state;
  #elif VERSION == V30W
    return ! state;
  #endif
}

void loop()
{
  //just loop until we need to take necessary action
  statusAction.check();
  shortAction.check();
  purgeAction.check();
}

void checkCaps()
{
  while( analogRead( CAP_V_SENSE ) < CAP_V )
  {
    Serial.println( analogRead( CAP_V_SENSE ) );
    blink();
    delay(100);
  }
  Serial.println( "CHARGED" );
}

void blink()
{
  digitalWrite(STATUS_LED, LEDSTATE );
  LEDSTATE = ! LEDSTATE;
}
void status()
{
  blink();
  printElect();
}

void printElect()
{
  float rawStackV = analogRead(VOLTAGE_SENSE );
  float stackVoltage = (326.00/100.00)*(SUPPLYMV/1024*rawStackV); // R1=226k R2=100k
  Serial.print(stackVoltage/1000);
  Serial.print( "V, " );

  float rawCurrent = analogRead(CURRENT_SENSE );
  float currentMV = (SUPPLYMV / 1024 ) * rawCurrent;
  float currentA = ( currentMV - SUPPLYMV / 2 ) / 185; //185mv per amp
  Serial.print( currentA );
  Serial.println( "A" );
}

void purge()
{
  delay( PREPURGE );
  Serial.println("PURGE");
  digitalWrite( PURGE, ni(HIGH) );
  delay( PURGEDEL);
  digitalWrite( PURGE, ni(LOW) );
}

void shortC()
{
  if (analogRead( CAP_V_SENSE ) < CAP_V) 
  {
    Serial.println("SKIPPING SHORT-CIRCUIT AS SUPERCAP VOLTAGE TOO LOW");
    return;
  }

  #if VERSION == V3W
    //disconnect load
    digitalWrite( LOAD, ni(LOW) );
  #endif

  //short circuit
  digitalWrite( SHORT, ni(HIGH) );
  Serial.println("SHORT-CIRCUIT");
  delay(SHORTDEL);
  digitalWrite( SHORT, ni(LOW) );
  
  #if VERSION == V3W
    //reconnect
    digitalWrite( LOAD, ni(HIGH) );
  #endif 
}
