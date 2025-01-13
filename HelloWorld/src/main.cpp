#include <M5Stack.h>

// Laser analog read setup
#define LASER_PIN 36
unsigned long int laser_val_raw,laser_val_raw_;
float laser_val_raw_ave;
float laser_val;

void setup() {
  M5.begin();
  M5.Power.begin();

  //Laser analog read
  pinMode(LASER_PIN, INPUT);

}

void loop() {
  M5.update();
  M5.Lcd.clear();
  laser_val_raw_ = 0;
  for (int i=0;i<50;i++){
    M5.update();
    laser_val_raw = analogReadMilliVolts(LASER_PIN);
    laser_val_raw_ += laser_val_raw;
    delay(1);
  }
  laser_val_raw_ave = laser_val_raw_ / 50;
  laser_val = (0.085054 * laser_val_raw_ave) - 140.543987;
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.print("Value: [mV]");
  M5.Lcd.println(laser_val_raw_ave);
  M5.Lcd.print("Value: [mm]");
  M5.Lcd.println(laser_val);
  delay(50);
}
