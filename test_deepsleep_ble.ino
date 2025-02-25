#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_bt.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool bleInitialized = false;

RTC_DATA_ATTR int bootCount = 0;
#define INACTIVITY_TIMEOUT 30000  // 30秒无命令进入休眠（单位：毫秒）

touch_pad_t touchPin;
unsigned long lastCommandTime = 0;

// 调色控制函数
void adjust_glass_color(String command) {
  if (command == "darken") {
    Serial.println("调整为深色");
  } else if (command == "lighten") {
    Serial.println("调整为浅色");
  }
}

// BLE 回调
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("设备已连接");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("设备已断开");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      String command = String(value.c_str());
      Serial.print("收到命令: "); Serial.println(command);
      lastCommandTime = millis();

      // 新增: 检查是否收到sleep命令
      if (command == "sleep") {
        Serial.println("收到sleep命令，立即进入休眠");
        pCharacteristic->setValue("Going to sleep");
        pCharacteristic->notify();
        delay(100); // 确保通知发送完成
        go_to_sleep();
        return; // 在休眠后return，避免执行后续代码
      }

      // 原有的调色逻辑
      adjust_glass_color(command);
      String response = "Done";
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
      Serial.print("发送响应: "); Serial.println(response);
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  bootCount++;
  Serial.println("启动次数: " + String(bootCount));

  touchSleepWakeUpEnable(T2, 40);

  deviceConnected = false;
  oldDeviceConnected = false;
  pServer = NULL;
  pCharacteristic = NULL;
  bleInitialized = false;

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD) {
    Serial.println("由D2触摸唤醒，开始BLE...");
    touchPin = esp_sleep_get_touchpad_wakeup_status();
    Serial.print("触摸唤醒通道: T"); Serial.println(touchPin);
  } else {
    Serial.println("首次启动或其他唤醒，开始BLE...");
  }

  init_ble();
  lastCommandTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  // 维持原有的30秒超时休眠机制
  if (bleInitialized && (currentTime - lastCommandTime >= INACTIVITY_TIMEOUT)) {
    Serial.println("30秒无命令，进入深度睡眠...");
    go_to_sleep();
  }

  if (deviceConnected) {
    delay(10);
  } else if (oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("重新开始广播");
    oldDeviceConnected = false;
  }
}

void init_ble() {
  Serial.println("初始化BLE...");
  BLEDevice::init("SmartGlasses");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("BLE服务创建");

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  Serial.println("BLE特征值创建");

  pService->start();
  Serial.println("BLE服务启动");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinInterval(0x100);
  pAdvertising->setMaxInterval(0x100);
  BLEDevice::startAdvertising();
  Serial.println("BLE广播启动，等待连接...");
  oldDeviceConnected = false;
  bleInitialized = true;
}

void go_to_sleep() {
  Serial.println("停止BLE...");
  if (bleInitialized) {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    if (pAdvertising) {
      pAdvertising->stop();
      Serial.println("广播已停止");
    }

    if (deviceConnected) {
      Serial.println("等待连接断开...");
      unsigned long startTime = millis();
      while (deviceConnected && (millis() - startTime < 5000)) {
        delay(10);
      }
      if (!deviceConnected) {
        Serial.println("连接已断开");
      } else {
        Serial.println("连接未完全断开，继续关闭BLE");
      }
    }

    esp_bt_controller_disable();
    Serial.println("BT控制器已禁用");
    esp_bt_controller_deinit();
    Serial.println("BT控制器已释放");
  } else {
    Serial.println("BLE未初始化，跳过关闭");
  }

  Serial.flush();
  touchSleepWakeUpEnable(T2, 40);
  Serial.println("触摸唤醒已重新启用");

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

  esp_sleep_enable_touchpad_wakeup();
  Serial.println("进入深度睡眠...");
  Serial.flush();
  delay(100);
  Serial.end();
  esp_deep_sleep_start();
}
