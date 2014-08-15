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
int TEMP =      A14;	    // LM36 temperature sensor, bias is 5v.
int HEAT =      32;	    // WHITE
int COOL =      34;	    // YELLOW
int FAN =       36;         // GREEN
int RED =       6;
int GREEN =     4;

int manual = 1;

int IOTreadTemperature(char* rets, char* param);
int IOTsystem(char* rets, char *params);
int IOTmanual(char* rets, char* param); 
int IOTautonomous(char* rets, char* param);
IOTstatus(char* rets, char* params);
int IOToff(char* rets, char* params);

double readTemperature();
long timeBase;

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
  
  CPXaddFunction("IOTsystem",&IOTsystem);			// used to operate fan, cooling, heating from across the network
  CPXaddFunction("IOTReadTemperature",&IOTreadTemperature);	// used to get the temperature from across the network
  CPXaddFunction("IOTmanual",&IOTmanual); 			// turns off autonomous operation from across the network
  CPXaddFunction("IOTautonomous",&IOTautonomous);		// turns autonomous operation on from across the network
  CPXaddFunction("IOToff",&IOToff);				// turns all controls off from across the network
  CPXaddFunction("IOToff",&IOTstatus);				// queries system status, from across the network

  CPXaddUserLoop("userLoop",&userLoop);				// When CPX is not busy, it will call this, in a loop.
  
  CPXerrorPin(RED);
  CPXokPin(GREEN);
  CPXsetup();
}

/**
  * Autonomously run and control system at 72 degrees fahrenheit
  */
void loop() {
  CPXloop();
  autonomous(72);
}


void autonomous(int target) {
  double temperature;
  char status[512];
  IOTstatus(char* rets, NULL);
  Serial.print("*** "); Serial.println(rets);
  if(autonomous) {
    long now = millis();
    if (now - timeBase < 30000)
	return;

    temperature = readTemperature();
    if (temperature > target + 5) {
      doCool(target - 5);
    }
    else 
    if (readTemperature() < target - 5) {
      doHeat(target + 5);
    }
    return;
  }
}

/**
  * Heating and air conditioning control
  */
void hacControl(int on, int system) {
  char* name = "??";
  switch(system) {
  case HEAT: name = "HEAT"; break;
  case COOL: name = "AC"; break;
  }
  if (on) {
    digitalWrite(system,ON);
    digitalWrite(FAN,ON);
    Serial.print("### FAN: ON, "); Serial.print(name); Serial.println(" ON");
  } else {
    digitalWrite(system,OFF);
    digitalWrite(FAN,OFF);
    Serial.print("### FAN: OFF, "); Serial.print(name); Serial.println(" OFF");
  }
}

/**
  * Read the temperature in degrees fahrenheit
  */
double readTemperature() {
  int reading = analogRead(TEMP);
  double voltage = reading * 5.0;
  voltage /= 1024.0; 
  double temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
  // now convert to Fahrenheit
  double temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return temperatureF;
}

/**
  * Turn HAC off
  */
void off() {
  analogWrite(FAN,OFF);
  analogWrite(HEAT,OFF);
  analogWrite(COOL,OFF);
  Serial.println("### FAN OFF, HEAT OFF, AC OFF");
}



/**
  * Keep the system at the target temperature.
  */
void autonomous(double target) {
  double temperature;
  manual = false;
  off();
  if(autonomous) {
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

/**
  * Read the temperature
  */
int IOTreadTemperature(char* rets, char* param) {
  sprintf(rets,"%f",readTemperature());
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
  double temperature;
  sscanf(params,"%f",&temperature);
  autonomous(temperature);
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
  double temp = getTemperature();
  strcpy(rets,"Mode: ");
  if (manual) 
	strcat(rets,"manual ");
  else
   	strcat("auto ");
  strcat(rets,"Fan: "
  if (fan == ON)
	strcat(rets,"on ");
  else
        strcat(rets,"off ");
  if (cool == ON)
	strcat(rets,"on ");
  else
        strcat(rets,"off ");
  if (heat == ON)
	strcat(rets,"on ");
  else
        strcat(rets,"off ");
  strcat(rets,"temp: ");
  strcat(rets,temp);
}
