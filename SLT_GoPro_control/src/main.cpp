#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <esp_now.h>
#include <mcp_can.h>

//調べた受信側のMACアドレスを設定
uint8_t broadcastAddress1[] = {0x2C, 0xBC, 0xBB, 0x81, 0xF8, 0x60};
uint8_t broadcastAddress2[] = {0x2C, 0xBC, 0xBB, 0x81, 0xF8, 0xFC};
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


uint8_t DATA[2];
char buf[2];
int temp;
int mode_num, mode_num_;

esp_now_peer_info_t peerInfo;   //受信側情報のパラメータオブジェクト

//送信が完了した時の処理
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == ESP_NOW_SEND_SUCCESS){
    Serial.println("Deli_Success");
    mode_num = 10;
  }else{
    Serial.println("Deli_Fail");
    mode_num = 1;
  }
}

// CAN setup
#define CAN_dji_IDaddress 0x200
#define CAN0_INT 15
MCP_CAN CAN0(12);
int ds;
float ts_time;

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

// Limit Switch setup
#define M5_Limitsw_read 36

void IRAM_ATTR onRise1() {
  mode_num = 20;
}
 
void setup() {
  M5.begin();
  M5.Power.begin();
  Serial.begin(115200);
  Serial.setTimeout(5000);
  WiFi.mode(WIFI_STA);            // Wi-FiをStationモードに設定

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("If you press GoPro Btn, press B");

  //Limit Switch
  pinMode(M5_Limitsw_read, INPUT_PULLDOWN);
  attachInterrupt(M5_Limitsw_read,onRise1,RISING);
  
  //Set CAN
  init_can();

  //ESP-NOWの初期化
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }else{
    mode_num = 1;
  }
  
  //ペア情報（受信側のアドレスとチャネルと暗号化の設定）
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  //受信側の情報追加      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_send_cb(OnDataSent);   //ESP-NOWでデータ送信した時のコールバック関数を登録
}
 
void loop() {
  M5.update();

  if(M5.BtnB.wasPressed()){
    mode_num = 10;
  }
  switch (mode_num){
  case 1:
    Serial.println("Set distance [mm]");
    mode_num = 2;
    break;
  
  case 2:
    if(Serial.available() > 0){
      String text = Serial.readStringUntil('\n');
      Serial.print("talk:");
      Serial.println(text);
      temp = atoi(text.c_str());
      if(temp % 10 == 0){
        sprintf(buf,"%d",temp); //数値型を文字型に変換
        memcpy(DATA,buf,strlen(buf)); //uint8_tのLED変数に値をコピー
        
        esp_err_t result = esp_now_send(broadcastAddress, DATA, sizeof(buf)); //ESP-NOWのデータ送信処理
      
        if (result == ESP_OK) {
          Serial.println("success");
          Serial.print("text : ");
          Serial.println(text);
        }
        else {
          Serial.println("Error");
          mode_num = 1;
        }
        delay(500);
      }else{
        Serial.println("Can't set distance!");
        Serial.println("Please use x10[mm]");
        mode_num = 1;
      }
    }else{
    }
    break;
  
  case 10:
    sendData_dji(500);
    break;
  
  case 20:
    ts_time = millis();
    mode_num = 21;
    break;

  case 21:
    sendData_dji(-500);
    if(millis() - ts_time > 2000){
      sendData_dji(0);
      mode_num = 1;
    }
    break;
  
  default:
    break;
  }
}