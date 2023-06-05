#include <dht.h>

#define DHTPIN 2      // Pin del sensor DHT11
#define LEDPIN 13     // Pin del LED

dht DHT;

void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  int chk = DHT.read11(DHTPIN);
  float temp = DHT.temperature;
  
  if (temp > 28) {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW);
  }
