#include "Arduino.h"
#include "LowPower.h"

class PowerMgr
{
  private:
    int _ledPin;

  public:
    PowerMgr(int ledPin) {
      _ledPin = ledPin;
      pinMode(_ledPin, OUTPUT);
    }

    void update(int pirSensorValue, int machineState) {
      if(pirSensorValue == 0 && machineState == 0)
        {
          digitalWrite(_ledPin, HIGH);
          LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
        }
        digitalWrite(_ledPin, LOW);
    }
};

class PirSensor
{
  private:
    int _pin;
  
  public:
    PirSensor(int pin) {
      pinMode(pin, INPUT_PULLUP);
      _pin = pin;
    }
    
    bool read() {
      return digitalRead(_pin);
    }
};

class LedStrip
{
  private:
    int _pin;

  public:
    LedStrip(int pin) {
      pinMode(pin, OUTPUT);
      _pin = pin;
    }

    void setBrightness(int value) {
      analogWrite(_pin, value);
    }
};

class Dimmer {
  private:
    int brightnessValue;
    bool dimmerActive; // TRUE every 100ms
    bool _motionDetected;
    int tonTimerValue;
    int machineState; // 0: led off // 1: dimming ongoing // 2:led on // 3: dimming offgoing
    
  public:
    int update(bool motionDetected) {
      int moduloResult;
      moduloResult = millis() % 100;
      _motionDetected = motionDetected;
      
      if(moduloResult == 0) {
        SetBrightness();
      }
      
      return brightnessValue;
    }

    int SetBrightness () {
      if(machineState == 0) { // 0: led off
        if(_motionDetected){
          machineState = 1;
        }
      }
            
      if(machineState == 1) { // 1: dimming ongoing
        delay(1);
        brightnessValue = brightnessValue + 10;
        
        if(brightnessValue>=255) {
          brightnessValue=255;
          machineState = 2;
        }
      }

      if(machineState == 2) { // 2:led on
        delay(1);
        tonTimerValue++;
        if(tonTimerValue > 200) {
          machineState = 3;
          tonTimerValue = 0;
        }

        if(_motionDetected){
          tonTimerValue = 0;
        }
      }

      if(machineState == 3) { // 3: dimming offgoing
        delay(1);
        brightnessValue = brightnessValue - 5;
        
        if(brightnessValue<=0) {
          brightnessValue=0;
          machineState = 0;
        }
      }
      
    return brightnessValue;
    }

    int getMachineState() {
      return machineState;
    }
};

/* >>>>> Main program <<<<<< */

PirSensor pirSensor(2);
LedStrip ledStrip(3);
PowerMgr powerMgr(17);
Dimmer dimmer;
bool pirSensorValue;
byte brightness;
int machineState;

void setup() {
  Serial.begin(9600);
}

void loop() {
  pirSensorValue = pirSensor.read();
  brightness = dimmer.update(pirSensorValue);
  ledStrip.setBrightness(brightness);
  machineState = dimmer.getMachineState();

  Serial.print("pirSensorValue: ");
  Serial.print(pirSensorValue);

  Serial.print(" brightness: ");
  Serial.print(brightness);

  Serial.print(" machineState: ");
  Serial.println(machineState);

  powerMgr.update(pirSensorValue, machineState);
}
