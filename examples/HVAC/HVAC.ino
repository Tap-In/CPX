#include <EEPROM.h>

#include <SPI.h>

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <JsonArray.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include <JsonParser.h>

#include <CPX.h>
#include <Hardware.h>
#include <Lang.h>
#include <List.h>
#include <Config.h>

int ON   =      1;
int OFF =       0;
int TEMP =      A15;
int HEAT =      44;
int COOL =      46;
int FAN =       48;
int RED =       6;
int GREEN =     4;

int manual = 1;

int IOTreadTemperature(char* rets, char* param);
int IOTsystem(char* rets, char *params);
int IOTmanual(char* rets, char* param); 
int IOTautonomous(char* rets, char* param);
int IOToff(char* rets, char* params);

double readTemperature();

/**
  * Set up the system
  */
  
void setup() {
  Serial.begin(19200);
  pinMode(TEMP,0);
  pinMode(HEAT,1);
  pinMode(COOL,1);
  pinMode(FAN,1);
  off();
  
  CPXaddFunction("IOTsystem",&IOTsystem);
  CPXaddFunction("IOTReadTemperature",&IOTreadTemperature);
  CPXaddFunction("IOTmanual",&IOTmanual); 
  CPXaddFunction("IOTautonomous",&IOTautonomous);
  CPXaddFunction("IOToff",&IOToff);
  
  CPXerrorPin(RED);
  CPXokPin(GREEN);
  CPXsetup();
}

/**
  * Autonomously run and control system at 72 degrees fahrenheit
  */
void loop() {
  autonomous(72);      // control the system at 72 degrees fahrenheit

}

/**
  * Heating and air conditioning control
  */
void hacControl(int on, int system) {
  if (on) {
    digitalWrite(system,ON);
    digitalWrite(FAN,ON);
  } else {
    digitalWrite(system,OFF);
    digitalWrite(FAN,OFF);
  }
}

/**
  * Read the temperature in degrees fahrenheit
  */
double readTemperature() {
  double centigrade = analogRead(TEMP) * 100 -50;
  double fahrenheit = (centigrade * 9)/5 + 32;
  return fahrenheit;
}

/**
  * Turn HAC off
  */
void off() {
  analogWrite(FAN,OFF);
  analogWrite(HEAT,OFF);
  analogWrite(COOL,OFF);
}

/**
  * Keep the system at the target temperature.
  */
void autonomous(double target) {
  double temperature;
  manual = false;
  off();
  while(autonomous) {
    temperature = readTemperature();
    if (temperature > target + 5) {
      doCool(target - 5);
    }
    else 
    if (readTemperature() < target - 5) {
      doHeat(target + 5);
    }
    sendPing();
    delay(30000);
  }
}

/**
  * Cool the system to 5 degrees below target
  */
void doCool(double target) {
  hacControl(ON,COOL);
  double temperature = 0;
  while(temperature = readTemperature() > target) {
     delay(30000); 
  }
  off();
}

/**
  * Heat the system 5 degrees above target
  */
void doHeat(double target) {
  hacControl(ON,HEAT);
  double temperature = 0;
  while(temperature = readTemperature() < target) {
     delay(30000); 
  }
  off();
}

int IOTreadTemperature(char* rets, char* param) {
  sprintf(rets,"%f",readTemperature());
  return 1;
}

int IOTsystem(char* rets, char *params) {
  int mode, system;
  sscanf(params,"%d %d",&system,&mode);
  digitalWrite(system,mode);
  return 1;
}

int IOTautonomous(char* rets, char* params) {
  double temperature;
  sscanf(params,"%f",&temperature);
  autonomous(temperature);
}

int IOTmanual(char* rets, char* params) {
  manual = 1;
}

int IOToff(char* rets, char* params) {
  manual = 1;
  off();
}
