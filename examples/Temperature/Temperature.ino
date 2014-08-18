#include <JsonArray.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include <JsonParser.h>

#include <EEPROM.h>

#include <SPI.h>

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <CPX.h>
#include <Hardware.h>
#include <Lang.h>
#include <List.h>
#include <Config.h>

#define TEMP  A14

#define GREEN  4
#define RED    6
#define RGB_GREEN 46
#define RGB_BLUE 45
#define RGB_RED 44
#define delayTime 20

// Get the temperature from across the network
double getTemperature();

// Read the temperature in fahrenheit
int readTemperature(char* rets, char* param);

/**
  * Set CPX up
  */
void setup() {
  Serial.begin(19200);
  pinMode(TEMP,INPUT);
  CPXaddFunction("readTemperature",&readTemperature);
  CPXsetup();
 
}

/**
  * Periodic processing here
  */
void loop() {
  CPXloop();

}

/**
  * Read the temperature from the network
  */
int readTemperature(char* rets, char* param) {
  dtostrf(getTemperature(),7, 3, rets);
  return 1;
}

/**
  * Get the temperature as read by the LM36. Note, the bias is
  * set at 12 mv., which accounts for other devices on the 5v
  * rail. A reduction on voltage results in a higher temperature.
  * Loading the rail will cause the voltage to drop.
  */
double getTemperature() {
  int reading = 0;
  for (int i=0;i<100;i++) {
    reading += analogRead(TEMP);
  }
  reading = reading/100 - 12;
  double voltage = reading * 5.0;
  voltage /= 1024.0; 
  double temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
  // now convert to Fahrenheit
  double temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return temperatureF; 
}
