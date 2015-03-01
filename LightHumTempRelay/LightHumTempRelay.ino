#include <SPI.h>
#include "MySensor.h"  
#include <DHT.h>
#include "BH1750.h"
#include "Wire.h"

#define CHILD_ID_HUM 10
#define CHILD_ID_TEMP 11
#define CHILD_ID_VOLT 12
#define CHILD_ID_LIGHT 13
#define LIGHT_SENSOR_ANALOG_PIN 0
#define HUMIDITY_SENSOR_DIGITAL_PIN 3
#define RELAY_1  4  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay


MySensor gw;
DHT dht;

BH1750 lightSensor;
float lastTemp;
float lastHum;
long lastVolt;
uint16_t lastlux;
unsigned long CYCLE_TIME = 0; // How long do we want to watch once first detected (in milliseconds)  Used to help stop false alarms
unsigned long CYCLE_INTERVAL = 3000; // How long do we want to watch once first detected (in milliseconds)  Used to help stop false alarms
  
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgVolt(CHILD_ID_VOLT, V_VOLTAGE);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);

//int node_id = 21;

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else`
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void setup()  
{ 
  gw.begin(incomingMessage, AUTO, true);
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Humidity&Temp&Volt&Light&Relay", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_VOLT, S_CUSTOM);
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  lightSensor.begin();
  Serial.println("Light initiated");
  
  // Fetch relay status
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    gw.present(sensor, S_LIGHT);
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);   
    // Set relay to last known state (using eeprom storage) 
    digitalWrite(pin, gw.loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
  
  CYCLE_TIME = millis ();
  metric = gw.getConfig().isMetric;
}
 
void loop()      
{  
  if ( (millis () - CYCLE_TIME) >= CYCLE_INTERVAL){
    delay(dht.getMinimumSamplingPeriod());
    //send battery voltage level
    long voltage = readVcc();
    if (isnan(voltage)) {
        Serial.println("Failed reading voltage from battery");
    } else if (voltage != lastVolt) {
        lastVolt = voltage;
  	  gw.send(msgVolt.set(voltage));
    	  Serial.print("V: ");
     	  Serial.println(voltage);
    }
  
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
    
    uint16_t lux = lightSensor.readLightLevel();// Get Lux value
    Serial.print("L: ");
    Serial.println(lux);
    if (lux != lastlux) {
        gw.send(msgLight.set(lux));
        lastlux = lux;
    }
  CYCLE_TIME = millis ();
  }
  
  gw.process();

}

void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     gw.saveState(message.sensor, message.getBool());
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}

