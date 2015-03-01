#include <SPI.h>
#include <MySensor.h>  
//#include <DHT.h>  
#include "dht.h"

#define CHILD_ID_HUM 5
#define CHILD_ID_TEMP 2
#define HUMIDITY_SENSOR_DIGITAL_PIN A0
unsigned long SLEEP_TIME = 1000; // Sleep time between reads (in milliseconds)

MySensor gw;
dht DHTT;
float lastTemp;
float lastHum;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);


void setup()  
{ 
  gw.begin();
//  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Humidity", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  metric = gw.getConfig().isMetric;
}

void loop()      
{  
  //delay(dht.getMinimumSamplingPeriod());
  delay(700);
  DHTT.read11(HUMIDITY_SENSOR_DIGITAL_PIN);
/*  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
  
  */
    
    Serial.print("T: ");
//    Serial.println(temperature);
    Serial.println(DHTT.temperature);
//  }
  /*
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }
*/
     Serial.print("Current humidity = ");
    Serial.print(DHTT.humidity);
    Serial.print("% ");
  gw.sleep(SLEEP_TIME); //sleep a bit
}


