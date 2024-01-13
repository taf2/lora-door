//
// using a magnetic contact sensor detect if the sensor is open or closed e.g. door open or closed window etc...
// 
// see: https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio
// see: https://learn.adafruit.com/trinket-bluetooth-alarm-system/code
//
#include <SPI.h>
#include <RH_RF95.h>

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#if defined (__AVR_ATmega32U4__)  // Feather 32u4 w/Radio
  #define RFM95_CS    8
  #define RFM95_INT   7
  #define RFM95_RST   4

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
  #define RFM95_CS    8
  #define RFM95_INT   3
  #define RFM95_RST   4

#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM)  // Feather RP2040 w/Radio
  #define RFM95_CS   16
  #define RFM95_INT  21
  #define RFM95_RST  17

#elif defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM95_CS    4  //
  #define RFM95_INT   3  //
  #define RFM95_RST   2  // "A"

#elif defined(ESP8266)  // ESP8266 feather w/wing
  #define RFM95_CS    2  // "E"
  #define RFM95_INT  15  // "B"
  #define RFM95_RST  16  // "D"

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
  #define RFM95_CS   10  // "B"
  #define RFM95_INT   9  // "A"
  #define RFM95_RST  11  // "C"

#elif defined(ESP32)  // ESP32 feather w/wing
  #define RFM95_CS   33  // "B"
  #define RFM95_INT  27  // "A"
  #define RFM95_RST  13

#elif defined(ARDUINO_NRF52832_FEATHER)  // nRF52832 feather w/wing
  #define RFM95_CS   11  // "B"
  #define RFM95_INT  31  // "C"
  #define RFM95_RST   7  // "A"

#endif

/* Some other possible setups include:

// Feather 32u4:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  7

// Feather M0:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  3

// Arduino shield:
#define RFM95_CS  10
#define RFM95_RST  9
#define RFM95_INT  7

// Feather 32u4 w/wing:
#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  2  // "SDA" (only SDA/SCL/RX/TX have IRQ!)

// Feather m0 w/wing:
#define RFM95_RST 11  // "A"
#define RFM95_CS  10  // "B"
#define RFM95_INT  6  // "D"
*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// status indicator LED
#define LED 13

// magnetic contact sensor
// A1 and A2
#define MAGPIN A1
#define MAGPOUT A2

bool isOpen = false;

void errorState() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}

// radiopacket must be 3 characters- we're sending 4 bytes OOO0 or CCC0
void sendPacket(const char *radiopacket) {
  //Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)radiopacket, 4);

  //Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
}

void sendOpened() {
  sendPacket("OOO");
  digitalWrite(LED, HIGH);
  isOpen = true;
}

void sendClosed() {
  sendPacket("CCC");
  digitalWrite(LED, LOW);
  isOpen = false;
}


void setup() {
  pinMode(LED, OUTPUT);

  pinMode(MAGPIN, INPUT_PULLDOWN);
  pinMode(MAGPOUT, OUTPUT);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  //while (!Serial) delay(1);
  delay(1000);

  digitalWrite(LED, LOW);
  digitalWrite(MAGPOUT, HIGH); // in this way we can detect if the sensor is disconnected because MAGPIN will be LOW otherwise HIGH when connected

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    while (1) {
      Serial.println("LoRa radio init failed");
      Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
      errorState();
    }
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    while (1) {
      Serial.println("setFrequency failed");
      errorState();
    }
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  delay(1000);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
}

unsigned long lastKeepAlive = 0;

void keepAlive() {
  // send a keep alive packet every 10 seconds
  unsigned long delta = millis() - lastKeepAlive;
  if (delta > 10000) {
    if (isOpen) {
      Serial.printf("keep alive[open] %d seconds\n", delta);
      sendClosed();
    } else {
      Serial.printf("keep alive[closed] %d seconds\n", delta);
      sendOpened();
    }
    lastKeepAlive = millis();
  }

}

void loop() {
  int magpin = digitalRead(MAGPIN);

  if (magpin == HIGH) {
    if (isOpen) { lastKeepAlive = millis(); sendClosed(); Serial.println("Closed"); }
    isOpen = false;
  } else {
    if (!isOpen) { lastKeepAlive = millis(); sendOpened(); Serial.println("Opened"); }
    isOpen = true;
  }

  keepAlive();

  delay(100); // Wait 1 second between transmits, could also 'sleep' here!
//  Serial.println("Transmitting..."); // Send a message to rf95_server
}
