#include <M5Stack.h>
#include <mcp_can.h>
#include <Adafruit_NeoPixel.h>

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
    M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

#define CAN0_INT 15   // Set INT to pin 15
MCP_CAN CAN0(12);     // Set CS to pin 12

// 受信したいCAN ID個数を設定する（ボタンを押す毎にスクロールする画面数になる）
#define BUF_NUM 50

// CAN情報
class CANData{
  public:
    long unsigned int canId;
    unsigned char len;
    unsigned char datas[8];
};

static CANData rcv_data[BUF_NUM]; 
static int display_number = 0;

int ds;
int d0,d1,d2,d3;

// CANモジュールの初期化
void init_can(){
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.fillScreen(0x0000);

  // MCP2515の初期化に成功した場合（ビットレート500kb/s ）
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK){
    // 特になにもしない    
  }else{  // 初期化に失敗した場合
    // 特になにもしない    
  }
  // MCP2515を通常モードに設定
  CAN0.setMode(MCP_NORMAL);
  
  // CAN0_INTを入力ピンに設定（受信用）
  pinMode(CAN0_INT, INPUT);

}

//CAN通信データを送信
void sendData(){
  d3 = ds & 0xFF;
  d2 = ds >> 8 & 0xFF;
  d1 = ds >> 16 & 0xFF;
  d0 = ds >> 24 & 0xFF;
  Serial.println(d1);
  byte data[4] = {d0, d1, d2, d3};

  byte sndStat = CAN0.sendMsgBuf(0x06A, 1, 4, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(10);   // send data per 100ms
}

// CAN通信データを受信
void receiveData(){

  long unsigned int temp_canId;
  unsigned char temp_len = 0;
  unsigned char temp_datas[8];
  byte i;
  int flag = 0;
      
  // CAN0_INTピンがLowなら、受信したバッファを読み込む
  if(!digitalRead(CAN0_INT)){

    // 受信データの取得
    CAN0.readMsgBuf(&temp_canId, &temp_len, temp_datas);

    // バッファに格納する。受信済みのCAN IDがあれば上書きする
    for(i = 0;i < BUF_NUM; i++){
      if(temp_canId == rcv_data[i].canId){
        flag = 1;
        break;        
      }else if(rcv_data[i].canId == 0x0000){
        flag = 1;
        break;        
      }else{
        // なにもしない        
      }
    }
    
    // 受信データを格納用バッファへコピー
    if(flag == 1){
      flag = 0;
      rcv_data[i].canId = temp_canId;
      rcv_data[i].len = temp_len;
      for(byte j = 0; j < rcv_data[i].len; j++){
        rcv_data[i].datas[j] = temp_datas[j];
      }
    }

  }
}

// 画面表示
void display_data(){
  
  String data = "None";
  byte i;

  // ボタンAを押す度に、バッファナンバーをマイナス側に変える
  if(M5.BtnA.wasPressed()){
    if(display_number == 0){
      display_number = (BUF_NUM - 1);
    }else{
      display_number--;
    }
    M5.Lcd.fillScreen(0x0000);
  }

  // ボタンCを押す度に、バッファナンバーをプラス側に変える
  if(M5.BtnC.wasPressed()){
    display_number++;

    if(display_number >= BUF_NUM){
      display_number = 0;
    }
    M5.Lcd.fillScreen(0x0000);
  }

  i = display_number;
  
  // CAN IDを表示
  M5.Lcd.drawString("CAN ID:0x" + String(rcv_data[i].canId, HEX)+"   " + String(i),0,0);
  
  // データ（8byte）を表示
  for(byte j = 0; j < rcv_data[i].len; j++){
    data = String(rcv_data[i].datas[j], HEX);
    M5.Lcd.drawString("data" + String(j) + ":0x" + data + "     ", 10, j*20 + 30);
  }

}

void LED_colorbar(){
  pixels.clear();
  if(ds >= 10000){
    pixels.setPixelColor(0, pixels.Color(7, 7, 7));
  }else if(ds >= 20000){
    pixels.setPixelColor(0, pixels.Color(7, 7, 7));
    pixels.setPixelColor(1, pixels.Color(7, 7, 7));
  }else if(ds >= 30000){
    pixels.setPixelColor(0, pixels.Color(7, 7, 7));
    pixels.setPixelColor(1, pixels.Color(7, 7, 7));
    pixels.setPixelColor(2, pixels.Color(7, 7, 7));
  }else if(ds >= 40000){
    pixels.setPixelColor(0, pixels.Color(7, 7, 7));
    pixels.setPixelColor(1, pixels.Color(7, 7, 7));
    pixels.setPixelColor(2, pixels.Color(7, 7, 7));
    pixels.setPixelColor(3, pixels.Color(7, 7, 7));
  }else if(ds >= 50000){
    pixels.setPixelColor(0, pixels.Color(7, 7, 7));
    pixels.setPixelColor(1, pixels.Color(7, 7, 7));
    pixels.setPixelColor(2, pixels.Color(7, 7, 7));
    pixels.setPixelColor(3, pixels.Color(7, 7, 7));
    pixels.setPixelColor(4, pixels.Color(7, 7, 7));
  }else{
    pixels.setPixelColor(5, pixels.Color(10, 10, 10));
    pixels.setPixelColor(6, pixels.Color(10, 10, 10));
    pixels.setPixelColor(7, pixels.Color(10, 10, 10));
    pixels.setPixelColor(8, pixels.Color(10, 10, 10));
    pixels.setPixelColor(9, pixels.Color(10, 10, 10));
  }
  pixels.show();
}

void setup() {
  pixels.begin();

  M5.begin();
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  delay(500);
  M5.Lcd.setTextSize(2);
  init_can();

  // バッファをクリアしておく
  for(byte i = 0; i < BUF_NUM ; i++){
    rcv_data[i].canId = 0x00;
    rcv_data[i].len = 0x08;
    for(byte j = 0; j < rcv_data[i].len; j++){
      rcv_data[i].datas[j] = 0x00;
    }
  }

}

void loop() {
  if(M5.BtnB.wasPressed()){
    ds = ds + 1000;
  }
  sendData();

  receiveData();
  
  display_data();

  LED_colorbar();
   
  M5.update();
  
}