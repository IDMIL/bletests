/*
 * ESP32 BLE MIDI Trigger Test
 * 
 * Generates a BLE MIDI note event when a digital pin is toggled.
 * 
 * Tested on an Lolin32, Lolin D32 Pro, and Huzzah32 feather but should work on all ESP32 boards
 * 
 * Dec 2018
 * johnty.wang@mail.mcgill.ca
 * 
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define D0 2

bool armed = true;

static const uint8_t led = 5;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

int delay_ms = 1000;
uint16_t time_0;
uint16_t time_elapsed;

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp, not implemented
  0x00,  // status
  0x3c,  // 0x3c == 60 == middle c
  0x00   // velocity
};

#define PKT_SIZE 20
uint8_t midiPacketLong[PKT_SIZE];

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(led, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(led, LOW);
    };

    void onCongestion(BLEServer* pServer) {
      Serial.println("CONGESTION!!!");
      delay_ms = 1000; //slow down!
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
        { Serial.print(rxValue[i], HEX);
          Serial.print(" ");
        }
        Serial.println();

        Serial.print("len = ");
        Serial.println(rxValue.length());
        Serial.println("*********");
      }
    }
};

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  Serial.begin(115200);

  pinMode(D0, INPUT);

  BLEDevice::init("esp32");

  //BLEDevice::setMTU(25);
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  //BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

  // Create the BLE Service
  BLEService *pService = pServer->createService(MIDI_SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(MIDI_CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  //pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  //connect rx callback
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  //set conn intervals
  pServer->getAdvertising()->setMinPreferred(0x06); //6x1.25 = 7.5 ms
  pServer->getAdvertising()->setMaxPreferred(0x0C); //12x1.25 = 15 ms (Apple BLE MIDI spec)

  //  BLESecurity *pSecurity = new BLESecurity();
  //  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  //  pSecurity->setCapability(ESP_IO_CAP_NONE);
  //  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  pServer->getAdvertising()->addServiceUUID(MIDI_SERVICE_UUID);

  // Start advertising
  pServer->getAdvertising()->start();

  for (int i = 0; i < 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }


  for (int i = 0; i < PKT_SIZE; i++) {
    midiPacketLong[i] = i;
  }

  midiPacketLong[0] = 0x80;
  midiPacketLong[1] = 0x80;
  midiPacketLong[2] = 0xF0;

  midiPacketLong[PKT_SIZE - 1] = 0xF7;

  time_0 = millis();
}

bool flip;

void loop() {

  if (deviceConnected) {



    time_elapsed = millis() - time_0;
    if (time_elapsed > 8191)
      time_0 = millis();
    //timetamp high = 10HH HHHH
    byte th = B10000000;
    th = time_elapsed >> 7;
    midiPacket[0] = B10000000 | th;
    //timestamp low = 1LLL LLLL
    byte tl = B01111111 & time_elapsed;
    midiPacket[1] = B10000000 | tl;

    if (digitalRead(D0) == LOW && !armed) {
      armed = true;
      //AppleMIDI.noteOff(84, 0, 1);
    }

    if (digitalRead(D0) == HIGH && armed) {
      //AppleMIDI.noteOn(84, 127, 1);

      delay(1);
      midiPacket[3] = 0x3c; // middle C
      midiPacket[2] = 0x90; // note down, channel 0
      midiPacket[4] = 127;  // velocity
      pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
      pCharacteristic->notify();

      armed = false;
      Serial.println(".");
    }



   


    
    //delay(delay_ms);

    // note up
    /*
        midiPacket[2] = 0x80; // note up, channel 0
        midiPacket[4] = 0;    // velocity
        pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
        pCharacteristic->notify();


        delay(delay_ms);
    */

  }
}
