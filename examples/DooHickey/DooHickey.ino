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

#define BUTTON_PIN 22

// DIGITAL
#define GREEN  4
#define RED    6

// RGB
#define RGB_GREEN 46
#define RGB_BLUE 45
#define RGB_RED 44
#define delayTime 20


int lamp(char* returns, char* param);
int testme(char* rets, char* param);
int fade(char* rets, char* param);

void setup() {
  SPI.begin();
  Serial.begin(19200);
   
  CPXaddFunction("testme",&testme);
  CPXaddFunction("lamp",&lamp);
  CPXaddFunction("fade",&fade);
  
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  
  digitalWrite(RED,0);
  digitalWrite(GREEN,1);
  delay(100);
  
  char returns[512];
  
  fade(returns,"1");
  digitalWrite(GREEN,0);
 

  // RGB SETUP
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  pinMode(RGB_RED, OUTPUT);
  digitalWrite(RGB_GREEN, HIGH);
  digitalWrite(RGB_BLUE, HIGH);
  digitalWrite(RGB_RED, HIGH);
  ///////////////////////////////////////////////////
  
  /*
 * Comment out if you don't want the light show on Peter's fun box
 */
  int mode = 1;
  for (int i=0;i<6;i++) {
    digitalWrite(RED,!mode);
    digitalWrite(GREEN,!mode);
    delay(250);
    if (mode == 1)
      mode = 0;
    else
      mode = 1;
      
  }
  digitalWrite(RED,1);
  digitalWrite(GREEN,0);
  
  CPXerrorPin(RED);
  CPXokPin(GREEN);
  CPXsetup();
  
  digitalWrite(RED,0);
  digitalWrite(GREEN,1);
  delay(200);
  digitalWrite(RGB_RED,1);
  digitalWrite(RGB_GREEN,1);
  digitalWrite(RGB_BLUE,1);
  
  digitalWrite(GREEN,1);
  

}

void loop() {
  CPXloop();

}

/*
 * A demonstration function for use with the 'call' command.
 */
int testme(char* rets, char* param) {
    strcpy(rets,"You have arrived");
    return 1;
}

int lamp(char* returns, char* param) {
  int offset;
  int n;
  int k = 0;
  char* data = param;
  while (1 == sscanf(data, " %d%n", &n, &offset)) {
        data += offset;
        
        if (k == 0) {
          k = n;
        } else {
          Serial.print("K "); Serial.println(k);
          Serial.print("X "); Serial.println(5000/k);
          double x = (double)n / 100;
          if (x < 0) {
            n = -1 *n;
           digitalWrite(7,0);
           for (int z= 0;z<5000/k; z+= n) {
              digitalWrite(9,1);
              delay(n);
              digitalWrite(9,0);
              delay(n);
           }
            
          } else {
           digitalWrite(9,0);
           for (int z= 0;z<5000/k; z+= n) {
              digitalWrite(7,1);
              delay(n);
              digitalWrite(7,0);
              delay(n);
           }
        }
        }
  }

  strcpy(returns,"lamp changed");
  return 1;
}

int fade(char* rets, char* param) {
  int count = atoi(param);
  int redVal = 255;
  int blueVal = 0;
  int greenVal = 0;
  
for (int k=0;k<count;k++) {
  for( int i = 0 ; i < 255 ; i += 1 ){
    greenVal += 1;
    redVal -= 1;
    analogWrite( RGB_GREEN, 255 - greenVal );
    analogWrite( RGB_RED, 255 - redVal );

    delay( delayTime );
    sendPing();
  }
  sendPing();
  
  redVal = 0;
  blueVal = 0;
  greenVal = 255;
  for( int i = 0 ; i < 255 ; i += 1 ){
    blueVal += 1;
    greenVal -= 1;
    analogWrite( RGB_BLUE, 255 - blueVal );
    analogWrite( RGB_GREEN, 255 - greenVal );

    delay( delayTime );
    sendPing();
  }
  sendPing();
 
  redVal = 0;
  blueVal = 255;
  greenVal = 0;
  for( int i = 0 ; i < 255 ; i += 1 ){
    redVal += 1;
    blueVal -= 1;
    analogWrite( RGB_RED, 255 - redVal );
    analogWrite( RGB_BLUE, 255 - blueVal );

    delay( delayTime );
    sendPing();
  }
  sendPing();
  
}
  strcpy(rets,"complete");
}
