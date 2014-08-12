
void readConfigFromProm() ;

#define WIFI   1
#define PROXY  0

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

//////////////////////////////////////////////////////////////////

#define temp      PSTR("{\"map\":{\"value\":\"%s\"},\"globals\":[]}")
#define tempq      PSTR("{\"map\":{\"value\":%s},\"globals\":[]}")
#define iprint    PSTR("{\"iprint\":\"%s\"}")
#define tempn     PSTR("{\"map\":{\"value\":%d},\"globals\":[]}")
#define err       PSTR("{\"map\":{\"error\":true,\"value\":\"%s\"},\"globals\":[]}")

/////////////////////////


