#include <M5stack.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

char buf[2];

//受信データ完了した時の処理
void OnDataRecv(const uint8_t *mac, const uint8_t *recvData, int len) {
  memcpy(&buf[0], recvData, len);
  buf[len]='\0';          //文字列の最後にNULL。
  Serial.print("buf : ");
  Serial.println(buf);
  if(atoi(buf) == 1){     //LED ON
    int color_r = 255 ;
    int color_g = 255 ;
    int color_b = 255 ;
    for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
    pixels.show(); 
    delay(5);
    }   
  }else{  //LED OFF
    int color_r = 0;
    int color_g = 0;
    int color_b = 0;
    for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
    pixels.show(); 
    delay(5);
    }  
  }
}

void setup() {
  M5.begin();
  M5.Power.begin();
  pixels.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);      // Wi-FiをStationモードに設定

  //ESP-NOWの初期化
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);   //ESP-NOWでデータ受信した時のコールバック関数を登録
}

void loop() {
  M5.update();
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.print("MACWIfi: ");
  M5.Lcd.println(WiFi.macAddress());
  M5.Lcd.print("buf: ");
  int buf_ = atoi(buf);
  M5.Lcd.print(buf_);
  delay(1000);

}

