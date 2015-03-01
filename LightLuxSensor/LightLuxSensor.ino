/*
  Vera Arduino BH1750FVI Light sensor
  communicate using I2C Protocol
  this library enable 2 slave device addresses
  Main address  0x23
  secondary address 0x5C
  connect the sensor as follows :

  VCC  >>> 5V
  Gnd  >>> Gnd
  ADDR >>> NC or GND  
  SCL  >>> A5
  SDA  >>> A4
  
  Contribution: idefix
 
*/

#include <SPI.h>
#include <MySensor.h>  

#define CHILD_ID_LIGHT 0
#define LIGHT_SENSOR_ANALOG_PIN 3
unsigned long SLEEP_TIME = 1000; // Sleep time between reads (in milliseconds)

MySensor gw;
MyMessage msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
volatile unsigned long cnt = 0;
unsigned long oldcnt = 0;
unsigned long t = 0;
unsigned long last;

void irq1()
{
  cnt++;
}

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Light Cayco Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  
  pinMode(LIGHT_SENSOR_ANALOG_PIN, INPUT);
  digitalWrite(LIGHT_SENSOR_ANALOG_PIN, HIGH);
  attachInterrupt(1, irq1, CHANGE);
}

void loop()      
{     
//  Serial.print("In da loop "); 
    if (millis() - last >= 1000)
  {
    last = millis();
    t = cnt;
    unsigned long hz = t - oldcnt;
    Serial.print("FREQ: "); 
    Serial.print(hz);
    Serial.print("\t = "); 
    Serial.print((hz+50)/100);  // +50 == rounding last digit
    Serial.println(" mW/m2");
    gw.send(msg.set((hz+50)/100));
    oldcnt = t;
  }
  

//  gw.sleep(SLEEP_TIME);
}
