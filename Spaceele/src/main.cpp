#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <mcp_can.h>

int mode_num = 0;
float ts_time;
int run_mode; //10(Virtical),20(Incline)

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

// Limit Switch setup
#define M5_Limitsw1 5
#define M5_Limitsw2 2

// Limit switch 割り込み関数
void IRAM_ATTR onRise1() {
  mode_num = 11;
  ts_time = millis();
}
void IRAM_ATTR onRise2() {
  mode_num = 10;
  ts_time = millis();
}

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
float perimeter;
int data_pulse[4];
int data[4];
int value;
int value_1st;
int set_position_mode;
float set_position, set_position_;
#define DIAMETAR 30
#define ENCODER_P 512

// CAN setup
#define CAN_vesc_IDaddress_A 0x051 //VESC ID = 81(A) 0x051
#define CAN_vesc_IDaddress_B 0x022 //VESC ID = 34(B) 0x022
#define CAN_vesc_IDaddress_C 0x074 //VESC ID = 116(C) 0x074
#define CAN_dji_IDaddress 0x200
#define CAN0_INT 15
MCP_CAN CAN0(12);
int ds;

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
  byte data[8] = {(byte)d0, (byte)d1, (byte)d0, (byte)d1, (byte)d0, (byte)d1, (byte)d0, (byte)d1};
  byte sndStat = CAN0.sendMsgBuf(CAN_dji_IDaddress, 0, 8, data);
}

void sendData_vesc(int ds_, int id){
  //-10000~10000[%/100]
  int d3 = ds_ & 0xFF;
  int d2 = ds_ >> 8 & 0xFF;
  int d1 = ds_ >> 16 & 0xFF;
  int d0 = ds_ >> 24 & 0xFF;
  byte data[4] = {(byte)d0, (byte)d1, (byte)d2, (byte)d3};
  byte sndStat = CAN0.sendMsgBuf(id, 1, 4, data);
}


void setup() {
  M5.begin();
  M5.Power.begin();

  //Limit Switch
  pinMode(M5_Limitsw1, INPUT_PULLDOWN);
  pinMode(M5_Limitsw2, INPUT_PULLDOWN);
  //Interrupt set
  attachInterrupt(M5_Limitsw1,onRise1,RISING);
  attachInterrupt(M5_Limitsw2,onRise2,RISING);

  //Set CAN
  init_can();

  //Encoder write
  Wire.begin(M5_SDA,M5_SDL);
  //pulse
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_PULSE);
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
  M5.Lcd.println("Brake free, don't press");
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
  
  if(set_position_mode == 1){
    mode_num = 15;
    M5.Lcd.clear();
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println("Press A, UP --- Press B, DOWN");
    M5.Lcd.println("If you set target, Press C");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,100);
    M5.Lcd.print("Target: ");
    while(1){
      M5.update();
      M5.Lcd.fillRect(100,90,80,20, BLACK);
      if(M5.BtnA.isPressed()){
        set_position = set_position + 1000;
      }else if(M5.BtnB.isPressed()){
        set_position = set_position - 1000;
      }else if(M5.BtnC.isPressed()){
        break;
      }
      M5.Lcd.setCursor(90,100);
      M5.Lcd.print(set_position/1000);
      M5.Lcd.setCursor(180,100);
      M5.Lcd.print("[m]");
      delay(100);
    }
    M5.Lcd.clear();

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Target: ");
    M5.Lcd.print(set_position/1000);
    M5.Lcd.print("[m]");

    M5.Lcd.setCursor(0,80);
    M5.Lcd.println("Press A, Virtical");
    M5.Lcd.println("Press C, Incline");
    delay(100);
    while (1){
      M5.update();
      delay(100);
      if(M5.BtnA.isPressed()){
        run_mode = 10;
        break;
      }else if(M5.BtnC.isPressed()){
        run_mode = 20;
        break;
      }
    }
  }else{
    mode_num = 40;
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
  M5.update();
//Read Encoder value
  Wire.beginTransmission(ENCODER_ADDR);
  Wire.write(ENCODER_VALUE);
  Wire.endTransmission(true);
  delay(1);
  Wire.requestFrom(ENCODER_ADDR,4);
  for (int i = 0; i < 4; i++){
    data[i] = Wire.read();
  }
  value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
  perimeter = (value - value_1st) * DIAMETAR * 3.14 / (set_pulse*4);

  switch (mode_num){

  case 10: //Limit Switch Bottom side(STOP Unlimited)
    sendData_dji(-2000);
    sendData_vesc(0,CAN_vesc_IDaddress_A);
    sendData_vesc(0,CAN_vesc_IDaddress_B);
    sendData_vesc(0,CAN_vesc_IDaddress_C);   
    break;

  case 11: //Limit Switch Up side (STOP --> DOWN) 11-->12
    ts_time = millis();
    mode_num = 12;
    break;

  case 12: //Limit Switch Up side (STOP --> DOWN)
    sendData_dji(-2000);
    sendData_vesc(0,CAN_vesc_IDaddress_A);
    sendData_vesc(0,CAN_vesc_IDaddress_B);
    sendData_vesc(0,CAN_vesc_IDaddress_C);
    if (millis() - ts_time > 5000){
      if(run_mode == 10){
        mode_num = 30;
      }else if(run_mode == 20){
        mode_num = 35;
      }
    }
    break;
  
  case 15: //Start RUN 15-->16-->25
    ts_time = millis();
    mode_num = 16;
    break;
  
  case 16: //Stop 5sec --> UP   16-->25
    sendData_dji(-2000);
    sendData_vesc(0,CAN_vesc_IDaddress_A);
    sendData_vesc(0,CAN_vesc_IDaddress_B);
    sendData_vesc(0,CAN_vesc_IDaddress_C);
    if(millis() - ts_time > 5000){
      mode_num = 25;
      set_position_ = set_position * 0.2;
    }
    break;

  case 20: //UP Unlimited
    sendData_dji(2000);
    sendData_vesc(-10000,CAN_vesc_IDaddress_A);
    sendData_vesc(-10000,CAN_vesc_IDaddress_B);
    sendData_vesc(-10000,CAN_vesc_IDaddress_C);
    break;
  
  case 25: //UP Target 0-20% 25-->26
    sendData_dji(2000);
    sendData_vesc(-5000,CAN_vesc_IDaddress_A);
    sendData_vesc(-5000,CAN_vesc_IDaddress_B);
    sendData_vesc(-5000,CAN_vesc_IDaddress_C);
    if(perimeter > set_position_){
      mode_num = 26;
      set_position_ = set_position * 0.8;
    }
    break;
  
  case 26: //UP Target 20-80%  26-->27
    sendData_dji(2000);
    sendData_vesc(-10000,CAN_vesc_IDaddress_A);
    sendData_vesc(-10000,CAN_vesc_IDaddress_B);
    sendData_vesc(-10000,CAN_vesc_IDaddress_C);
    if(perimeter > set_position_){
      mode_num = 27;
    }
    break;
  
  case 27: //UP Target 80-100%  27-->11
    sendData_dji(2000);
    sendData_vesc(-5000,CAN_vesc_IDaddress_A);
    sendData_vesc(-5000,CAN_vesc_IDaddress_B);
    sendData_vesc(-5000,CAN_vesc_IDaddress_C);
    if(perimeter > set_position){
      mode_num = 11;
    }
    break;

  case 30: //DOWN Unlimited(Vertical) 30-->31
    ts_time = millis();
    mode_num = 31;
    break;

  case 31:
    sendData_dji(-2000);
    sendData_vesc(0,CAN_vesc_IDaddress_A);
    sendData_vesc(0,CAN_vesc_IDaddress_B);
    sendData_vesc(0,CAN_vesc_IDaddress_C);
    if(ts_time - millis() > 10000){
      sendData_dji(2000);
      sendData_vesc(0,CAN_vesc_IDaddress_A);
      sendData_vesc(0,CAN_vesc_IDaddress_B);
      sendData_vesc(0,CAN_vesc_IDaddress_C);
      if(ts_time - millis() > 10100){
        ts_time = millis();
      }
    }
    break;
  
  case 35: //DOWN Target 100-10% and 10-0% (Incline) 35-->36-->37-->10
    ts_time = millis();
    mode_num = 36;
    set_position_ = set_position * 0.1;
    break;
  
  case 36: //DOWN Target 100-10%(Incline) 36-->37
    sendData_dji(2000);
    sendData_vesc(10000,CAN_vesc_IDaddress_A);
    sendData_vesc(10000,CAN_vesc_IDaddress_B);
    sendData_vesc(10000,CAN_vesc_IDaddress_C);
    if(perimeter < set_position_){
      mode_num = 37;
    }
    break;
  
  case 37: //DOWN Target 10-0%(Incline) 37-->10
    sendData_dji(2000);
    sendData_vesc(5000,CAN_vesc_IDaddress_A);
    sendData_vesc(5000,CAN_vesc_IDaddress_B);
    sendData_vesc(5000,CAN_vesc_IDaddress_C);
    if(perimeter < 0){
      mode_num = 10;
    }
    break;
  
  case 40: //Debug mode (break free)
    sendData_dji(2000);
    sendData_vesc(0,CAN_vesc_IDaddress_A);
    sendData_vesc(0,CAN_vesc_IDaddress_B);
    sendData_vesc(0,CAN_vesc_IDaddress_C);
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