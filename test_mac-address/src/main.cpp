#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.print("MACWIfi: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}