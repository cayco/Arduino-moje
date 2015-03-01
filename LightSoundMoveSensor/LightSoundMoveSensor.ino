#include <SPI.h>
#include <MySensor.h>  

#define CHILD_ID_LIGHT 0
#define CHILD_ID_MOTION 1   // Id of the sensor child
#define CHILD_ID_SOUND 2   // Id of the sensor child

#define LIGHT_SENSOR_ANALOG_PIN 0
#define DIGITAL_INPUT_SENSOR 2   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define DIGITAL_SOUND_SENSOR A2

unsigned long SEND_FREQUENCY = 20000;           // Minimum time between send (in milliseconds). We don't want to spam the gateway.
unsigned long CYCLE_TIME = 0; // How long do we want to watch once first detected (in milliseconds)  Used to help stop false alarms
unsigned long CYCLE_INTERVAL = 1000; // How long do we want to watch once first detected (in milliseconds)  Used to help stop false alarms
unsigned long SND_TIME = 0; // How long do we want to triggered alarm last
unsigned long SND_ON = 2000;

MySensor gw;
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage msgSound(CHILD_ID_SOUND, V_LIGHT_LEVEL);
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);

int lastLightLevel;
int lastMotionLevel;
int tSoundLevel = 750;
int lastSoundLevel = 0;
int sentSoundLevel = 0;
long tmp = 0;

void setup()  
{ 
  gw.begin(NULL, AUTO, true);
  // Send the sketch version information to the gateway and Controller`
//  pinMode(DIGITAL_SOUND_SENSOR, INPUT); 
  
  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  gw.present(CHILD_ID_MOTION, S_MOTION);
  gw.present(CHILD_ID_SOUND, S_LIGHT_LEVEL);

  CYCLE_TIME = millis ();
  SND_TIME = millis();
}

void loop()      
{ 

  if ( (millis () - CYCLE_TIME) >= CYCLE_INTERVAL) {

    //read light
    int lightLevel = (1023-analogRead(LIGHT_SENSOR_ANALOG_PIN))/10.23; 
    tmp = lightLevel - lastLightLevel;
    if (abs(tmp) > 10) {
      Serial.print("Poziom swiatla: ");
      Serial.println(lightLevel);
      gw.send(msgLight.set(lightLevel));
        lastLightLevel = lightLevel;
    }
    
    // Read digital sound value
    int volume = analogRead(DIGITAL_SOUND_SENSOR);
    Serial.println(volume);
    if (volume < tSoundLevel && lastSoundLevel == 0) { //movement detected, update if there was a change only
        lastSoundLevel = 1;
    } else if (volume > tSoundLevel && lastSoundLevel == 1) {  //movement not detected, update if there was a change only
        lastSoundLevel = 0;
    }
    
    //read sound
    
    if ( sentSoundLevel != lastSoundLevel) {
      Serial.print("Dzwiek: ");
      Serial.println(volume);
      gw.send(msgSound.set(lastSoundLevel));  // Send tripped value to gw
      sentSoundLevel = lastSoundLevel;
    }
     
    CYCLE_TIME = millis ();
   } 

  gw.process();
}
