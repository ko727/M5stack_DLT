#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <mcp_can.h>
#include <esp_now.h>

int mode_num, mode_num_;
float ts_time;

// WiFi credentials.
// Set password to "" for open networks.
// #1 712, #2 702C, #3 mobile router
char ssid1[] = "Buffalo-4740";
char pass1[] = "m55ks3xdv6buk";
char ssid2[] = "aterm-cfc4b1-5p";
char pass2[] = "23b2c159372b8";
char ssid3[] = "FS030W_P473654";
char pass3[] = "37052573";

int wifi_cnt;
int wifi_num = 1;
bool OTA_flag = false;

// ESP-NOW
char buf[2];

// Laser analog read setup
#define LASER_PIN 36
unsigned long int laser_val_raw,laser_val_raw_;
float laser_val_raw_ave;
float laser_val,laser_val_,laser_val_st;

// Limit Switch setup
#define M5_Limitsw1 22

int set_position_mode;
int set_position;

// CAN setup
#define CAN_dji_IDaddress 0x1FF
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

void IRAM_ATTR onRise1() {
  mode_num = 31;
}

void sendData_dji(int ds_){
  //-25000~25000[mV]
  int d1 = ds_ & 0xFF;
  int d0 = ds_ >> 8 & 0xFF;
  byte data[8] = {(byte)d0, (byte)d1, (byte)d0, (byte)d1, (byte)d0, (byte)d1, (byte)d0, (byte)d1};
  byte sndStat = CAN0.sendMsgBuf(CAN_dji_IDaddress, 0, 8, data);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *recvData, int len) {
  memcpy(&buf[0], recvData, len);
  buf[len]='\0';
  set_position = atoi(buf);
  delay(5);
}

void setup() {
  M5.begin();
  M5.Power.begin();

  //Laser analog read
  pinMode(LASER_PIN, INPUT);

  //Limit Switch
  pinMode(M5_Limitsw1, INPUT_PULLUP);
  attachInterrupt(M5_Limitsw1,onRise1,RISING);
  
  //Set CAN
  init_can();
  
  //WiFi OTA
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Use OTA, press A");
  M5.Lcd.println("Set position, press B"); //mode_num = 50
  M5.Lcd.println("Manual, press C"); //mode_num = 40
  M5.Lcd.println("SLT slave, don't press"); //mode_num = 10
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
    if (M5.BtnC.isPressed()){
      mode_num = 40;
      M5.Lcd.setCursor(0,60);
      M5.Lcd.printf("Manual mode");
      delay(500);
      break;
    }
  }

  M5.Lcd.clear();

  if(OTA_flag == true){
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Connecting HOME");
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
  }else if(set_position_mode == 1){
    mode_num = 50;
    mode_num_ = mode_num;
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
        set_position = set_position + 1;
      }else if(M5.BtnB.isPressed()){
        set_position = set_position - 1;
      }else if(M5.BtnC.isPressed()){
        break;
      }
      M5.Lcd.setCursor(90,100);
      M5.Lcd.print(set_position);
      M5.Lcd.setCursor(180,100);
      M5.Lcd.print("[mm]");
      delay(100);
    }
    M5.Lcd.clear();

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Target: ");
    M5.Lcd.print(set_position);
    M5.Lcd.print("[mm]");

    delay(100);
    laser_val_raw_ = 0;
    for (int i=0;i<50;i++){
      laser_val_raw = analogReadMilliVolts(LASER_PIN);
      laser_val_raw_ += laser_val_raw;
      delay(1);
    }
    laser_val_raw_ave = laser_val_raw_ / 50;
    laser_val_st = (0.085054 * laser_val_raw_ave) - 140.543987;

    delay(1000);
    
  }else if(mode_num == 40){
  }else{
    mode_num = 10;
    WiFi.mode(WIFI_STA); 
    if (esp_now_init() != ESP_OK) {
      //Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.print("MACWIfi: ");
    M5.Lcd.print(WiFi.macAddress());
  }
}

void loop() {
  M5.update();
  laser_val_raw_ = 0;
  for (int i=0;i<100;i++){
    laser_val_raw = analogReadMilliVolts(LASER_PIN);
    laser_val_raw_ += laser_val_raw;
    delay(1);
  }
  laser_val_raw_ave = laser_val_raw_ / 100;
  laser_val = (0.085054 * laser_val_raw_ave) - 140.543987;

  switch (mode_num){
  case 10:  //SLT slave mode
    if(set_position > 0){
      laser_val_st = laser_val;
      mode_num_ = mode_num;
      mode_num = 11;
      ts_time = millis();
    }
    break;
  
  case 11:
    if(millis() - ts_time > 5000){
      mode_num = 50;
    }
    break;
  
  case 30:  // [ALL] Go back Origin
    if(digitalRead(M5_Limitsw1) == HIGH){
    }else{
      sendData_dji(24000);
      if(set_position_mode == 1){
        mode_num_ = 30;
      }
    }
    break;
  
  case 31:  // [ALL] STOP limit
    if(digitalRead(M5_Limitsw1) == HIGH){
    }else{
      sendData_dji(0);
    }
    if (mode_num_ == 10){
      mode_num = mode_num_;
    }else if(mode_num_ == 40){
      mode_num = mode_num_;
    }else if(mode_num_ == 30){
      mode_num = 40;
    }else if (set_position_mode == 1){
      mode_num = 50;
    }
    break;
  
  case 40:  //Manual mode
    mode_num_ = mode_num;
    if(M5.BtnA.isPressed()){
      sendData_dji(-24000);
    }else if(digitalRead(M5_Limitsw1) == HIGH){
    }else if(M5.BtnC.isPressed()){
      sendData_dji(24000);
    }
    break;

  case 50:  //Set position
    sendData_dji(-24000);
    laser_val_ = laser_val_st - laser_val;
    if(laser_val_ > set_position){
      sendData_dji(0);
      ts_time = millis();
      mode_num = 51;
    }
    break;
  
  case 51:
    sendData_dji(0);
    if(millis() - ts_time > 10000){
      set_position = 0;
      mode_num = 30;
    }
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