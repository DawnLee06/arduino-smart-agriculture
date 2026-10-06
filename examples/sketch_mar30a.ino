#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ========== 配置参数（请根据实际情况修改） ==========
const char* WIFI_SSID = "Dawn";       // 替换为您的Wi-Fi名称
const char* WIFI_PASSWORD = "12345678"; // 替换为您的Wi-Fi密码
const char* SERVER_URL = "http://192.168.43.188:8080/api/parking/update"; // 替换为您的API地址

// 车位状态常量（与主控单片机通信协议）
#define STATUS_AVAILABLE 0  // 空车位
#define STATUS_OCCUPIED  1  // 已占用

// ========== 全局变量 ==========
WiFiClient client;
HTTPClient http;
StaticJsonDocument<200> jsonDoc; // 固定大小JSON缓冲区（避免内存碎片）

// 车位数据（由主控单片机通过串口发送）
int totalSpots = 50;      // 总车位数（示例值，实际由主控设置）
int availableSpots = 20;  // 剩余车位数（示例值）
char spotID[5] = "A3";    // 车位ID（示例值）

// ========== 程序初始化 ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Smart Parking Network Module ===");
  Serial.println("Initializing ESP8266...");

  // 连接Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 从主控单片机初始化车位数据（实际项目需通过串口接收）
  // 此处为示例，实际应替换为串口读取逻辑
  initParkingData();
}

// ========== 主循环 ==========
void loop() {
  // 每10秒上传一次车位数据
  static unsigned long lastUpload = 0;
  if (millis() - lastUpload > 10000) {
    lastUpload = millis();
    uploadParkingData();
  }

  // 从主控单片机接收更新（通过串口，示例）
  if (Serial.available()) {
    handleSerialCommand();
  }

  delay(100); // 降低CPU占用
}

// ========== 上传车位数据 ==========
void uploadParkingData() {
  Serial.println("\n[UPLOAD] Uploading parking data...");
  
  // 构造JSON数据
  jsonDoc.clear();
  jsonDoc["total_spots"] = totalSpots;
  jsonDoc["available_spots"] = availableSpots;
  jsonDoc["spot_id"] = spotID;
  jsonDoc["status"] = (availableSpots > 0) ? STATUS_AVAILABLE : STATUS_OCCUPIED;

  String jsonData;
  serializeJson(jsonDoc, jsonData);
  Serial.println("JSON Data: " + jsonData);

  // 发送HTTP POST请求
  if (http.begin(client, SERVER_URL)) {
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(jsonData);
    
    if (httpCode > 0) {
      String response = http.getString();
      Serial.printf("[HTTP %d] Response: %s\n", httpCode, response.c_str());
      
      // 处理服务器响应（例如更新本地数据）
      if (httpCode == 200) {
        Serial.println("[SUCCESS] Data uploaded successfully!");
      }
    } else {
      Serial.printf("[ERROR] HTTP Error: %d\n", httpCode);
      handleNetworkError();
    }
    
    http.end();
  } else {
    Serial.println("[ERROR] Connection failed");
    handleNetworkError();
  }
}

// ========== 处理网络错误 ==========
void handleNetworkError() {
  Serial.println("[ERROR] Network error! Attempting to reconnect...");
  
  // 尝试重连Wi-Fi
  WiFi.reconnect();
  delay(2000);
  
  // 如果重连失败，等待5秒再试
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[RETRY] Waiting 5 seconds before next attempt...");
    delay(5000);
  }
}

// ========== 从主控接收车位更新（串口通信） ==========
void handleSerialCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  
  Serial.print("[SERIAL] Received: ");
  Serial.println(command);
  
  // 示例：主控发送 "UPDATE:50,15,A3" 格式
  if (command.startsWith("UPDATE:")) {
    // 解析数据：UPDATE:总车位,剩余车位,车位ID
    int pos1 = command.indexOf(',');
    int pos2 = command.indexOf(',', pos1 + 1);
    
    if (pos1 > 0 && pos2 > pos1) {
      totalSpots = command.substring(7, pos1).toInt();
      availableSpots = command.substring(pos1 + 1, pos2).toInt();
      strncpy(spotID, command.substring(pos2 + 1).c_str(), 4);
      spotID[4] = '\0'; // 确保字符串结束
      
      Serial.printf("[UPDATE] Updated: %d/%d [%s]\n", totalSpots, availableSpots, spotID);
    }
  }
}

// ========== 初始化车位数据（示例） ==========
void initParkingData() {
  // 实际项目中应通过串口从主控获取
  Serial.println("[INIT] Using example parking data:");
  Serial.printf("Total: %d, Available: %d, Spot: %s\n", totalSpots, availableSpots, spotID);
}