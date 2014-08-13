#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>


#include "Arduino.h"

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <stdio.h>
#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>

#include "List.h"
#include "Lang.h"
#include "Hardware.h"
#include "Config.h"

extern JsonHashTable hashTable;
extern int interface;
extern Adafruit_CC3000_Client client;
extern int nJumps;
extern int nLang;
extern int nFuncs;
extern int nSym;
extern jumpTYPE jumps[];
extern langTYPE lang[];
extern List* functions;  
extern symbolTYPE symbols[];
extern int getEEPROM(int address);
extern void setEEPROM(int address, int value);
extern long R;
extern int errorPin;

extern void sendPing();

/**
  * Sends the device authentication to the host.
  */
void auth(char* returns, JsonHashTable json, char* text) {
  boolean auth = json.getBool("value");
  if (auth == false) {
    //Serial.println("Authorization failed, halted");
    digitalWrite(6,1);
    while(true);
  } else {
    if (interface == WIFI)
      Serial.println("Authorization Ok");
  }
  digitalWrite(4,1);
  returns[0] = 0;
}

/**
  * Respond to a PING from the host.
  */
void ping(char* returns, JsonHashTable json, char* text) { 
    sprintf(returns,"{\"map\":{\"value\":%u},\"globals\":[]}",millis()/1000);
}

/**
  * Wait here, and trigger on an event. Expects a pin to wait on; if timeout
  * is 0, the trigger waits indefinitely, otherwise it waits the
  * specified number of milliseconds.
  */
void trigger(char* returns, JsonHashTable json, char* text) { 
  
  int pin = symbolRef(json,"pin");
  long to = symbolRef(json,"timeout");
  pinMode(pin, INPUT);
  delay(100);
  int old  = digitalRead(pin);
  long value = millis();
  while(digitalRead(pin) == old) {
    delay(100);
    if (to > 0 &&( to < millis() - value)) {
        sprintf_P(returns,tempn,-1);
        return;
    }
  }
  value = millis() - value;
  R = value;
  sprintf_P(returns,tempn,value);
}

/**
  * Digital Write to a pin using the specified value.
  */
  
void digitalwrite(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;

  v1 = symbolRef(json,"pin");
  v2 = symbolRef(json,"value");
  setDigitalValue(v1,v2);
  sprintf_P(returns,temp,"ok");
}

/**
  * Read a digital pin. Expects a pin number.
  */
void digitalread(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  R = v2 = getDigitalValue(v1);
  sprintf_P(returns,tempn,v2);
  shift(json, v2);
}

/**
  * Analog write, expects a pin number and a 0 or 1 as the value
  */

void analogwrite(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  v2 = symbolRef(json,"value");
  setAnalogValue(v1,v2);
  sprintf_P(returns,temp,"ok");
}

/**
  * Read an analog pin, expects a pin number.
  */
void analogread(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  R = v2 = getAnalogValue(v1);
  sprintf_P(returns,tempn,v2);
  shift(json, v2);
}

/**
  * Return the value of an EEPROM address. Expects an address.
  */
  
void geteeprom(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"address");
  R = v2 = getEEPROM(v1);
  sprintf_P(returns,tempn,v2);
  shift(json,v2);
}

/**
  * Set EEPROM value. Sets address to value.
  */
void seteeprom(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"address");
  v2 = symbolRef(json,"value");
  setEEPROM(v1,v2);
  sprintf_P(returns,temp,"ok");
}

/**
  * Delay "value" number of milliseconds.
  */
  
void delayx(char* returns, JsonHashTable json, char* text) { 
  int v1;
  v1 = symbolRef(json,"value");
  delay(v1);
  sprintf_P(returns,temp,"ok");
}

/**
  * The control plan notification command. Expects plan-user.
  * plan, args, and endpoint. See sendCPMessage for explanation.
  */

void notify(char* returns, JsonHashTable json, char* text) { 
  char returnsx[512];
  char* user = json.getString("plan-user");
  char* plan = json.getString("plan");
  char* value = json.getString("args");
  char* endpoint = json.getString("endpoint");
  long wait = json.getLong("wait");
  
  sendCPmessage(user,plan, value, endpoint, returnsx, wait);
  sprintf_P(returns,temp,returnsx);
}

/**
  * Send a callback to the proxy.
  */
void callback(char* returns, JsonHashTable json, char* text) { 

  char* buf = (char*)malloc(512);
  strcpy(buf,"{\"map\":{\"proxy-command\":\"callback\"}}");
  char* send = encode(buf);
  Serial.println(send);
  transmit(send);
  free(send);
  free(buf);
  sprintf_P(returns,temp,"");
}

/**
  * The goto command, expects where, count and label. count is
  * optional.
  */
void gotox(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
    char* label = json.getString("where");
    if (json.containsKey("count")) {
      int count = json.getLong("count");
      int k = getJumpCount(label);
      if (k == -1) {
        k = count - 1;
        strcpy(jumps[nJumps].label,label);
        jumps[nJumps].count = k;
        nJumps++;
      }
      if (k > 1) 
        strcpy(text,label);
      else
        text[0]=0;
    }
    else {
      strcpy(text,label);
      
    }
    sprintf_P(returns,temp,"ok");
  }


/**
  * Embedded print, prints on the USB console, not interpreted
  * as a serial command to the host proxy.
  */
void printx(char* returns, JsonHashTable json, char* text) { 
  char* value = json.getString("value");
  Serial.println("READY");
  sprintf_P(returns,iprint,value);
  Serial.println("DONE");
}


/**
  * Test Last operation's value. Tests 'R' against the value you
  * give it to work with. If the operation is true the where is jumped
  * to
  */
void testR(char* returns, JsonHashTable json, char* text) {
  int v1, v2;
  char* label = json.getString("where");
  char* op = json.getString("op");
  int value = json.getLong("value");
  int truth = 0;
  text[0] = 0;
  if (strcmp(op,"lt")==0) {
	if (R < value)
	   truth = 1;
  } else
  if (strcmp(op,"le")==0) {
	if (R <= value)
           truth = 1;
  } else
  if (strcmp(op,"eq")==0) {
        if (R == value)
	   truth = 1;
  } else
  if (strcmp(op,"gt")==0) {
	if (R > value)
           truth = 1;
  } else
  if (strcmp(op,"ge")==0) {
       if (R >= value)
           truth = 1;
  } 
  if (truth == 1) {
      strcpy(text,label);
  }
  sprintf_P(returns,temp,"ok");
}

/**
  * Call a C function, giben the name, and the param. The param
  * is given as a string, the C function must deal with the param
  * as approrpriate.
  */
void call(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  char* fname = json.getString("function");
  char* param = json.getString("param");
  callTYPE* call;
  findFunction(fname,call);
  if (call == NULL)
      sprintf(returns,err,fname);
  else {
    char rval[128];
    int rc = call->functionPtr(rval,param);
      sprintf(returns,"{\"map\":{\"value\":\"%s\",\"rc\":%d},\"globals\":[]}",rval,rc);
    }
  }
  
/**
  * Send control plan a message.
  * User is the cplan's user name
  * Plan is the plan id.
  * Args is the JSON arguments expected by the plan.
  * Endpoint is the HTTP address of the control plan server.
  */
void sendCPmessage(char* user, char* plan, char* args, char* endpoint, char* returns, int wait) {
  char* buf = (char*)malloc(sizeof(char)*1024);
  char swait[16];
  sprintf(swait,"%d",wait);
  strcpy(buf,"{\"map\":{\"proxy-command\":\"notify\",\"plan-user\":\"");
  strcat(buf,user);
  strcat(buf,"\",\"plan\":\"");
  strcat(buf,plan);
  strcat(buf,"\"");

  if (!(args == NULL || strlen(args)==0)) {
  	strcat(buf,"\",\"args\":");
	strcat(buf,args);
  }

  strcat(buf, ",\"endpoint\":\"");
  strcat(buf,endpoint);
  strcat(buf,"\",\"wait\":");
  strcat(buf,swait);
  strcat(buf,"}}");
  char* send = encode(buf);
  //Serial.println(send);
  transmit(send);
  free(send);
  free(buf);
  char *json = readBlock();
  strcpy(returns,json);
  free(json);
}

int getJumpCount(char* label) {
  for (int i=0;i<nJumps;i++) {
    if (strcmp(label,jumps[i].label)==0)
      return jumps[i].count--;
  }
  return -1;
}

/**
  * Given the name of a symbol and it's start and stop address
  * return the values as a list.
  */
void getimage(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int tSize;
  int k = findSymbol(name);
  byte* bvalue;
  int* ivalue;
  double* dvalue;
  char array[1024];
  
  if (k == -1) {
    sprintf(returns,err,"unknown symbol");
    return;
  }
  int start = symbolRef(json,"start");
  int stop  = symbolRef(json,"stop");
  char value[16];
  int *imemory = (int*)&symbols[k].memory[start*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[start*getSize(symbols[k].type)];
  strcpy(array,"[");
  for (int i = 0; i<stop;i++) {
    switch(symbols[k].type) {
      case BYTE:
         sprintf(value,"%u",bmemory[i]);
         break;
      case INT:
         sprintf(value,"%i",imemory[i]);
         break;
     case DOUBLE:
        break;
     }
     strcat(array,value);
     if (i+1 < stop)
       strcat(array,",");
  }
  strcat(array,"]");
  sprintf_P(returns,temp,array);
}

/**
  * Given a symbol name and index, return the value.
  */
long getValueAt(char* name, int index) {
  int k = findSymbol(name);
  long value;
  int *imemory = (int*)&symbols[k].memory[index*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[index*getSize(symbols[k].type)];
  switch(symbols[k].type) {
      case BYTE:
         value = bmemory[0];
         break;
      case INT:
         value = imemory[0];
         break;
     case DOUBLE:
        break;
     }
  return value;
}

/**
  * Returns the size of the symbol's base type.
  */
int getSize(int type) {
  switch(type) {
    case INT: return sizeof(int);
    case BYTE: return sizeof(byte);
    case DOUBLE: return sizeof(double);
  }
  return -1;
}

///////////////////////////////////////////////////////////

void redis(char* returns, JsonHashTable json, char* text) { 
Serial.println("REDIS COMMAND");
   char rc[512];
   char* subcmd = json.getString("subcommand");
   char* key = json.getString("key");
   char* value = json.getString("value");
   long wait = json.getLong("wait");
   doRedis(rc,subcmd,key,value,wait);
   sprintf_P(returns,tempq,rc);
   Serial.print("REDIS RETURNS "); Serial.println(returns);
}

void createString(char* buf, char* one, char *two) {
  if (two == NULL)
	return;
  if (one == NULL) {
	strcat(buf,two);
 	return;
  }
  strcat(buf,"\"");
  strcat(buf,one);
  strcat(buf,"\":\"");
  strcat(buf,two);
  strcat(buf,"\"");
}

char* doRedis(char* returns, char* subcmd, char* key, char* value, long wait) {
  char* buf = (char*)malloc(sizeof(char)*1024);
  char swait[16];
  sprintf(swait,"%d",wait);
  
  buf[0] = 0;
  createString(buf,NULL,"{\"map\":{");
  createString(buf,"command","redis");
  strcat(buf,",");
  createString(buf,"subcommand",subcmd);
  strcat(buf,",");
  if (strcmp(subcmd,"pub")==0) {
    createString(buf,"channel",key);
    strcat(buf,",");
    createString(buf,"message",value);
  } else {
    createString(buf,"key",key);
    if (value != NULL) strcat(buf,",");
    createString(buf,"value",value);
  }
  createString(buf,NULL,"}}");

  char* send = encode(buf);
  Serial.print("REDIS OUTGOING: "); Serial.println(send);
  transmit(send);
  free(send);
  free(buf);
  char *json = readBlock();
  strcpy(returns,json);
  free(json);
}

//////////////////////////////////////

/**
  * Set the arduino memory at a certain address to a list
  * of values
  */
void setimage(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int tSize;
  byte bvalue;
  int ivalue;
  double dvalue;
  int k = findSymbol(name);
  if (k == -1) {
    sprintf(returns,err,"unknown symbol");
    return;
  }
  
  int start = symbolRef(json,"start");
  JsonArray array = json.getArray("values");
  char value[16];
  int stop = array.getLength();
  int x, y;
  int *imemory = (int*)&symbols[k].memory[start*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[start*getSize(symbols[k].type)];
  for (int i = 0; i<stop;i++) {
    switch(symbols[k].type) {
      case BYTE:
         x = (char)array.getLong(i);
         bmemory[i] = x;
         break;
      case INT:
         y = array.getLong(i);
         imemory[i] = y;
         break;
     case DOUBLE:
        break;
     }
  }
  sprintf_P(returns,temp,"ok");
}

/**
  * Allocate memory on the Arduino and give it a name.
  */

void allocate(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int size = symbolRef(json,"size");
  char* type = json.getString("type");
  int tSize = 1;
  
  int k = findSymbol(name);
  if (k != -1) {
      free(symbols[k].memory);
  } else {
    strcpy(symbols[nSym].name,name);
    if (strcmp(type,"byte")==0) {
      symbols[nSym].type = BYTE;
      tSize = 1;
    } 
    if (strcmp(type,"int")==0) {
      symbols[nSym].type = INT;
      tSize = sizeof(int);
    } 
    if (strcmp(type,"double")==0) {
      symbols[nSym].type = DOUBLE;
      tSize = sizeof(double);
    } 
    k = nSym;
    nSym++;
  }
  symbols[k].memory = (char*)malloc(tSize * size);
  sprintf_P(returns,temp,"ok");
}

/**
  * Find the index of a symbol in the symbol table
  */
int findSymbol(char* name) {
  for (int i=0;i<nSym;i++) {
     if (strcmp(name,symbols[i].name)==0)
      return i; 
  }
  return -1;
}

/*
 * Call an internal function by name, the 'call' method
 */
void findFunction(char* name, callTYPE* call) {
  call = NULL;
  for (int i = 0; i < List_size(functions); i++) {
    callTYPE* key = (callTYPE*)List_query(functions, i);
    if (strcmp(key->name, name) == 0)
    {
     call = key;
     return;
    }
  }
}

/*
 * Find the label specified in the goto construct
 */
int findIndex(char* label,JsonArray commands) {
    int pc = 0;
    char* what;
    while(pc < commands.getLength()) {
      JsonHashTable cmd = commands.getHashTable(pc);
      char* cLabel = cmd.getString("label");
      if (cLabel != NULL && strcmp(cLabel,label)==0)
        return pc;
      pc++;
    }
    return -1;
}


/*
 * Read first 4 bytes, convert to a number, which is how many bytes follow.
 * For example XXX9{"a":"b"}
 */
char* readBlock() {
  char len[4];
  char* buf;
  int sz = 0;
  char returns[512];
  int value;
  for (int i=0;i<4;i++) {
    if (interface == PROXY) {
      while(!Serial.available());
      value = Serial.read();
    } else {
     int z = 0;
     while(client.connected() && !client.available()) {
         if (z++ > 1000) {
           z = 0;
          sendPing();client.write("!",1);
    }
     }
     if (!client.connected()) {
         Serial.println("Host has disconnected");
delay(250);
         return NULL;	
         //blink(errorPin,250,0);
     }
      value = client.read();
    }
    if (value != 'X')
      len[sz++] = (char)value;
  }
  len[sz] = 0;
  
  sz = atoi(len);
  buf = (char*)malloc(sizeof(char)*sz + 6);
  for (int i = 0; i< sz; i++) {
    if (interface == PROXY) {
      while(!Serial.available());
      buf[i] = (char)Serial.read();
    } else {
      while(!client.available());
      buf[i] = (char)client.read();
    }
  }
  buf[sz] = 0;
  return buf;
}

/**
  * BLINK A PIN n times
  */
void blink(int PIN, int delta, int times) {
  int k = 0;
  while(true) {
    digitalWrite(PIN,0);
    delay(delta);
    digitalWrite(PIN,1);
    delay(delta);
    if (times > 0) {
      if (k++ > times) {
          digitalWrite(PIN,0);
          return;
      }
    }
  }
}

/*
 * Encode a buffer, puts 4 byte length of trailing buffer leading 0 is X and
 * and appends the rest of the buffer. For example {"a":"b"} is
 * encoded to: XXX9{"a":"b"}
 */
 
char* encode(char* data) {
   char len[6];
   char* newData = (char*)malloc((strlen(data)+4) * sizeof(char));
   sprintf(len,"%04d",strlen(data));
   for (int i=0;i<4;i++) {
     if (len[i] == '0')
       len[i] = 'X';
     else
       break;
   }
   len[5] = 0;
   strcpy(newData,len);
   strcat(newData,data);
   return newData;
}

/*
 * Send the response to either Serial or to the Wifi shield, depending
 * on how you have it set up. If WiFi send in little chunks so you don't
 * overrun the C3000 transmit buffer.
 */
void transmit(char* buf) {
   if (interface == PROXY) {
     Serial.print(buf);
   } else {
     if (strlen(buf) > 60) {
       char block[70];
       for (int i = 0; i < strlen(buf); i+= 60) {
         strncpy(block, buf+i,60);
         block[60] = 0;
         client.fastrprint(block);
       }
     } else
       client.fastrprint(buf);
   }
}

/**
  * Dereference a symbol at this label, or if a number, simply return the number
  */
long symbolRef(JsonHashTable json, char* label) {
  long returns;
  if (label == "R")
    return R;
    
  int index = json.getLong("index");
  if (json.isNumber(label)) {
    returns = json.getLong(label);
  } else {
     returns = getValueAt(label,index);
  }
  return returns;
}

/**
  * Shift a value into the named command
  */
void shift(JsonHashTable json, long value) {
   char* name = json.getString("shift");
   if (name == NULL)
     return;
  int k = findSymbol(name);
  if (k == -1)
    return;
  int index = json.getLong("index");
  int *imemory = (int*)&symbols[k].memory[index*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[index*getSize(symbols[k].type)];
  switch(symbols[k].type) {
      case BYTE:
         bmemory[0] = (byte)value;
         break;
      case INT:
         imemory[0] = value;
         break;
     case DOUBLE:
        break;
     }
}


