#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <mcp_can.h>

int mode_num = 0;
long timer,tm,tm_;

// WiFi credentials.
// Set password to "" for open networks.
// #1 712, #2 702C, #3 mobile router
char ssid1[] = "Buffalo-G-9AC8";
char pass1[] = "rd66td4bde67s";
char ssid2[] = "aterm-cfc4b1-5p";
char pass2[] = "23b2c159372b8";
char ssid3[] = "Galaxy_5GMW_3739";
char pass3[] = "vrcl3968";

int wifi_cnt;
int wifi_num = 1;
bool OTA_flag = false;

// Interrupt setup
#define M5_Interrupt1 2
#define M5_Interrupt2 5

// I2C Encoder
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

int set_pulse = 512;
int set_Z = 0;
volatile float perimeter;
int data_pulse[4];
int data[4];
volatile int value;
int value_1st;
int set_position_mode;

// CAN setup
#define CAN_vesc_IDaddress 0x019
#define CAN_dji_IDaddress 0x200
#define CAN0_INT 15
MCP_CAN CAN0(12);
int ds;

// Limit switch 割り込み関数
volatile int stoper;

void IRAM_ATTR onRise1() {
  stoper = 1;
}
void IRAM_ATTR onRise2() {
  stoper = 2;
}

void init_can(){
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK){  
  }else{ 
  }
  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);
}

void sendData_dji(int ds_){
  //-10000~10000[mA]
  int d1 = ds_ & 0xFF;
  int d0 = ds_ >> 8 & 0xFF;
  byte data[8] = {(byte)d0, (byte)d1, 0,0,0,0,0,0};
  byte sndStat = CAN0.sendMsgBuf(CAN_dji_IDaddress, 0, 8, data);
}

void sendData_vesc(int ds_){
  //-10000~10000[%/100]
  int d3 = ds_ & 0xFF;
  int d2 = ds_ >> 8 & 0xFF;
  int d1 = ds_ >> 16 & 0xFF;
  int d0 = ds_ >> 24 & 0xFF;
  byte data[4] = {(byte)d0, (byte)d1, (byte)d2, (byte)d3};
  byte sndStat = CAN0.sendMsgBuf(CAN_vesc_IDaddress, 1, 4, data);
}


void setup() {
  M5.begin();
  M5.Power.begin();
  mode_num = 10;

  //Interrupt set
  pinMode(M5_Interrupt1,INPUT_PULLDOWN);
  attachInterrupt(M5_Interrupt1,onRise1,HIGH);
  pinMode(M5_Interrupt2,INPUT_PULLDOWN);
  attachInterrupt(M5_Interrupt2,onRise2,HIGH);

  //Set CAN
  init_can();

  //Encoder write
  Wire.begin(M5_SDA,M5_SDL);
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
  //Read encoder value
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_VALUE);
  Wire.endTransmission(true);
  delay(5);
  Wire.requestFrom(ENCODER_ADDR,4);
  for (int i = 0; i < 4; i++){
    data[i] = Wire.read();
  }
  value_1st = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

  //WiFi OTA
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Use OTA, press A");
  M5.Lcd.println("Set position, press B");
  while(wifi_cnt < 20){
    wifi_cnt++;
    delay(500);
    M5.Lcd.printf(".");
    M5.update();
    if (M5.BtnA.isPressed()){
      OTA_flag = true;
      M5.Lcd.setCursor(0,60);
      M5.Lcd.printf("OTA mode");
      delay(500);
      break;
    }
    if (M5.BtnB.isPressed()){
      set_position_mode = 1;
      M5.Lcd.setCursor(0,60);
      M5.Lcd.printf("Set position mode");
      delay(500);
      break;
    }
  }
  M5.Lcd.clear();
  if(OTA_flag == true){
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Connecting 712 room");
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid1, pass1);
    while (WiFi.status() != WL_CONNECTED){
      wifi_cnt++;
      M5.Lcd.printf(".");
      if (wifi_cnt > 40){
        wifi_cnt = 0;
        wifi_num = 2;
        WiFi.disconnect();
        delay(10);

        break;
      }
      delay(200);
    }
    if (wifi_num == 2){
      M5.Lcd.setCursor(0,30);
      M5.Lcd.printf("Conecting 702C room");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid2, pass2);
      while (WiFi.status() != WL_CONNECTED){
        wifi_cnt++;
        M5.Lcd.printf(".");
        if (wifi_cnt > 40){
          wifi_cnt = 0;
          wifi_num = 3;
          WiFi.disconnect();
          delay(10);

          break;
        }
        delay(200);
      }
    }
    if (wifi_num == 3){
      M5.Lcd.setCursor(0,60);
      M5.Lcd.printf("Conecting mobile router");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid3, pass3);
      while (WiFi.status() != WL_CONNECTED){
        wifi_cnt++;
        M5.Lcd.printf(".");
        if (wifi_cnt > 40){
          wifi_cnt = 0;
          wifi_num = 4;
          WiFi.disconnect();
          delay(10);
        
          break;
        }
        delay(200);
      }
    }
    M5.Lcd.clear();
    if (WiFi.status() == WL_CONNECTED){
      mode_num = 100;
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("WiFi Connected!!");
      M5.Lcd.print("SSID: ");
      M5.Lcd.println(WiFi.SSID());
      M5.Lcd.print("IP address: ");
      M5.Lcd.println(WiFi.localIP());

      ArduinoOTA
      .setHostname("M5Core")
      .onStart([]() {})
      .onEnd([]() {})
      .onProgress([](unsigned int progress, unsigned int total) {})
      .onError([](ota_error_t error) {});   // Set the network port name. 
      ArduinoOTA.begin();            // Initialize the OTA.
      M5.Lcd.println("OTA ready!");  // M5.lcd port output format string.

    }else{
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("WiFi NOT Connected!!");
      OTA_flag = false;
      delay(1000);
      M5.Power.reset();
    }
  }else{
  }
}

void loop() {
  delay(1);
  tm_ = millis();t
  timer = tm_ - tm;
  if(stoper == 1){
    mode_num = 10;
    tm = tm_ - 40000;
    stoper = 0;
  }else if(stoper == 2){
    mode_num = 10;
  }
  else if(timer > 10000 && timer < 30000){//up
    mode_num = 20;
  }else if(timer > 30000 && timer < 40000){//stop
    mode_num = 10;
  }else if(timer > 40000 && timer < 60000){//down
    mode_num = 30;
  }

  switch (mode_num){
  case 0:
    tm = millis();
    break;
  case 10:
    sendData_dji(-2000);
    sendData_vesc(0);
    break;
  
  case 20:
    sendData_dji(2000);
    sendData_vesc(5000);
    break;

  case 30:
    sendData_dji(2000);
    sendData_vesc(-5000);
    break;

  case 100://OTA program
    if(OTA_flag == true){
      ArduinoOTA.handle();
      M5.Lcd.setCursor(0,150);
      M5.Lcd.println("Stop OTA, press C");
      M5.update();
      if (M5.BtnC.isPressed()) {
        ArduinoOTA.end();
        M5.Lcd.println("OTA End!");
        OTA_flag = false;
        delay(3000);
        M5.Lcd.clear();
      }
      delay(50);
    }else{
    }
    break;
    
  default:
    break;
  }
}