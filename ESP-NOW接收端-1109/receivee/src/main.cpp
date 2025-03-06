#include <Arduino.h>
#include <esp_now.h>
#include "WiFi.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <U8g2lib.h>

// OLED
#define SCL 22
#define SDA 21
U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // U8G2_R0正常，U8G2_R2翻转180

// 创建一个结构体接收数据
typedef struct struct_message {
  unsigned char id;
  float level;
  signed char restTime[3];
  int intervalTime;
} struct_message;

// 创建一个结构体变量
struct_message myData;
struct_message board1;
struct_message board2;
struct_message boardsStruct[2] = {board1, board2};


void showLevel(float levelTemp, int HH, int MM);
std::string format(float var, int precision, bool leadingSpace);
void OLED_Init();

// 回调函数,当收到消息时会调佣该函数
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);

  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  Serial.println("");
  // 更新数据
  boardsStruct[myData.id-1].level = myData.level;
  boardsStruct[myData.id-1].intervalTime = myData.intervalTime;
  boardsStruct[myData.id-1].restTime[0] = myData.restTime[0];
  boardsStruct[myData.id-1].restTime[1] = myData.restTime[1];
  boardsStruct[myData.id-1].restTime[2] = myData.restTime[2];

  Serial.printf("level value: %lf \n", boardsStruct[myData.id-1].level);
  Serial.printf("intervalTime value: %d \n", boardsStruct[myData.id-1].intervalTime);
  Serial.printf("restTime value: ");
  Serial.printf("%d 时 %d 分 %d 秒 \n", boardsStruct[myData.id-1].restTime[0], boardsStruct[myData.id-1].restTime[1], boardsStruct[myData.id-1].restTime[2]);
  Serial.println();

}


void setup() {
  Serial.begin(9600);
  OLED_Init();
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());

  // 初始化esp-now
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //注册接收信息的回调函数
  esp_now_register_recv_cb(OnDataRecv);
  
}
 
void loop() {
  Serial.println("running...");
  showLevel(boardsStruct[0].level, boardsStruct[0].restTime[0], boardsStruct[0].restTime[1]);
  
  delay(3000);
}

void showLevel(float levelTemp, int HH, int MM) {
  std::string HHstr, MMstr, levelStr;
  if (levelTemp == 100) {
    HHstr = "-小时";
    MMstr = "-分钟";
  }
  else {
    HHstr = format(HH, 0, true) + "小时";
    MMstr = format(MM, 0, true) + "分钟";
  }
  levelStr = format(levelTemp, 2, false) + " %";

  oled.clearBuffer();
  oled.setFontPosTop();
  oled.drawUTF8(7, 7, "液位");
  oled.drawUTF8(54, 7, "--");
  oled.drawUTF8(72, 7, levelStr.c_str());
  oled.drawUTF8(7, 32, "剩余");
  oled.drawUTF8(7, 50, "时间");
  oled.drawUTF8(54, 40, "--");
  oled.drawUTF8(80, 32, HHstr.c_str());
  oled.drawUTF8(80, 50, MMstr.c_str());
  oled.sendBuffer();
}

/**
 * @brief 格式化浮点数，保留指定小数位数，并在整数部分前添加空格
 * @param var 要格式化的浮点数
 * @param precision 保留的小数位数
 * @param leadingSpace 是否在整数部分前添加空格(整数部分保留两位)
 */
std::string format(float var, int precision, bool leadingSpace) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision);
    if (leadingSpace) {
        oss << std::setw(precision + 2) << std::setfill(' ');
    }
    oss << var;
    return oss.str();
}

/**
 * @brief 初始化OLED显示屏
 */
void OLED_Init() {
  oled.setI2CAddress(0x3C*2);
  oled.setFont(u8g2_font_wqy16_t_gb2312a);
  oled.begin();
  oled.enableUTF8Print();
  oled.firstPage();
  Serial.println("OLED初始化完成");
}