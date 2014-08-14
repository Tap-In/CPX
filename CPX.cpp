
/*
    This program is free software: you can redistribute it and/or modify it under the 
    terms of the GNU General Public License as published by the Free Software Foundation, 
    either version 3 of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <SPI.h>
#include <EEPROM.h>

#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include "Lang.h"
#include "Config.h"
#include "Hardware.h"
#include "List.h"


////////////////////////////////////////////////////////////////
int INTERFACE_TYPE  =       WIFI;

// If proxy these will be ignored
#define CONTROL_PLAN_ADDR    "arduinoproxy.tapinsystems.net"
//#define CONTROL_PLAN_ADDR   "192.168.1.15"
//#define CONTROL_PLAN_ADDR   "192.168.1.3"
#define CONTROL_PLAN_PORT    6666
////////////////////////////////////////////////////////////////

#define WLAN_SECURITY        WLAN_SEC_WPA2
char USER[32] =               {"BEN"};
char ID[32]   =               {"5551212"};
char WLAN_SSID[32] =          {"SuperiorCourtData"};
char WLAN_PASS[32]  =         {"jiujitsu"};
////////////////////////////////////////////////////////////////

int errorPin = 13;
int okPin = 13;

JsonParser<128> parser;
JsonHashTable hashTable;
int interface = INTERFACE_TYPE;

Adafruit_CC3000_Client client;
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

int nJumps = 0;
int nFuncs = 0;
int nSym = 0;
jumpTYPE jumps[8];

List* keywords =  List_create();
List* functions = List_create();
symbolTYPE symbols[8];

/*
 * Internal functions of the CPX
 */
langTYPE* initKey(char* word,  void (*functionPtr)(char* returns, JsonHashTable json, char* text));
void doWiFi();
bool displayConnectionDetails(void);
void doCommand(char* returns, JsonHashTable json, char* label);
void sendPing();

void construct(char*,char*[]);
uint32_t getIp(char *address) ;
uint32_t ipAddress;
void connectToProxy();
long lastTime;
int pingCount = 0;

long R;

void injectKeyWords() {
  List_push(keywords,initKey((char*)"ping", &ping));
  List_push(keywords,initKey((char*)"auth", &auth));
  List_push(keywords,initKey((char*)"trigger", &trigger));
  List_push(keywords,initKey((char*)"digitalwrite",&digitalwrite));
  List_push(keywords,initKey((char*)"digitalread",&digitalread));
  List_push(keywords,initKey((char*)"analogread",&analogread));
  List_push(keywords,initKey((char*)"analogwrite",&analogwrite));
  List_push(keywords,initKey((char*)"delay", &delayx));
  List_push(keywords,initKey((char*)"notify",&notify));
  List_push(keywords,initKey((char*)"goto",&gotox));
  List_push(keywords,initKey((char*)"print", &printx));
  List_push(keywords,initKey((char*)"call",&call));
  List_push(keywords,initKey((char*)"allocate", &allocate));
  List_push(keywords,initKey((char*)"getimage",&getimage));
  List_push(keywords,initKey((char*)"setimage", &setimage));
  List_push(keywords,initKey((char*)"seteeprom",&seteeprom));
  List_push(keywords,initKey((char*)"geteeprom",&geteeprom));
  List_push(keywords,initKey((char*)"send-callback", &callback));
  List_push(keywords,initKey((char*)"testR", &testR));
  List_push(keywords,initKey((char*)"redis", &redis));
}

void CPXaddFunction(char* name, int (*functionPtr)(char*,char*)) {
  callTYPE *key = (callTYPE*)malloc(sizeof(callTYPE));
  strcpy(key->name, name);
  key->functionPtr = functionPtr;
  List_push(functions,key);
}


langTYPE* initKey(char* word,  void (*functionPtr)(char* returns, JsonHashTable json, char* text))
{
  langTYPE *key = (langTYPE*)malloc(sizeof(langTYPE));
  strcpy(key->name, word);
  key->functionPtr = functionPtr;
  return key;
}

void CPXsetup() {
  
  char returns[512];
  injectKeyWords();
  readConfigFromProm();
  lastTime = millis();
 
  doWiFi();
 
/************************************************************/
 if (interface == WIFI) {
     Serial.println("CPX .1 started");
 /*    char buf[128];
     sprintf(buf,"{\"map\":{\"user\":\"%s\",\"id\":\"%s\"}}",USER,ID);
     //construct(returns,values);
     char *send = encode(buf);
     Serial.println(send);
     transmit(send);
     free(send); */
  } else {
    sprintf_P(returns,iprint,"CPX .1 started");
    char *send = encode(returns);
    Serial.write(send);
    free(send);
  }
  sprintf(returns,"!Boot/%s/%s",USER,ID);
  char *send = encode(returns);
  Serial.write(send);
  free(send);
}

void CPXerrorPin(int pin) {
  errorPin = pin;
}

void CPXokPin(int pin) {
  okPin = pin;
}

void construct(char* returns,char *vals[]) {
  strcpy(returns,"{\"map\":{");
  int i = 0;
  char **p = vals;
  while(*p != NULL) {
    strcat(returns,*p++);
    strcat(returns,":");
    strcat(returns,*p++);
    if (*p != NULL)
      strcat(returns,",");
  }
  strcat(returns,"}}");
}

void doWiFi() {
  if (interface != WIFI)
    return;
  if (!cc3000.begin())
  {
    Serial.println(F("CC3000 Couldn't begin(, hardware error"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
   // Try looking up the website's IP address
  
  ipAddress = getIp(CONTROL_PLAN_ADDR);
  connectToProxy();
}

void connectToProxy() {
  Serial.print(F("Connecting to ")); Serial.print(CONTROL_PLAN_ADDR); Serial.print("... "); 
  while(true) {
    client = cc3000.connectTCP(ipAddress, CONTROL_PLAN_PORT);
    if (client.connected()) {
      Serial.println(F("Wifi Connected"));
      blink(okPin,250,3);

  char buf[128];
     sprintf(buf,"{\"map\":{\"user\":\"%s\",\"id\":\"%s\"}}",USER,ID);
     //construct(returns,values);
     char *send = encode(buf);
     Serial.println(send);
     transmit(send);
     free(send);

      return;
    }
Serial.println("Waiting for retry");
    delay(5000);
  }
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/*
 * Read commands from Control Plan
 */
void CPXloop() {
  char *json;
  char command[32];
  char returns[512];
  char label[32];
  int v1, v2, pc;
  nJumps = 0;
  json = readBlock();
  lastTime = millis();
  if (json == NULL) {
    Serial.println("Host disconnected, need to reconnect");
    //blink(errorPin,250,0);
    connectToProxy();
    return;
  }

  // Serial.print("GOT COMMANDS: "); Serial.println(json);

  JsonHashTable hashTable = parser.parseHashTable(json);
  hashTable = hashTable.getHashTable("map");
  JsonArray commands = hashTable.getArray("commands");
  if (commands.success()) {
    pc = 0;
    sprintf_P(returns,temp,"ok");
    char* send = encode(returns);
    transmit(send);                 // ack command block
    free(send);
   
    while(pc < commands.getLength()) {
        label[0] = 0;
        int inter = 0;
	sendPing();
        if (interface == PROXY) {
          if (Serial.available()) { 
              inter = 1;
              char* send = encode("{\"map\":{\"value\":\"interrupt\"},\"globals\":[]}");      
              transmit(send);
              free(json);;
              free(send);
              while(Serial.available()) Serial.read();
              return;
          }
        } else {
          if (client.available()) {
              inter = 1;
              char* send = encode("{\"map\":{\"value\":\"interrupt\"},\"globals\":[]}"); 
              transmit(send);
              free(json);;
              free(send);
              while(client.available()) client.read();
              return;
          }
        }
        
        if (inter == 1) {
            char* send = encode("{\"map\":{\"value\":\"ok\"},\"globals\":[]}");      
            transmit(send);
            free(json);;
            free(send);
            while(Serial.available()) Serial.read();
            return;
        }
        
        JsonHashTable cmd = commands.getHashTable(pc);
        doCommand(returns,cmd,label);
        if (strlen(label) == 0)
          pc++;
        else
          pc = findIndex(label,commands);
          if (pc == -1) {
            sprintf_P(returns,err,"No such label %s",label);
            break;
          }
      }
  } else {
      doCommand(returns,hashTable,label);
  }
  
  if (strlen(returns) == 0) {                   // No need to answer
      free(json);
      return;
  }
    
  char* send = encode(returns);
  transmit(send);
  free(json);
  free(send);
  
}

int CPXconnected() {
   if (interface == PROXY || client.connected())
      return 1;
   return 0;
}

/*

/*
 * Execute the command specified
 */
void doCommand(char* returns, JsonHashTable json, char* text) { 
  char* command;
  command = json.getString("command");
  if (command == NULL) {
     sprintf_P(returns,err,"badly formed command"); 
     return;
  }
  for (int i = 0; i < List_size(keywords); i++) {
    langTYPE* key = (langTYPE*)List_query(keywords, i);
    if (strcmp(key->name, command) == 0)
    {
      key->functionPtr(returns,json,text);
      return;
    }
  }
  
  
  sprintf_P(returns,err,"unknown command"); 
}


void setEEPROM(int address, int value) {
   EEPROM.write(address, value);
}

int getEEPROM(int address) {
   return EEPROM.read(address);
}


void sendPing() {
  if (lastTime + 15000 < millis()) {
    if (interface != PROXY) {
      if(client.connected()) {
       client.write("!",1); 
       Serial.print("PING " ); Serial.println(pingCount++);
      }
    }
    lastTime = millis();
  }
}

uint32_t getIp(char *address) {
  uint32_t ip = 0;
  int k = 0;
  
  int a, b, c, d;
  k = sscanf(address,"%u.%u.%u.%u",&a,&b,&c,&d);
  if (k == 4) {
    ip = a;
    ip = ip << 8 | b;
    ip = ip << 8 | c;
    ip = ip << 8 | d;
    
    Serial.print("A = " ); Serial.println(a);
    Serial.print("B = " ); Serial.println(b);
    Serial.print("C = " ); Serial.println(c);
    Serial.print("D = " ); Serial.println(d);

    
    Serial.print("Remote IP address: "); Serial.print(ip); Serial.println();
    return ip;
  } else {
    while (ip == 0) {
      if (! cc3000.getHostByName(address, &ip)) {
        Serial.println(F("Couldn't resolve, using dotted decimal"));
      }
      delay(500);
      if (k++ > 20)
        break;
    }
  }
  if (k < 20) {
     Serial.print("Remote IP address: "); Serial.print(ip); Serial.println();
    return ip;
  }
  Serial.println("IP address resolution failed, Halt!");
  while(true);
}





