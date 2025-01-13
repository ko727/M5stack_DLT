#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>

int mode_num, mode_num_;
float ts_time;

int set_position;

//Trigger logger
#define TRIGGER_LOGGER 26

//M5GO LED Bar
#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
int color_r;
int color_g;
int color_b;

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

void OnDataRecv(const uint8_t *mac, const uint8_t *recvData, int len) {
  memcpy(&buf[0], recvData, len);
  buf[len]='\0';
  set_position = atoi(buf);
  delay(5);
}


int MC_short = 200;
int MC_long = MC_short * 3;

void MorseCode_short(){
  color_r = 255 ;
  color_g = 255 ;
  color_b = 255 ;
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS-1; i++) {
    if (i == 4){
    }else{
      pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
      pixels.show(); 
      delay(1);
    }
  }
  delay(MC_short);
  color_r = 0 ;
  color_g = 0 ;
  color_b = 0 ;
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS-1; i++) {
    if (i == 4){
    }else{
      pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
      pixels.show(); 
      delay(1);
    }
  }
}

void MorseCode_long(){
  color_r = 255 ;
  color_g = 255 ;
  color_b = 255 ;
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS-1; i++) {
    if (i == 4){
    }else{
      pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
      pixels.show(); 
      delay(1);
    }
  }
  delay(MC_long);
  color_r = 0 ;
  color_g = 0 ;
  color_b = 0 ;
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS-1; i++) {
    if (i == 4){
    }else{
      pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
      pixels.show(); 
      delay(1);
    }
  }
}

void Color_LED(int c_r, int c_g, int c_b){
  pixels.setPixelColor(4, pixels.Color(c_r, c_g, c_b)); 
  pixels.show(); 
  delay(1);
  pixels.setPixelColor(9, pixels.Color(c_r, c_g, c_b)); 
  pixels.show(); 
  delay(1);
}

void setup() {
  M5.begin();
  M5.Power.begin();
  M5.update();

  pinMode(TRIGGER_LOGGER, OUTPUT);

  //WiFi OTA
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Use OTA, press A");
  M5.Lcd.println("SLT slave, don't press"); //mode_num = 200
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
  }else{
    mode_num = 1;
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
  switch (mode_num){
  case 1:  //SLT slave mode
    if(set_position > 0){
      mode_num_ = mode_num;
      mode_num = 2;
      ts_time = millis();
    }
    break;
  
  case 2: //Start logging
    digitalWrite(TRIGGER_LOGGER,HIGH);
    delay(200);
    digitalWrite(TRIGGER_LOGGER,LOW);
    mode_num = 3;
    break;
  
  case 3:
    if(millis() - ts_time > 5000){
      mode_num = set_position;
    }
    break;
  
  case 4://End logging 4-->5
    if(millis() - ts_time > 20000){
      mode_num = 5;
    }
    break;
  
  case 5: //(Turn off LED)
    mode_num = 1;
    set_position = 0;
    color_r = 0 ;
    color_g = 0 ;
    color_b = 0 ;
    for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(color_r, color_g, color_b)); 
      pixels.show(); 
      delay(1);
    }
    break;

  case 10:  //Distance 10[mm] .----(1)
    Color_LED(255,0,0);
    MorseCode_short();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_long);
    mode_num = 4;
    break;
  
  case 20:  //Distance 20[mm] ..---(2)
    Color_LED(0,255,0);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_long);
    mode_num = 4;
    break;
  
  case 30:  //Distance 30[mm] ...--(3)
    Color_LED(0,0,255);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_long();
    delay(MC_short);
    MorseCode_long();
    delay(MC_long);
    mode_num = 4;
    break;
  
  case 40:  //Distance 40[mm] ....-(4)
    Color_LED(255,255,0);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_long();
    delay(MC_long);
    mode_num = 4;
    break;
  
  case 50:  //Distance 50[mm] .....(5)
    Color_LED(255,0,255);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_long);
    mode_num = 4;
    break;
  
  case 60:  //Distance 60[mm] -....(6)
    Color_LED(0,255,255);
    MorseCode_long();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_short);
    MorseCode_short();
    delay(MC_long);
    mode_num = 4;
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