#include <M5Stack.h>

int test;

void IRAM_ATTR onRise() {
  test++;
}

void setup() {
  M5.begin();
  M5.Lcd.begin();
  Serial.begin(115200);
  pinMode(5,INPUT_PULLDOWN);
  attachInterrupt(5,onRise,RISING);
}

void loop() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(ORANGE,BLACK);
  M5.Lcd.setTextFont(4);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1); 
  M5.Lcd.println(test);
  delay(10);
}


