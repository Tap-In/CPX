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

void setup() {
  Serial.begin(19200);
  CPXerrorPin(13);
  CPXokPin(13);
  CPXsetup();
}

/**
  * Autonomously run and control system at 72 degrees fahrenheit
  */
void loop() {
  CPXloop();
}
