#include "SPI.h"
#include "MySensor.h"
#include "Wire.h"
#include <Adafruit_BMP085.h>
#include <DHT.h>


#define BARO_CHILD 0
#define TEMP_CHILD 1
#define CHILD_ID_LIGHT 2
#define CHILD_ID_HUM 3
#define CHILD_ID_TEMP 4
#define CHILD_ID_RAIN 5

#define HUMIDITY_SENSOR_DIGITAL_PIN 8
#define RAIN_SENSOR_ANALOG_PIN A1
#define LIGHT_SENSOR_ANALOG_PIN 0

unsigned long SLEEP_TIME = 3000; // Sleep time between reads (in seconds)

Adafruit_BMP085 bmp = Adafruit_BMP085();      // Digital Pressure Sensor 
MySensor gw;
DHT dht;

int lastLightLevel;
int lastRainLevel = 100;

float lastPressure = -1;
float lastTemp = -1;
float lastTemp2 = -1;

long tmp = 0;
float lastHum;


boolean metric; 
MyMessage tempMsg(TEMP_CHILD, V_TEMP);
MyMessage pressureMsg(BARO_CHILD, V_PRESSURE);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgRain(CHILD_ID_RAIN, V_RAIN);

//MyMessage forecastMsg(BARO_CHILD, V_FORECAST);

void setup() {
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Meteo Station", "1.0");


  if (!bmp.begin()) {
   Serial.println("Could not find a valid BMP085 sensor, check wiring!");
   while (1) { }
   Serial.print("Started");
  }

  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Register sensors to gw (they will be created as child devices)
  gw.present(BARO_CHILD, S_BARO);
  gw.present(TEMP_CHILD, S_TEMP);
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_RAIN, S_RAIN);


  metric =  gw.getConfig().isMetric;
}

void loop() {
  //Humidity and temperature DHT-22
  delay(dht.getMinimumSamplingPeriod());
    float temperature = dht.getTemperature();
    if (isnan(temperature)) {
        Serial.println("Failed reading temperature from DHT");
    } else if (temperature != lastTemp) {
      lastTemp = temperature;
      if (!metric) {
        temperature = dht.toFahrenheit(temperature);
      }
      gw.send(msgTemp.set(temperature, 1));
      Serial.print("T: ");
      Serial.println(temperature);
    }
    
    float humidity = dht.getHumidity();
    if (isnan(humidity)) {
        Serial.println("Failed reading humidity from DHT");
    } else if (humidity != lastHum) {
        lastHum = humidity;
        gw.send(msgHum.set(humidity, 1));
        Serial.print("H: ");
        Serial.println(humidity);
    }  
  
  //read light level   
  int lightLevel = (1023-analogRead(LIGHT_SENSOR_ANALOG_PIN))/10.23; 
  tmp = lightLevel - lastLightLevel;
  if (abs(tmp) > 10) {
    Serial.print("Poziom swiatla: ");
    Serial.println(lightLevel);
    gw.send(msgLight.set(lightLevel));
    lastLightLevel = lightLevel;
  }

  //read rain level   
  int rainLevel = (1023-analogRead(RAIN_SENSOR_ANALOG_PIN))/10.23 - 100;
  rainLevel = abs(rainLevel);
  tmp = rainLevel - lastRainLevel;
  if (abs(tmp) > 10) {
    Serial.print("Poziom deszczu: ");
    Serial.println(rainLevel);
    gw.send(msgRain.set(rainLevel));
    lastRainLevel = rainLevel;
  }  
  //pressure
  float pressure = bmp.readSealevelPressure(120)/100; // 205 meters above sealevel
  float temperature2 = bmp.readTemperature();
  
  if (!metric) {
    // Convert to fahrenheit
    temperature2 = temperature2 * 9.0 / 5.0 + 32.0;
  }
  
  //I2C temperature
  Serial.print("Temperature = ");
  Serial.print(temperature2);
  Serial.println(metric?" *C":" *F");
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" Pa");

  if (temperature2 != lastTemp2) {
    gw.send(tempMsg.set(temperature2,1));
    lastTemp2 = temperature2;
  }

  if (pressure != lastPressure) {
    gw.send(pressureMsg.set(pressure, 0));
    lastPressure = pressure;
  }
  
  gw.sleep(SLEEP_TIME);
}



