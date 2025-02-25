#include <Arduino.h>

#define TOUCH_PIN T2  // 触摸引脚 T2，对应 GPIO2
#define THRESHOLD 40  // 触摸阈值，可根据硬件调整

RTC_DATA_ATTR int bootCount = 0;  // 保存在 RTC 内存中的启动次数

void setup() {
  Serial.begin(115200);
  delay(1000);  // 等待串口初始化

  // 记录并打印启动次数
  bootCount++;
  Serial.println("启动次数: " + String(bootCount));

  // 检查唤醒原因
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD) {
    Serial.println("由触摸唤醒 (GPIO2)");
    performTask();  // 执行任务
  } else {
    Serial.println("首次启动或其他原因唤醒");
    // 可选：首次启动时的操作
  }

  // 任务完成后进入深度睡眠
  go_to_sleep();
}

void loop() {
  // loop() 不会执行，因为 setup() 中已进入休眠
}

void performTask() {
  // 在这里定义你的任务
  Serial.println("执行任务中...");
  delay(2000);  // 模拟任务执行时间（可替换为实际任务）
  Serial.println("任务完成");
}

void go_to_sleep() {
  // 配置触摸唤醒源
  touchSleepWakeUpEnable(TOUCH_PIN, THRESHOLD);

  // 进入深度睡眠
  Serial.println("进入深度睡眠...");
  Serial.flush();  // 确保串口输出完成
  delay(100);      // 短暂延迟以完成串口操作
  Serial.end();    // 关闭串口
  esp_deep_sleep_start();
}
