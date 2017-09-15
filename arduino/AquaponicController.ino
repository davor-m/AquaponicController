/*
  Aquaponic Controller
  Copyright (C) 2016 Davor Marjanović <aquaponic.controller@gmail.com>
  Licence: GNU GENERAL PUBLIC LICENSE
*/

#include <TimeLib.h>

class AqTime
{
    const unsigned long _def_time = 1475164800;
                                  //1475305200; // 1.10.2016
  public:
    boolean dateSet = false;

    AqTime(){
      setTime(_def_time);
      setSyncProvider(requestSync);
      setSyncInterval(60);// one minute
    }

    void processSyncMessage() {
       unsigned long pctime;
    
       pctime = Serial.parseInt();
       if( pctime >= _def_time) {
         setTime(pctime);
         dateSet = true;
         setSyncInterval(86400);// one day
       }
    }

    unsigned long addHours(unsigned long startTime, int hours){
      return startTime + (hours * 60 * 60);
    }

    unsigned long addMinutes(unsigned long startTime, int minutes){
      return startTime + (minutes * 60);
    }

    String timeDisplay(){
      // digital clock display of the time
      Serial.print(hour());
      printDigits(minute());
      printDigits(second());
      Serial.print(" ");
      Serial.print(day());
      Serial.print(" ");
      Serial.print(month());
      Serial.print(" ");
      Serial.print(year()); 
      Serial.println(); 
    }
    
    void printDigits(int digits){
      // utility function for digital clock display: prints preceding colon and leading 0
      Serial.print(":");
      if(digits < 10)
        Serial.print('0');
      Serial.print(digits);
    }

};

AqTime aTime;

time_t requestSync(){
  Serial.write("T");
  return 0;
}

/*
 * Water Flow sensor
 */
class FlowSensor
{
    int _interrupt, _pin;
    float _calibrationFactor = 4.5;
    unsigned int _flowMilliLitres;
    unsigned long _oldTime;
  public:
    volatile byte pulseCount;
    float flowRate;
    unsigned long totalMilliLitres;

    FlowSensor(int pin, float factor){
      _pin = pin;
      _interrupt = digitalPinToInterrupt(_pin);
      pinMode(_pin, INPUT_PULLUP);
      _calibrationFactor = factor;
      pulseCount        = 0;
      flowRate          = 0.0;
      totalMilliLitres  = 0;
      _flowMilliLitres  = 0;
      _oldTime          = 0;
      attachInterrupt(_interrupt, pulseCounter, FALLING);
    }

    void processCounter(){
      detachInterrupt(_interrupt);
      flowRate = ((1000.0 / (millis() - _oldTime)) * pulseCount) / _calibrationFactor;
      _oldTime = millis();
      _flowMilliLitres = (flowRate / 60) * 1000;
      totalMilliLitres += _flowMilliLitres;

      pulseCount = 0;
      attachInterrupt(_interrupt, pulseCounter, FALLING);
    }

    void resetCycle(){
      totalMilliLitres = 0;
    }
};

FlowSensor flowSensor(2, 4.5);

void pulseCounter(){
  // Increment the pulse counter
  flowSensor.pulseCount++;
}

/* Aquaponic Settings
 * Load and save settings to the EEPROM
 */
#include <EEPROM.h>

class Settings
{
  int startAddress, version;
  public:
  int  litersPerCycle, pauseCycle, lightStartHour, lightEndHour, lightMinimum, airPumpConnected, fanConnected, temperatureMax;
  
  Settings (int v, int start){
    startAddress = start;
    version = v;
    litersPerCycle = 50;
    pauseCycle = 10;
    lightStartHour = 10;
    lightEndHour = 20;
    lightMinimum = 70;
    airPumpConnected = 0;
    fanConnected = 0;
    temperatureMax = 30;
  }

  void load() {
    if (EEPROM.read(startAddress) == version){
      litersPerCycle    = EEPROM.read(startAddress+1);
      pauseCycle        = EEPROM.read(startAddress+2);
      lightStartHour    = EEPROM.read(startAddress+3);
      lightEndHour      = EEPROM.read(startAddress+4);
      lightMinimum      = EEPROM.read(startAddress+5);
      airPumpConnected  = EEPROM.read(startAddress+6);
      fanConnected      = EEPROM.read(startAddress+7);
      temperatureMax    = EEPROM.read(startAddress+8);
    }
  }
  
  void save() {
    EEPROM.write(startAddress, version);
    EEPROM.write(startAddress+1, litersPerCycle);
    EEPROM.write(startAddress+2, pauseCycle);
    EEPROM.write(startAddress+3, lightStartHour);
    EEPROM.write(startAddress+4, lightEndHour);
    EEPROM.write(startAddress+5, lightMinimum);
    EEPROM.write(startAddress+6, airPumpConnected);
    EEPROM.write(startAddress+7, fanConnected);
    EEPROM.write(startAddress+8, temperatureMax);
  }
};

/*
  Ambient sensors
  Setting up and reading sensors
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

class Ambient
{
    int _tempWaterPin, _dhtPin, _lightPin, _lightSysPin, _lightIndex = 0, _arrSize = 10;
    float _lightArr[10] = {0,0,0,0,0,0,0,0,0,0}, _lightSysArr[10] = {0,0,0,0,0,0,0,0,0,0}, _lightBuffer;
    OneWire _tempWire;
    DallasTemperature _tempSensor;
    DHT _dht;

  public:
    float tempWater, temp, hum, hi, light, lightSys;

    Ambient(int tempWaterPin, int dhtPin, int lightP, int lightSysP):_tempWire(tempWaterPin),_tempSensor(&_tempWire),_dht(dhtPin, DHT11){
      _tempWaterPin = tempWaterPin;
      _dhtPin = dhtPin;
      _dht.begin();
      _lightPin = lightP;
      _lightSysPin = lightSysP;
    }

    void read(){
      _tempSensor.requestTemperatures();
      tempWater = _tempSensor.getTempCByIndex(0);
      temp = _dht.readTemperature();
      hum  = _dht.readHumidity();
      hi = _dht.computeHeatIndex(temp, hum, false);

      _lightArr[_lightIndex] = analogRead(_lightPin) * (100.0 / 1024.0);
      light = averageArr(_lightArr);
      _lightSysArr[_lightIndex] = analogRead(_lightSysPin) * (100.0 / 1024.0);
      lightSys = averageArr(_lightSysArr);
      _lightIndex++;
      if(_lightIndex == 10)
        _lightIndex = 0;
    }

    float averageArr(float arr[]){
      float sum = 0;
      for (int i = 0; i < _arrSize; i++)
        sum += arr[i];
      return sum / _arrSize;
    }
};

class Control
{
    int _pumpPin, _lightPin, _airPumpPin, _fanPin;
  public:
    boolean pumpState = false, lightState = false, airPumpState = false, fanState = false, lightPeriod = false;
    unsigned long flowPauseEnd, lightLastChange;

    Control(int pumpPin, int lightPin, int airPumpPin, int fanPin){
      _pumpPin = pumpPin;
      _lightPin = lightPin;
      _airPumpPin = airPumpPin;
      _fanPin = fanPin;
      pinMode(_pumpPin, OUTPUT);
      pinMode(_lightPin, OUTPUT);
      pinMode(_airPumpPin, OUTPUT);
      pinMode(_fanPin, OUTPUT);
    }

    void setPump(boolean state){
      if (state){
        if (!pumpState){
          digitalWrite(_pumpPin, HIGH);
          pumpState = true;
          flowSensor.resetCycle();
          Serial.print("{M5}");//Pump ON
        }
      }else if (!state){
        if (pumpState){
          digitalWrite(_pumpPin, LOW);
          pumpState = false;
          flowPauseEnd = aTime.addMinutes(now(), pauseCycle)
          Serial.print("{M6}");//Pump OFF
        }
      }
    }

    void setLight(boolean state){
      if (state){
        if (!lightState){
          digitalWrite(_lightPin, HIGH);
          lightState = true;
          Serial.print("{M7}");//Light ON
        }
      }else if (!state){
        if (lightState){
          digitalWrite(_lightPin, LOW);
          lightState = false;
          Serial.print("{M8}");//Light OFF
        }
      }
    }

    void setAirPump(boolean state){
      if (state){
        if (!airPumpState){
          digitalWrite(_airPumpPin, HIGH);
          airPumpState = true;
          Serial.print("{M1}");//Air Pump ON
        }
      }else if (!state){
        if (airPumpState){
          digitalWrite(_airPumpPin, LOW);
          airPumpState = false;
          Serial.print("{M2}");//Air Pump OFF
        }
      }
    }

    void setFan(boolean state){
      if (state){
        if (!fanState){
          digitalWrite(_fanPin, HIGH);
          fanState = true;
          Serial.print("{M3}");//Fan ON
        }
      }else if (!state){
        if (fanState){
          digitalWrite(_fanPin, LOW);
          fanState = false;
          Serial.print("{M4}");//Fan OFF
        }
      }
    }
};

Settings set(1,0);
Ambient ambient(3,4,A2,A3);
Control control(5,6,7,8);

class NodeCom
{
    int _incomingByte = 0;
  public:
    NodeCom(){
      
    }

    void sendData(){
      //clr();
      Serial.print("{"); // begin character 
      Serial.print("\"light\": ");
      Serial.print(ambient.light);
      Serial.print(", \"lightSys\": ");
      Serial.print(ambient.lightSys);
      Serial.print(", \"temp\": ");
      Serial.print(ambient.temp);
      Serial.print(", \"hum\": ");
      Serial.print(ambient.hum);
      Serial.print(", \"hi\": ");
      Serial.print(ambient.hi);
      Serial.print(", \"tempWater\": ");
      Serial.print(ambient.tempWater);
      Serial.print(", \"flowRate\": ");
      Serial.print(flowSensor.flowRate);
      Serial.print(", \"flowLiters\": ");
      Serial.print((flowSensor.totalMilliLitres / 1000.00));
      Serial.print(", \"time\": ");
      Serial.print(a.time.);
      Serial.print(", \"min\": ");
      Serial.print(minute());
      Serial.print(", \"day\": ");
      Serial.print(day());
      Serial.print(", \"month\": ");
      Serial.print(month());
      Serial.print(", \"year\": ");
      Serial.print(year());
      Serial.print("}"); // end character
      Serial.flush();
    }

    void receiveData(){
      if(Serial.find("T")) {
        aTime.processSyncMessage();
      }else{
        _incomingByte = Serial.read();
        Serial.print("I received: ");
        Serial.println(_incomingByte, DEC);
        if (_incomingByte == 49) set.save();
      }
    }

    void clr(){
      //clears previous sebt serial messages
      Serial.println("<CLR>");
      Serial.flush();
    }
};

NodeCom nodeCom;

void setup() {
  Serial.begin(9600);
  set.load();
  control.flowPauseEnd = now();
  //Serial.println(set.temperatureMax);
}

void loop() {
  delay(2000);
  if (Serial.available() > 0) {
    nodeCom.receiveData();
  }
  ambient.read();
  flowSensor.processCounter();
  relayCheck();
  nodeCom.sendData();
}

void relayCheck(){
  unsigned long now = now();
  control.lightPeriod = true;
  // Flow control
  if(control.pumpState && flowSensor.totalMilliLitres => set.litersPerCycle){
    control.setPump(false);
  }else if(!control.pumpState && now => control.flowPauseEnd){
    control.setPump(true);
  }

  // Air pump control
  if (set.airPumpConnected == 1){
    control.setAirPump(true);
  }else if (set.airPumpConnected == 0){
    control.setAirPump(false);
  }

  // Fan control
  if (set.fanConnected == 1 && ambient.temp > set.temperatureMax){
    control.setFan(true);
  }else{
    control.setFan(false);
  }
}

