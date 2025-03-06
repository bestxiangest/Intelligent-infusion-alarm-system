#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFiUdp.h>    // NTP时间
#include <NTPClient.h>  // NTP时间
#include "qrcode.h"     // 二维码
#include <esp_now.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <HTTPClient.h>
#include "I2Cdev.h"
// #include "MPU6050.h"
#include <MPU6050_tockn.h> // MPU6050库
#include <bitmap_data.h>

// oled显示屏
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128  // MN150-128128-4 是 128x128 的分辨率
#define OLED_RESET -1      // 这通常用于硬件复位引脚
#define OLEDSDA 0
#define OLEDSCL 1
#define MPUSDA 18
#define MPUSCL 19
#define pianyi 30

//二维码模块
// 准备要转换为二维码的 URL
const char* url = "https://mp.weixin.qq.com/a/~~lqdfz0CK6UY~2Yequemy5iK_Daenhet9BA~~";
// 二维码每个模块的像素大小
uint8_t pixelSize = 4; 


WiFiClient espClient;                   // WiFi模板
PubSubClient client(espClient);         // WiFi模板
WiFiUDP ntpUDP;                         // NTP时间
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*8, 60000); // 8是时区

//mpu6050防抖模块
unsigned long timer = 0,lastTimer = 0;
const float shakeThreshold = 1.2; // 震动阈值

// 常量定义
const int nodeNum = 8;         // 节点数量
const int baudRate = 115200;   // 波特率

// 74LS151控制引脚
const int pin_A = 5; // A输入
const int pin_B = 6; // B输入
const int pin_C = 7; // C输入
const int pin_Y = 4; // Y输出

const int beepIO = 10;          // 蜂鸣器IO口
const int keyIO = 2;          // 中断按键IO口
const int ledIO = 3;          // ledIO口

short firstConnectWiFi = 1;//第一次连接WiFi

struct WiFi_Info {
  const char* ssid;
  const char* password;
};
const WiFi_Info savedWiFi[] = {
  {"sharp_caterpillar", "zzn20041031"},
  {"HNX望远的vivo X200 Pro", "98765321"},
};
int cntSavedWiFi = sizeof(savedWiFi) / sizeof(savedWiFi[0]); // 计算网络数量

bool node_last[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};             // 节点[i]上一次的状态
bool node_cur[10];              // 节点[i]当前状态
short sum = 0;                   // 检测到有水的节点的总数
float level = 100;               // 输液水位
long long nowTime = 0, lastTime = 0, intervalTime = 0; // 当前时间，上一次时间，时间差
int startTime[5];              // 开始输液时间。startTime[1]时，startTime[2]分，startTime[3]秒
int restTime[5] = {-1, -1, -1, -1, -1};               // 剩余输液时间。restTime[1]时，restTime[2]分，restTime[3]秒
char propertiesNames[15][32];
volatile bool isShowQRcode = false;

// 配置华为云MQTT
const char* mqttServer = "9767762256.st1.iotda-device.cn-east-3.myhuaweicloud.com";
const int mqttPort = 1883;
const char* ClientId ="660fbe30387fa41cc8a08b04_esp32c3-002_0_0_2024041608";         
const char* mqttUser ="660fbe30387fa41cc8a08b04_esp32c3-002";
const char* mqttPassword = "b770d807a23e57f7127ec5c96406de5cb2464a8ec3c7f371e018128728332f42";
#define device_id "660fbe30387fa41cc8a08b04_esp32c3-002" 
#define secret "ddws2024" 

// JSON
#define publishJsonHead "{\"services\":[{\"service_id\":\"levelDetector\",\"properties\":{" 
#define publishJsonFoot "}}]}"
#define Iot_link_MQTT_Topic_Report "$oc/devices/" device_id "/sys/properties/report"

// ESPNOW结构体
typedef struct struct_message {
  unsigned char id;
  float level;
  signed char restTime[3];
  int intervalTime;
} struct_message;
struct_message* myData = new struct_message;

// 函数声明
std::string format(float var, int precision, bool leadingSpace);
void IRAM_ATTR interrupt();
void OLED_Init();
void WiFi_Scan();
String rssiRate(int rssi);
void showWiFi(String ssid, int rssi, int pauseTime);
bool WiFi_Init();
bool MQTT_Init();
void publishPropertiesJson(char *properties);
void NTP_Init();
// >>>>>>
bool readNode(int node);
// <<<<<<
bool isLevelChanged(int refreshInterval);
void showLevel(float levelTemp, int HH, int MM);
void showQRcode(int pauseTime);
void showQR_code(int pauseTime);//新的
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void UsingEspNowSendMessage(struct_message *);
void mpuInit();
void mpu_read_and_process();
void QR_Code();
// 98:3D:AE:EB:41:C0
uint8_t broadcastAddress[] = {0x98, 0x3D, 0xAE, 0xEB, 0x41, 0xC0}; // ESPNOW发送端MAC地址接收端MAC地址

// 创建 U8G2 对象，使用 SH1107 128x128 的 I2C 模式
U8G2_SH1107_128X128_F_SW_I2C oled(U8G2_R3, /* clock=*/ 1, /* data=*/ 0, /* reset=*/ U8X8_PIN_NONE);

//MPU6050实例化
MPU6050 mpu6050(Wire);


void setup() {
  Serial.begin(115200);       // 波特率
  // Wire.begin(OLEDSDA,OLEDSCL);
  WiFi.mode(WIFI_STA);
  // WiFi_Scan();

  // OLED
  OLED_Init();
  // 初始化74LS151控制引脚
  pinMode(pin_A, OUTPUT);
  pinMode(pin_B, OUTPUT);
  pinMode(pin_C, OUTPUT);
  pinMode(pin_Y, INPUT);
  // <<<<<<

  pinMode(beepIO, OUTPUT);
  pinMode(ledIO, OUTPUT);

  digitalWrite(ledIO,LOW);
  digitalWrite(beepIO,LOW);    // 蜂鸣器不响

  // MQTT初始化
  WiFi_Init();
  MQTT_Init();
  NTP_Init();
  char properties[128];
  sprintf(properties, "\"status\":\"正常\",\"tips\":\"在线\"");
  publishPropertiesJson(properties);

  // 中断KEY配置
  pinMode(keyIO, INPUT_PULLUP);   // 输入模式，启用内部上拉电阻
  attachInterrupt(digitalPinToInterrupt(keyIO), interrupt, FALLING); // 将KEY配置为外部中断，使用下降沿触发模式

  showLevel(level,restTime[1],restTime[2]);
  //初始化MPU6050
  mpuInit();
  lastTimer = millis();
}
short isLevelChanging = 0;
void loop() {
  isLevelChanging = isLevelChanged(1000);
  if (isShowQRcode) {
    showQR_code(3000);
    // showLevel(level,restTime[1],restTime[2]);
  }
  // 计时器自增
  timer = millis();

  showLevel(level,restTime[1],restTime[2]);

  // 如果当前状态和上一次状态不一样，则说明水位发生变化
  if (isLevelChanging && (timer - lastTimer > 2000)) {
    lastTimer = timer;    //MPU6050消抖模块计时器
    lastTime = nowTime;   // 当前时间作为上一次时间
    nowTime = millis();   // 读入系统时间作为当前时间
    intervalTime = nowTime - lastTime;  // 计算本次水位变化和上一次水位变化的时间差(毫秒)

    char properties[256];

    //Serial.println("=\\/======================================================================\\/=");
    sum = 0;
    for (int j=1;j<=nodeNum;j++) {
      sum += node_cur[j];
      sprintf(propertiesNames[j], "node%d", j);
    }
    sprintf(properties, "\"nodeStatus\":[%d,%d,%d,%d,%d,%d,%d,%d]", node_cur[1], node_cur[2], node_cur[3], node_cur[4], node_cur[5], node_cur[6], node_cur[7], node_cur[8]);
    // properties = "nodeStatus":[?,?,?,?,?,?,?,?]
    level = (float)sum/nodeNum*100;  // 液位: level%
    intervalTime *= sum;      // 水位变化时间差(毫秒) -> 预测剩余总时间(毫秒)
    restTime[1] = intervalTime / 1000 / 60 / 60;  // 时
    restTime[2] = intervalTime / 1000 / 60 % 60;  // 分
    restTime[3] = intervalTime / 1000 % 60;            // 秒

    if (level == 0) {
      Serial.println("报警");
      digitalWrite(ledIO,HIGH);
      while (!isLevelChanged(500)) {
        digitalWrite(beepIO,HIGH);
        delay(100);
        digitalWrite(beepIO,LOW);
        delay(100);
      }
      digitalWrite(ledIO,HIGH);
      digitalWrite(beepIO,LOW);
    }
    else digitalWrite(ledIO,LOW);

    sprintf(properties, "%s,\"level\":%.1lf,\"restTime\":[%d,%d,%d]", properties, level, restTime[1], restTime[2], restTime[3]);
    showLevel(level,restTime[1],restTime[2]);
    
    // ESPNOW
    myData->level = level;
    myData->restTime[0] = restTime[1];
    myData->restTime[1] = restTime[2];
    myData->restTime[2] = restTime[3];
    myData->intervalTime = intervalTime;
    UsingEspNowSendMessage(myData);

    digitalWrite(beepIO,LOW);
    if (intervalTime == 0) {
      sprintf(properties, "%s,\"status\":\"输液完成\"", properties);
      publishPropertiesJson(properties);
    }
    else if (intervalTime > 0 && intervalTime < 2*60*1000) {   // 剩余不足2分钟
      sprintf(properties, "%s,\"status\":\"还有%d分钟\"", properties, restTime[2]);
      publishPropertiesJson(properties);
    }
    else {
      sprintf(properties, "%s,\"status\":\"正常\"", properties);
      publishPropertiesJson(properties);}
    //Serial.println("=/\\======================================================================/\\=");    
  }
  else if(isLevelChanging && (timer - lastTimer) <= 2000){
    // timer = millis(); 
    // 记录当前时间,重置intervalTime
    lastTimer = timer;
  }
  // Wire.begin(MPUSDA,MPUSCL);
  // // 读取 MPU6050 数据并处理
  mpu_read_and_process(); 
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
  // Wire.begin(OLEDSDA, OLEDSCL);
  // oled.setI2CAddress(0x3C*2);
  oled.begin();
  oled.enableUTF8Print();
  oled.firstPage();
  Serial.println("OLED初始化完成");
}

/**
 * @brief 判断WiFi信号强度
 */
String rssiRate(int rssi) {
  if (rssi > -50)
    return "强";
  else if (rssi <= -50 && rssi > -70)
    return "中";
  else
    return "弱";
}

/**
 * @brief 扫描WiFi网络
 */
void WiFi_Scan() {
  Serial.println("WiFi扫描中...");
  int n = WiFi.scanNetworks();
  Serial.println("扫描到的网络数量: " + String(n));
  for (int i = 0; i < n; ++i) {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID(i));
    // int rssi = WiFi.RSSI(i);
    // Serial.print("信号强度 (RSSI): ");
    // Serial.println(rssi);
    // bool enc = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    // Serial.print("是否加密: ");
    // Serial.println(enc ? "是" : "否");
    // Serial.println();
  }
}

/**
 * @brief OLED显示WiFi信息
 */
void showWiFi(String ssid, int rssi, int pauseTime) {
  oled.firstPage();
  do
  {
    oled.setFontPosTop();
    oled.setFont(u8g2_font_wqy16_t_gb2312a);
    oled.setCursor(12,7);
    if (ssid == "") {
      oled.print("未连接WiFi");
    }
    else {
      oled.print("已连接: ");
      oled.setCursor(12,25);
      if (ssid.length() >= 14) {
        // 如果SSID太长，使用小字体并换行显示
        oled.setFont(u8g2_font_timB10_tf);
        oled.print(ssid);
        oled.setFont(u8g2_font_timB14_tf);
      } else {
        oled.setFont(u8g2_font_timB14_tf);
        oled.print(ssid);
      }
      oled.setCursor(12,50);
      oled.setFont(u8g2_font_wqy16_t_gb2312a);
      oled.print(String("信号强度: " /*+ String(rssi)*/  + rssiRate(rssi)));
    }
  }while(oled.nextPage());
  delay(pauseTime);
}
/**
 * @brief 初始化WiFi连接
 */
bool WiFi_Init() {
    for (int i = 0; i < cntSavedWiFi; ++i) {
      const char* ssid = savedWiFi[i].ssid;
      const char* password = savedWiFi[i].password;
      WiFi.begin(ssid, password);
      Serial.print("[WiFi]\t尝试连接到\"");Serial.print(ssid);Serial.print("\"...");
      if(firstConnectWiFi == 1){  
        oled.firstPage();
        do
        {
          oled.setFontPosTop();
          oled.setFont(u8g2_font_wqy16_t_gb2312a);
          oled.setCursor(12,7+pianyi);
          oled.print("尝试连接到: ");
          oled.setCursor(12,25+pianyi);
          if (strlen(ssid) >= 14) {
            // 如果SSID太长，使用小字体并换行显示
            oled.setFont(u8g2_font_timB10_tf);
            oled.print(ssid);
            oled.setFont(u8g2_font_timB14_tf);
          } else {
            oled.setFont(u8g2_font_timB14_tf);
            oled.print(ssid);
          }
        }while(oled.nextPage());
      }
      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (millis() - startTime > 3000) {
          Serial.print("\r\n[WiFi]\t\"");Serial.print(ssid);Serial.println("\"连接超时, 尝试下一个网络...");
          break;
        }
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\r\n[WiFi]\t连接成功");
        Serial.print("[WiFi]\t信号强度: ");Serial.print(WiFi.RSSI());
        Serial.println(" (" + rssiRate(WiFi.RSSI()) + ")");
        Serial.print("本设备IP地址: ");Serial.println(WiFi.localIP());
        
        if(firstConnectWiFi == 1) {
          showWiFi(WiFi.SSID(), WiFi.RSSI(), 500);
          firstConnectWiFi = 0;
        }
        return true;
      }
    }
    Serial.println("\r\n[WiFi]\t连接失败");
    
    return false;

}

/**
 * @brief 初始化MQTT连接
 */
bool MQTT_Init() {
  int state = 0;
  Serial.print("[MQTT]\t连接中...");
  client.setServer(mqttServer, mqttPort);
  client.setKeepAlive(60);
  client.connect(ClientId, mqttUser, mqttPassword);
  unsigned long startTime = millis();
  while (!client.connected()) 
  {
    delay(500);
    Serial.print(".");
    if (client.connect(ClientId, mqttUser, mqttPassword)) 
    {
      Serial.println("\r\n[MQTT]\t连接成功");
      return true;
    } 
    else 
    {
      if (state != client.state()) {
        Serial.print("连接失败, 错误代码: ");
        Serial.print(client.state());
        state = client.state();
      }
    }
    if (millis() - startTime > 10000) {
      Serial.println("\r\n[MQTT]\t连接超时");
      return false;
    }
  } 
  Serial.println("\r\n[MQTT]\t连接成功");
  return true;
}

/**
 * @brief MQTT上报properties
 * @param properties 属性JSON字符串
 */
void publishPropertiesJson(char *properties) {
    const char *publishJsonHeadString = "{\"services\":[{\"service_id\":\"levelDetector\",\"properties\":{";
    const char *publishJsonFootString = "}}]}";
    char json[256];    // JSON
    memset(json, 0, sizeof(json));  // 清空json以确保没有旧数据
    sprintf(json, "%s%s%s", publishJsonHeadString, properties, publishJsonFootString);
    client.publish(Iot_link_MQTT_Topic_Report, json);
    //Serial.println("++++++++++\\/++++++++++ publishPropertiesJson ++++++++++\\/++++++++++");
    //Serial.print("  Iot_link_MQTT_Topic_Report: "); Serial.println(Iot_link_MQTT_Topic_Report);
    Serial.print("[MQTT]\tJSON: "); Serial.println(json);
    //Serial.println("MQTT Publish OK!");
    //Serial.println("++++++++++/\\++++++++++ publishPropertiesJson ++++++++++/\\++++++++++");
}

/**
 * @brief 初始化NTP时间同步
 */
void NTP_Init() {
  char properties[128];
  timeClient.begin();
  timeClient.update();
  startTime[1] = timeClient.getFormattedTime().substring(0,2).toInt();
  startTime[2] = timeClient.getFormattedTime().substring(3,5).toInt();
  startTime[3] = timeClient.getFormattedTime().substring(6,8).toInt();
  // Serial.println("startTime: " + String(timeClient.getFormattedTime()));
  sprintf(properties, "\"startTime\":[%d,%d,%d]", startTime[1], startTime[2], startTime[3]);
  publishPropertiesJson(properties);
}

/**
 * @brief 读取指定节点的状态
 * @param node 节点编号
 */
bool readNode(int node) {
  // 确保节点编号在1到nodeNum之间
  if (node < 1 || node > nodeNum) {
    Serial.println("节点编号超出范围");
    return false;
  }

  // 将节点编号转换为3位二进制数
  // int abc[3] = {
  //   (node - 1) & 0x01, // C
  //   (node - 1) & 0x02, // B
  //   (node - 1) & 0x04  // A
  // };
  
  // 设置GPIO5~7为输出，选择节点
  // digitalWrite(pin_A, abc[0]);
  // digitalWrite(pin_B, abc[1]);
  // digitalWrite(pin_C, abc[2]);

  switch (node) {
    case 1:
      digitalWrite(pin_A, 0); // A = 0
      digitalWrite(pin_B, 0); // B = 0
      digitalWrite(pin_C, 0); // C = 0
      break;
    case 2:
      digitalWrite(pin_A, 1); // A = 1
      digitalWrite(pin_B, 0); // B = 0
      digitalWrite(pin_C, 0); // C = 0
      break;
    case 3:
      digitalWrite(pin_A, 0); // A = 0
      digitalWrite(pin_B, 1); // B = 1
      digitalWrite(pin_C, 0); // C = 0
      break;
    case 4:
      digitalWrite(pin_A, 1); // A = 1
      digitalWrite(pin_B, 1); // B = 1
      digitalWrite(pin_C, 0); // C = 0
      break;
    case 5:
      digitalWrite(pin_A, 0); // A = 0
      digitalWrite(pin_B, 0); // B = 0
      digitalWrite(pin_C, 1); // C = 1
      break;
    case 6:
      digitalWrite(pin_A, 1); // A = 1
      digitalWrite(pin_B, 0); // B = 0
      digitalWrite(pin_C, 1); // C = 1
      break;
    case 7:
      digitalWrite(pin_A, 0); // A = 0
      digitalWrite(pin_B, 1); // B = 1
      digitalWrite(pin_C, 1); // C = 1
      break;
    case 8:
      digitalWrite(pin_A, 1); // A = 1
      digitalWrite(pin_B, 1); // B = 1
      digitalWrite(pin_C, 1); // C = 1
      break;
    default:
      Serial.println("无效的节点编号");
      return false;
  }
  
  // 读取Y端的状态，即当前节点的状态
  return digitalRead(pin_Y);
}

/**
 * @brief 判断液位是否变化
 * @param refreshInterval 刷新时间间隔
 */
bool isLevelChanged(int refreshInterval) {
  static uint32_t lastCheckTime = 0;
  if (millis() - lastCheckTime < refreshInterval) {
    return false; // 时间未到，直接返回
  }
  lastCheckTime = millis();

  // 当前(上一次循环读入的)各节点数据，作为上一次状态
  for (int i=1;i<=nodeNum;i++) {
    node_last[i] = node_cur[i];
  }
  // delay(refreshInterval);   // 刷新时间间隔
  // 再读入各节点数据，作为当前状态
  int flag = 0;
  for (int i=1;i<=nodeNum;i++) {
    node_cur[i] = readNode(i);
    if (node_last[i] != node_cur[i])
      flag = 1;
  }
  return flag;
}
/**
 * @brief OLED显示液位
 * @param levelTemp 剩余液位
 * @param HH 剩余小时
 * @param MM 剩余分钟
 */
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

  oled.drawUTF8(7, 7 + pianyi, "液位");
  oled.drawUTF8(54, 7 + pianyi, "--");
  oled.drawUTF8(72, 7 + pianyi, levelStr.c_str());
  oled.drawUTF8(7, 32 + pianyi, "剩余");
  oled.drawUTF8(7, 50 + pianyi, "时间");
  oled.drawUTF8(54, 40 + pianyi, "--");
  oled.drawUTF8(80, 32 + pianyi, HHstr.c_str());
  oled.drawUTF8(80, 50 + pianyi, MMstr.c_str());
  oled.sendBuffer();
}

/**
 * @brief 按键中断处理函数
 */
void IRAM_ATTR interrupt() {
  Serial.println("按键被按下");
  isShowQRcode = true;
}

/**
 * @brief OLED显示二维码
 * @param pauseTime 显示二维码的持续时间
 */
void showQRcode(int pauseTime) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData , 3, ECC_MEDIUM, "");
  // 绘制二维码
  oled.firstPage();
  do {
    // get the draw starting point,128 and 64 is screen size
    uint8_t x0 = (128 - qrcode.size * 2) / 2;
    uint8_t y0 = (64 - qrcode.size * 2) / 2;
    // get QR code pixels in a loop
    for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
        // Check this point is black or white
        if (qrcode_getModule(&qrcode, x, y)) {
          oled.setColorIndex(1);
        } else {
          oled.setColorIndex(0);
        }
        // Double the QR code pixels
        oled.drawPixel(x0 + x * 2, y0 + y * 2);
        oled.drawPixel(x0 + 1 + x * 2, y0 + y * 2);
        oled.drawPixel(x0 + x * 2, y0 + 1 + y * 2);
        oled.drawPixel(x0 + 1 + x * 2, y0 + 1 + y * 2);
      }
    }
  } 
  while (oled.nextPage());  // 结束绘画
  delay(pauseTime);
  isShowQRcode = false;
}

/**
 * @brief 回调函数，函数在发送消息时执行
 * @param mac_addr 要发送消息的MAC地址
 * @param status 发送状态
 */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW发送");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "成功" : "失败");
}

/**
 * @brief 使用ESP-NOW发送消息
 * @param myData 要发送的数据结构
 */
void UsingEspNowSendMessage(struct_message *myData) {
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  Serial.println("已断开WiFi连接");
  //初始化ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESPNOW]\t初始化错误");
    return;
  }
  // 注册通信频道
  esp_now_peer_info_t peerInfo;
  //注册回调函数
  esp_now_register_send_cb(OnDataSent);
  //设置广播地址
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;  //通道,先设置默认通道传输WiFi信道信息，随后更改信道
  peerInfo.encrypt = false;//是否加密为False
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("[ESPNOW]\t配对失败");
    return;
  }
  myData->id = 1;
  
  //发送消息到指定的MAC地址
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) myData, sizeof(*myData));
  //判断是否发送成功
  if(result == ESP_OK)Serial.println("[ESPNOW]\t发送端已发送");
  else {
    Serial.println("[ESPNOW]\t发送端发送失败,正在第二次发送...");
    //发送消息到指定的MAC地址
    result = esp_now_send(broadcastAddress, (uint8_t *) myData, sizeof(*myData));  
    if(result == ESP_OK)Serial.println("[ESPNOW]\t发送端已发送");else Serial.println("[ESPNOW]\t第二次发送端发送失败");
  }
  WiFi_Init();
  WiFi.mode(WIFI_STA);
  MQTT_Init();
}

//MPU6050初始化
void mpuInit(){
  Wire.begin(MPUSDA, MPUSCL);// SDA, SCL
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);//校准陀螺仪
}


//MPU6050读取数据
void mpu_read_and_process(){
  mpu6050.update();

  // 获取加速度的XYZ值
  float accelX = mpu6050.getAccX();
  float accelY = mpu6050.getAccY();
  float accelZ = mpu6050.getAccZ();

  // 计算加速度的总和
  float totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
  Serial.print("Total Accel: ");
  Serial.println(totalAccel);

  // 检测是否超出阈值
  if (totalAccel > shakeThreshold) {
    Serial.println("Shake detected!");
    // 晃动事件触发发出警报
    Serial.println("摇晃报警");
    digitalWrite(ledIO,HIGH);
    while (totalAccel > shakeThreshold) {
      for (int i = 0; i < 3; i++) {
        digitalWrite(beepIO,HIGH);
        delay(100);
        digitalWrite(beepIO,LOW);
        delay(100);
      }

      mpu6050.update();

      // 获取加速度的XYZ值
      accelX = mpu6050.getAccX();
      accelY = mpu6050.getAccY();
      accelZ = mpu6050.getAccZ();

      // 计算加速度的总和
      totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
    }
    digitalWrite(ledIO,LOW);
    digitalWrite(beepIO,LOW);
  }
  else {
    digitalWrite(ledIO,LOW);
  }
}

//画出二维码
void QR_Code() {
  oled.clearBuffer();
// 假设二维码位图的大小是 128x128，可以根据实际数据调整
  int bitmapWidth = 64;
  int bitmapHeight = 64;

  // 计算二维码的居中偏移量
  int offset_x = (oled.getWidth() - bitmapWidth) / 2 + 10;
  int offset_y = (oled.getHeight() - bitmapHeight) / 2 + pianyi - 5;

  // 绘制位图数据
  for (int y = 0; y < bitmapHeight; y++) {
    for (int x = 0; x < bitmapWidth; x++) {
      if (bitmap[y][x] == 1) {  // 如果像素值为 1，则绘制黑色点
        oled.drawPixel(offset_x + x, offset_y + y);
      }
    }
  }

  // 发送缓冲区数据到 OLED 显示屏
  oled.sendBuffer();
}

void showQR_code(int pauseTime) {
  // 绘制二维码
  oled.firstPage();
  do {
    QR_Code();
  } 
  while (oled.nextPage());  // 结束绘画
  delay(pauseTime);
  isShowQRcode = false;
}