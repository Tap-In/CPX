
#include <Arduino.h>
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

// TEMP LM36    A14 	  
// HEAT WHITE
// COOL YELLOW
// FAN  GREEN

#define ON      0
#define OFF     1
#define TEMP    A14 	   
#define HEAT    32 	
#define COOL    34 	
#define FAN     36     
#define RED     6
#define GREEN   4

int IOTreadTemperature(char* rets, char* param);
int IOTsystem(char* rets, char *params);
int IOTmanual(char* rets, char* param); 
int IOTautonomous(char* rets, char* param);
int IOTstatus(char* rets, char* params);
int IOToff(char* rets, char* params);

double readTemperature();
double autoTemp = 72.0;
long timeBase;
long statusBase = 0;
int heat, fan, cool, manual;

double sims[] = {60,65,70,80};
double lastTemperature;
int nSims = 4;
int simCount = 0;
int bias = 0;

/**
  * Set up the system
  */
  
void setup() {
  Serial.begin(19200);
  pinMode(TEMP,INPUT);
  pinMode(HEAT,OUTPUT);
  pinMode(COOL,OUTPUT);
  pinMode(FAN,OUTPUT);
  manual = false;
  off();
  timeBase = millis();
  
  CPXaddFunction("IOTstatus",&IOTstatus);			// used to operate fan, cooling, heating from across the network
  CPXaddFunction("IOTsystem",&IOTsystem);			// used to operate fan, cooling, heating from across the network
  CPXaddFunction("IOTReadTemperature",&IOTreadTemperature);	// used to get the temperature from across the network
  CPXaddFunction("IOTmanual",&IOTmanual); 			// turns off autonomous operation from across the network
  CPXaddFunction("IOTautonomous",&IOTautonomous);		// turns autonomous operation on from across the network
  CPXaddFunction("IOToff",&IOToff);				// turns all controls off from across the network
  CPXaddFunction("IOToff",&IOTstatus);				// queries system status, from across the network
  
  CPXerrorPin(RED);
  CPXokPin(GREEN);
  CPXsetup();
  CPXprint("HVAC starting...");
  lastTemperature = readTemperature();
}

/**
  * Autonomously run and control system at 72 degrees fahrenheit
  */
void loop() {
  CPXloop();
  autonomous();
}

/**
  * Turn HAC off
  */
void off() {
  digitalWrite(FAN,OFF);
  digitalWrite(HEAT,OFF);
  digitalWrite(COOL,OFF);
  heat = fan = cool = OFF;
  bias = 0;
  lastTemperature = readTemperature();
}

void autonomous() {
  long now = millis();
  double temperature;
  char status[512];
  if (now-statusBase < 30000) 
    return;
  IOTstatus(status, NULL);
  CPXprint(status);
  statusBase = now;
  if(!manual) {
    temperature = readTemperature();
    if (temperature > autoTemp + 5) {
      doCool(autoTemp - 5);
    }
    else 
    if (readTemperature() < autoTemp - 5) {
      doHeat(autoTemp + 5);
    }
  }
  simCount++;
  if (simCount == nSims)
    simCount = 0;
  
}

/**
  * Heating and air conditioning control
  */
void hacControl(int mode, int system) {
  char msg[512];
    
  switch(system) {
  case HEAT:
      if (heat == mode)
        return;
      heat = fan = ON;
      cool = OFF;
      digitalWrite(HEAT,ON);
      digitalWrite(FAN,ON);
      digitalWrite(COOL,OFF);
            CPXprint("*** HEAT ON ***");
            bias = 15;
            delay(2000);
      break;
   case COOL: 
       if (cool == mode)
        return;
      cool = fan = ON;
      heat = OFF;
      digitalWrite(HEAT,OFF);
      digitalWrite(FAN,ON);
      digitalWrite(COOL,ON);    
      CPXprint("*** AC ON ***");
      bias = 15;
      break;
  }
}

/**
  * Read the temperature in degrees fahrenheit
  */
double readTemperature() {
  
 // return(sims[simCount]);
  int reading = 0;
  for (int i=0;i<100;i++) {
    reading += analogRead(TEMP);
  }
  reading /= 100;
  reading -= bias;
  double voltage = reading * 5.0;
  voltage /= 1024.0; 
  double temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
  // now convert to Fahrenheit
  double temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return temperatureF; 
}

/**
  * Cool the system to 5 degrees below target
  */
void doCool(double target) {
  hacControl(ON,COOL);
}

/**
  * Heat the system 5 degrees above target
  */
void doHeat(double target) {
  hacControl(ON,HEAT);
}

/**
  * Read the temperature
  */
int IOTreadTemperature(char* rets, char* param) {
  dtostrf(readTemperature(),7, 3, rets);
  return 1;
}

/**
  * Use a system command from the cloud
  */
int IOTsystem(char* rets, char *params) {
  int mode, system;
  sscanf(params,"%d %d",&system,&mode);
  digitalWrite(system,mode);
  return 1;
}

/**
  * Execute autonomously
  */
int IOTautonomous(char* rets, char* params) {
  manual = OFF;
  double temperature;
  sscanf(params,"%f",&temperature);
  IOTstatus(rets,params);
}

/**
  * Execute autonomously
  */
int IOTmanual(char* rets, char* params) {
  manual = ON;
  double temperature;
  sscanf(params,"%f",&autoTemp);
  autonomous();
}

/**
  * Sets the system to manual mode (controlled from the cloud).
int IOTmanual(char* rets, char* params) {
  manual = 1;
}

/**
  * Turn all heating controls off (controlled from the cloud).
  */
int IOToff(char* rets, char* params) {
  manual = 1;
  off();
}

/**
  * Return system status
  */
int IOTstatus(char* rets, char* params) {
  char stemp[16];
  double temperature = lastTemperature;
  char mstate[8];
  char fstate[8];
  char cstate[8];
  char hstate[8];
  strcpy(fstate,"OFF");
  strcpy(cstate,"OFF");
  strcpy(hstate,"OFF");
  strcpy(mstate,"MANUAL");
  strcpy(rets,"Mode: ");
  if (!manual) strcpy(mstate,"AUTO");
  if (fan == ON) strcpy(fstate,"ON");
  if (heat == ON) strcpy(hstate,"ON");
  if (cool == ON) strcpy(cstate,"ON");
  dtostrf(temperature,7, 3, stemp);
  sprintf(rets,"Mode: %s, Heat: %s, Cool: %s, Fan %s, Temp: %s",
                mstate,hstate,cstate,fstate,stemp);
}

