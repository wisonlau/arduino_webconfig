#include <FS.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif

ESP8266WebServer webServer(80);

#define ACCESSORY_NAME  ("人体存在传感器")
#define ACCESSORY_SN  ("SN_20220713")
#define ACCESSORY_MANUFACTURER ("bilibili:白纸最开心; 抖音:白纸最开心; github:https://github.com/wisonlau/arduino_webconfig")
#define ACCESSORY_MODEL  ("XBPB004")
#define compile_time "2022-07-16 20:20:20"

String WifiPrefix = "motion-";
String config_path = "/config.json";

uint8_t MAC_array_STA[6];
uint8_t MAC_array_AP[6];
char MAC_CHAR_STA[18];
char MAC_CHAR_AP[18];

// config
char radar_exist[255]; // 雷达有人射频
char radar_not_exist[255]; // 雷达无人射频
char infrared_ray_exist[255]; // 红外有人射频
char infrared_ray_not_exist[255]; // 红外无人射频
int delay_report = 5; // 延时上报
char location[255]; // 位置
String read_config_data = ""; // 读取到的配置数据

void setup() {
  Serial.begin(9600);
  wifiInit();
  webserverInit();
}

void wifiInit() {
  WiFi.mode(WIFI_AP_STA);
  IPAddress softLocal(192, 168, 4, 1);
  IPAddress softGateway(192, 168, 4, 1);
  IPAddress softSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(softLocal, softGateway, softSubnet);
  String chipId = String(ESP.getChipId());
  String apName = WifiPrefix + chipId;
  const char *softAPName = apName.c_str();
  WiFi.softAP(softAPName);
}

void webserverInit() {
  webServer.on("/operate", operate);
  webServer.on("/clearconfig", clearconfig);
  webServer.on("/saveconfig", saveconfig);
  webServer.on("/readconfig", readconfig);
  webServer.on("/test", test);
  webServer.onNotFound(webpage);
  webServer.begin();
}

#include "Web_page.h"
void webpage() {
  webServer.send(200, "text/html", html_page);
}

void operate() {
  ESP.restart();
}

void clearconfig() {
  if (SPIFFS.begin()) {
    SPIFFS.remove(config_path);
    webServer.send(200, "application/json;charset=UTF-8;", "clear config success!");
  } else {
    SPIFFS.remove(config_path);
    webServer.send(200, "application/json;charset=UTF-8;", "clear config fail!");
  }
}

void saveconfig() {
  if (webServer.hasArg("radar_exist")) {
    strcpy(radar_exist, webServer.arg("radar_exist").c_str());
  }
  else
  {
    strcpy(radar_exist, "");
  }

  if (webServer.hasArg("radar_not_exist")) {
    strcpy(radar_not_exist, webServer.arg("radar_not_exist").c_str());
  }
  else
  {
    strcpy(radar_not_exist, "");
  }

  if (webServer.hasArg("infrared_ray_exist")) {
    strcpy(infrared_ray_exist, webServer.arg("infrared_ray_exist").c_str());
  }
  else
  {
    strcpy(infrared_ray_exist, "");
  }

  if (webServer.hasArg("infrared_ray_not_exist")) {
    strcpy(infrared_ray_not_exist, webServer.arg("infrared_ray_not_exist").c_str());
  }
  else
  {
    strcpy(infrared_ray_not_exist, "");
  }

  if (webServer.hasArg("delay_report")) {
    delay_report = webServer.arg("delay_report").toInt();
  }
  else
  {
    delay_report = 5;
  }

  if (webServer.hasArg("location")) {
    strcpy(location, webServer.arg("location").c_str());
  }
  else
  {
    strcpy(location, "");
  }

  DynamicJsonDocument json(1024);
  json["radar_exist"] = radar_exist;
  json["radar_not_exist"] = radar_not_exist;
  json["infrared_ray_exist"] = infrared_ray_exist;
  json["infrared_ray_not_exist"] = infrared_ray_not_exist;
  json["delay_report"] = delay_report;
  json["location"] = location;
  String output;
  serializeJson(json, output);

  if (delay_report)
  {
    boolean save_status = saveConfigFunc(output);
  }

  webServer.send(200, "application/json;charset=UTF-8;", output);
}

void readconfig() {
  readConfigFunc();
  webServer.send(200, "application/json;charset=UTF-8;", read_config_data);
}

void test() {
  webServer.send(200, "application/json;charset=UTF-8;", "ok");
}

void readConfigFunc() {
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Started.");
    bool exist = SPIFFS.exists(config_path);
    if (exist) {
      Serial.println("The file exists!");
      File f = SPIFFS.open(config_path, "r");
      if (!f) {
        Serial.println("Open file fail!");
      } else
      {
        int s = f.size();
        read_config_data = f.readString();
      }
      f.close();
    }
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
}

boolean saveConfigFunc(String str) {
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Started.");
    File dataFile = SPIFFS.open(config_path, "w");
    dataFile.println(str);
    dataFile.close();
    return true;
  } else {
    Serial.println("SPIFFS Failed to Start.");
    return false;
  }
}

void loop() {
  webServer.handleClient();
}
