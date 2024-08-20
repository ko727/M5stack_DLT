#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>

// WiFi credentials.
// Set password to "" for open networks.
//#1 712, #2 702C, #3 mobile router
char ssid1[] = "elecom-ab5003";
char pass1[] = "5njiuhcuth7c";
char ssid2[] = "aterm-cfc4b1-5p";
char pass2[] = "23b2c159372b8";
char ssid3[] = "Galaxy_5GMW_3739";
char pass3[] = "vrcl3968";

int wifi_cnt;
int wifi_num = 1;
bool OTA_flag = false;

void setup() {
  M5.begin();
  M5.Power.begin();

  //WiFi OTA
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.printf("Use OTA, press A");
  while(wifi_cnt < 20){
    wifi_cnt++;
    delay(500);
    M5.Lcd.printf(".");
    M5.update();
    if (M5.BtnA.isPressed()){
      OTA_flag = true;
      M5.Lcd.setCursor(0,30);
      M5.Lcd.printf("OTA mode");
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
    }
  }

}

void loop() {
  if(OTA_flag == true){
    ArduinoOTA.handle();
    M5.Lcd.setCursor(0,150);
    M5.Lcd.println("Stop OTA, press C");
    M5.update();
    if (M5.BtnC.isPressed()) {  // if BtnA is Pressed.  如果按键A按下
      ArduinoOTA.end();       // Ends the ArduinoOTA service.  结束OTA服务
      M5.Lcd.println("OTA End!");
      OTA_flag = false;
      delay(3000);
      M5.Lcd.clear();
    }
    delay(50);
  }else{
    delay(20);
  }
}