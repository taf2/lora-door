// see: https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio
// modified to only be a receiver listening for the O signal or the C signal based on the received signal toggle LED on or off
//
// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include "Ethernet.h"
#include <SSLClient.h>
#include "www_adafruit_com_trust.h"

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
#define LED_OPEN 14
#define LED_CLOSED 16

/* ethernet interface */
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
const int rand_pin = A5;
EthernetClient base_client;
SSLClient client(base_client, WWW_ADAFRUIT_COM_TAs, (size_t)WWW_ADAFRUIT_COM_TAs_NUM, rand_pin);
#define MAX_RESPONSE_SIZE 1024

String httpsGet(const char *server, const char *path, int port=443) {
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.print("connected to ");
    Serial.println(base_client.remoteIP());
    // send GET request to the path
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.1");
    // set the Host header to the server
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    Serial.println("request sent");
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
  // get the response into the String and return
  String response = "";
  while (client.connected() && response.length() < MAX_RESPONSE_SIZE) {
    if (client.available()) {
      char c = client.read();
      response += c;
      Serial.print("reading bytes: ");
      Serial.println(response.length());
    }
  }
  client.stop();
  return response;
}

void setupEthernet() {
  Ethernet.init(10); // M0 lora radio featherwing

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      while (true) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        delay(1000); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      while (true) {
        Serial.println("Ethernet cable is not connected.");
        delay(1000); // do nothing, no point running without Ethernet hardware
      }
    }
    while (true) {
      Serial.println("DHCP not successful");
      delay(1000); // do nothing, no point running without Ethernet hardware
    }
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  // test the ethernet connection
  char server[] = "www.adafruit.com"; // name address for adafruit (using DNS)

  String response = httpsGet(server, "/");

  Serial.println("response:");
  Serial.println(response);
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  //while (!Serial) delay(1);
  delay(1000);

  digitalWrite(LED, LOW);
  digitalWrite(LED_OPEN, LOW);
  digitalWrite(LED_CLOSED, HIGH);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    while (1) {
      Serial.println("LoRa radio init failed");
      Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
      delay(1000);
    }
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);


  // when we are ready we can connect to our house server and send a message or track open/closed messages for the door
  //setupEthernet();

  digitalWrite(LED_OPEN, HIGH);
  digitalWrite(LED, HIGH);
  delay(2000);
  digitalWrite(LED_OPEN, LOW);
  digitalWrite(LED, LOW);
  delay(1000);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

uint8_t *opened_message = (uint8_t*)"OOO";
uint8_t *closed_message = (uint8_t*)"CCC";

void loop() {
  //delay(1000); // Wait 1 second between transmits, could also 'sleep' here!

  // wait for a messages from our peer
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  //Serial.println("Waiting for anything...");
  if (rf95.waitAvailableTimeout(1000)) {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      if (memcmp(opened_message, buf, 3) == 0) {
        digitalWrite(LED, HIGH);
        digitalWrite(LED_OPEN, HIGH);
        digitalWrite(LED_CLOSED, LOW);
      } else if (memcmp(closed_message, buf, 3) == 0) {
        digitalWrite(LED, LOW);
        digitalWrite(LED_OPEN, LOW);
        digitalWrite(LED_CLOSED, HIGH);
      }
    } else {
      Serial.println("Receive failed");
    }
  }// else {
    //Serial.println("No reply, is there a listener around?");
  //}

}
