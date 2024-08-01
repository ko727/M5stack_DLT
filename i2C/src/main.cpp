#include <M5Stack.h>

#define M5_SDA 21
#define M5_SDL 22
//Encoder i2C
#define ENCODER_ADDR 0x59
//write
#define ENCODER_PULSE 0x50
#define ENCODER_Z 0x70
//read
#define ENCODER_VALUE 0x00
#define ENCODER_TURNS 0x60

int set_pulse = 2048;
int set_Z = 0;
int data_pulse[4];
int data[4];
int value;


void setup(){
  M5.begin();
  Wire.begin(M5_SDA,M5_SDL);

  //Encoder write
  //pulse
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_PULSE);
  set_pulse = set_pulse*2;
  data_pulse[0] = set_pulse & 0xFF;
  data_pulse[1] = (set_pulse >> 8) & 0xFF;
  data_pulse[2] = (set_pulse >> 16) & 0xFF;
  data_pulse[3] = (set_pulse >> 24) & 0xFF;
  Wire.write(data_pulse[0]);
  Wire.write(data_pulse[1]);
  Wire.write(data_pulse[2]);
  Wire.write(data_pulse[3]);
  Wire.endTransmission(true);
  //Z
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_Z);
  Wire.write(set_Z);
  Wire.endTransmission(true);

  Serial.begin(115200);
}

void loop(){
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_PULSE);
  Wire.endTransmission(true);
  delay(10);

  Wire.requestFrom(ENCODER_ADDR,4);
  for (int i = 0; i < 4; i++){
    data[i] = Wire.read();
  }
  value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

  

  Serial.println(value);

}