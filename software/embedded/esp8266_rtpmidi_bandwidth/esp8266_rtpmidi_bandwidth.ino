/*
 * ESP8266 RTP MIDI Trigger Test
 * 
 * Incrementally send out SysEx packets at higher frequencies
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


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "AppleMidi.h"

char ssid[] = "xxxx"; //  your network SSID (name)
char pass[] = "xxxx";    // your network password (use for WPA, or use as key for WEP)

int delay_ms = 1000;
unsigned long t0 = millis();
bool isConnected = false;

#define PKT_SIZE 20
uint8_t midiPacketLong[PKT_SIZE];

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

  Serial.print(F("Getting IP address..."));


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));


  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);


  for (int i = 0; i < PKT_SIZE; i++) {
    midiPacketLong[i] = i;
  }

  midiPacketLong[0] = 0xF0;
  midiPacketLong[PKT_SIZE - 1] = 0xF7;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool armed = true;

void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();
  if (Serial.available()) {
    char val = Serial.read();
    if (val == '1')
      delay_ms = 10;
    else if (val == '2')
      delay_ms = 8;
    else if (val == '3')
      delay_ms = 5;
    else if (val == '4')
      delay_ms = 3;
    else if (val == '5')
      delay_ms = 2;
    else if (val == '0')
      delay_ms = 100;
    else if (val == '0')
      delay_ms = 5;
    Serial.print("delay set to ");
    Serial.print(delay_ms);
    Serial.println(" ms");
  }

  if (isConnected) {
    //t0 = millis();
    delay(delay_ms);
    AppleMIDI.sendSysEx(midiPacketLong, PKT_SIZE);

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
