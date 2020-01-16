

// These need to be included when using standard Ethernet
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
//#include "AppleMidi.h"


#define SOFTAP 0

//#define D0 21 //adafruit
#define D0 21 //lolin d32 

WiFiUDP udp;

char ssid[] = "guitarhub"; //  your network SSID (name)
char pass[] = "mappings";    // your network password (use for WPA, or use as key for WEP)

//IPAddress local_IP(192,168,88,88);
//IPAddress gateway(192,168,88,81);
//IPAddress subnet(255,255,0,0);

IPAddress destIP(192,168,5,200); //johnty's mac tower on guitarhub
const unsigned int destPort = 7000;
OSCMessage oscMsg("/test");


unsigned long t0 = millis();
bool isConnected = false;


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  oscMsg.add((int32_t)1);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  pinMode(D0, INPUT);

  //static:
  //  if (!WiFi.config(local_IP, gateway, subnet)) {
  //    Serial.println("STA failed to configure!");
  //  }

  Serial.print(F("Connecting to WiFi..."));
  if (!SOFTAP) {
    WiFi.begin(ssid,pass);
  }
  else {
    WiFi.softAP(ssid, pass);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP is: ");
    Serial.println(IP);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  isConnected = true;

  Serial.println();
  Serial.print(F("IP address is "));
  if (SOFTAP) Serial.println(WiFi.softAPIP());
  else Serial.println(WiFi.localIP());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool armed = true;

void loop()
{

  if (isConnected) {
    t0 = millis();
    if (digitalRead(D0) == LOW && !armed) {
      armed = true;
      //AppleMIDI.noteOff(84, 0, 1);
    }
    if (digitalRead(D0) == HIGH && armed) {
      //AppleMIDI.noteOn(84, 127, 1);
      delay(2);
      udp.beginPacket(destIP, destPort);
      oscMsg.send(udp);
      udp.endPacket();
      armed = false;
      Serial.println(".");
    }

  }
}

