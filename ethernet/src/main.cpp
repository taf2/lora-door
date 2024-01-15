// testing ethernet
#include <SPI.h>
#include "Ethernet.h"
#include <SSLClient.h>
#include "www_adafruit_com_trust.h"

// status indicator LED
#define LED 46

/* ethernet interface */
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
const int rand_pin = 17;
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
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      delay(1000); // do nothing, no point running without Ethernet hardware
    }

    while (Ethernet.linkStatus() == LinkOFF) {
      digitalWrite(LED, HIGH);
      delay(100);
      Serial.println("Ethernet cable is not connected.");
      digitalWrite(LED, LOW);
      delay(100);
    }

    while (!Ethernet.localIP()) {
      digitalWrite(LED, HIGH);
      delay(100);
      Serial.println("DHCP not successful");
      digitalWrite(LED, LOW);
      delay(100);
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

  Serial.begin(115200);
  //while (!Serial) delay(1);
  delay(1000);

  digitalWrite(LED, LOW);

  for ( int i = 0; i < 10; i++ ) {
    digitalWrite(LED, HIGH);
    delay(100);
    Serial.println("Hello, world!");
    digitalWrite(LED, LOW);
    delay(100);
  }

  // when we are ready we can connect to our house server and send a message or track open/closed messages for the door
  setupEthernet();

  digitalWrite(LED, HIGH);
  delay(2000);
  digitalWrite(LED, LOW);
  delay(1000);
}

void loop() {

  delay(1000);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
}
