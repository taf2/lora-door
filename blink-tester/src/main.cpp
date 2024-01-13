/* tester make sure the board is there and working well enough to blink */
#include <Arduino.h>
#define LED A3

void setup() {
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  //while (!Serial) delay(1);
  delay(1000);

  digitalWrite(LED, LOW);
}

void loop() {

  delay(1000);
  digitalWrite(LED, HIGH);

  Serial.println("Hello World!");

  delay(1000);
  digitalWrite(LED, LOW);

}
