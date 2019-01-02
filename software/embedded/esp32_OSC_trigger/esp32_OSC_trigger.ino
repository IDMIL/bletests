/*
 * ESP32 BLE OSC Trigger Test
 * 
 * Generates an OSC message when a digital pin is toggled.
 * 
 * Can operate as a wireless client or AP
 *  (in both cases, destination IP must be hardcoded)
 * 
 * Dec 2018
 * johnty.wang@mail.mcgill.ca
 * 
*/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>


//if we want ESP to operate as AP, set to 1:
#define SOFTAP 0


#define D0 21

WiFiUDP udp;

char ssid[] = "xxx"; //  your network SSID (name)
char pass[] = "xxx";    // your network password (use for WPA, or use as key for WEP)

//hardwired IP here:
IPAddress destIP(192,168,5,6);
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

