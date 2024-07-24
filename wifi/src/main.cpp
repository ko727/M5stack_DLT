#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <WiFi.h>

// WiFi credentials.
// Set password to "" for open networks.
char ssid1[] = "elecom-ab5003";
char pass1[] = "5njiuhcuth7c";
char ssid2[] = "MU0104";
char pass2[] = "17320508";
char ssid3[] = "Buffalo-G-0CBA";
char pass3[] = "hh4aexcxesasx";


bool second_flag = false;
bool third_flag = false;
bool wifi_flag = true;
unsigned char wifi_cnt = 0;

void setup() {
    M5.begin();  // Init M5Core. 初始化 M5Core
    M5.Power.begin();
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid1, pass1);  // Connect wifi and return connection status.
                                 // 连接wifi并返回连接状态
    M5.lcd.print("Waiting Wifi Connect");
    while (WiFi.status() != WL_CONNECTED) {  // If the wifi connection fails.  若wifi未连接成功
        delay(1000);
        M5.lcd.print(".");
    }
    M5.lcd.println("\nWiFi Connected!");
    M5.lcd.print("WiFi Connect To: ");
    M5.lcd.println(WiFi.SSID());  // Output Network name.  输出网络名称
    M5.lcd.print("IP address: ");
    M5.lcd.println(WiFi.localIP());  // Output IP Address.  输出IP地址

    ArduinoOTA
    .setHostname("M5Core")
    .onStart([]() {})
    .onEnd([]() {})
    .onProgress([](unsigned int progress, unsigned int total) {})
    .onError([](ota_error_t error) {});   // Set the network port name.  设置网络端口名称
    ArduinoOTA.begin();            // Initialize the OTA.  初始化OTA
    M5.lcd.println("OTA ready!");  // M5.lcd port output format string.
                                   // 串口输出格式化字符串
}

void loop() {
    ArduinoOTA.handle();  // Continuously check for update requests.
                          // 持续检测是否有更新请求
    M5.update();
    if (M5.BtnA.isPressed()) {  // if BtnA is Pressed.  如果按键A按下
        ArduinoOTA.end();       // Ends the ArduinoOTA service.  结束OTA服务
        M5.lcd.println("OTA End!");
        delay(200);
    }
    delay(20);
}