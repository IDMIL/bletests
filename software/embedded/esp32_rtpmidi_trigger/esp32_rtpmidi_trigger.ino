/*
 * ESP32 RTP MIDI Trigger Test
 * 
 * Generates a RTP MIDI message when a digital pin is toggled.
 * 
 * Can operate as a wireless client or AP
 * note: auto discovery may not work so rely on
 * knowing the IP address of the ESP after joining network (printed to serial console)
 * it may require a manual connect from Apple's network MIDI session.
 * 
 * Dec 2018
 * johnty.wang@mail.mcgill.ca
 * 
*/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "AppleMidi.h"


#define SOFTAP 0

//digital trigger pin:
#define D0 21

char ssid[] = "xxx"; //  your network SSID (name)
char pass[] = "xxx";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

// Forward declaration
void OnAppleMidiConnected(uint32_t ssrc, char* name);
void OnAppleMidiDisconnected(uint32_t ssrc);
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity);
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
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

  Serial.println();
  Serial.print(F("IP address is "));
  if (SOFTAP) Serial.println(WiFi.softAPIP());
  else Serial.println(WiFi.localIP());
  

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));

  if (SOFTAP) Serial.print(WiFi.softAPIP());
  else Serial.print(WiFi.localIP());

  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool armed = true;

void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();

  if (isConnected) {
    t0 = millis();
    if (digitalRead(D0) == LOW && !armed) {
      armed = true;
      //AppleMIDI.noteOff(84, 0, 1);
    }

    if (digitalRead(D0) == HIGH && armed) {
      AppleMIDI.noteOn(84, 127, 1);
      armed = false;
      Serial.println(".");
    }

  }

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  //  if (isConnected && (millis() - t0) > 100)
  //  {
  //    t0 = millis();
  //    //   Serial.print(".");
  //
  //    byte note = random(45, 90);
  //    byte velocity = 55;
  //    byte channel = 1;
  //
  //    AppleMIDI.noteOn(note, velocity, channel);
  //    AppleMIDI.noteOff(note, velocity, channel);
  //  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected  = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected  = false;
  Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOn from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}
