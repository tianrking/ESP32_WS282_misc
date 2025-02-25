#include <Arduino.h>
#include <FastLED.h>
//FASTLED 3.9.13

#define DATA_PIN D1  // WS2812 数据引脚
#define NUM_LEDS 25  // LED 数量 (5x5 = 25)

CRGB leds[NUM_LEDS];  // 定义 FastLED 的 LED 数组

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); // 初始化 FastLED
  FastLED.setBrightness(50); // 设置亮度 (0-255), 建议先设置较低的值
}

void loop() {
  // 示例 1: 全部点亮为红色
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red; // 设置所有 LED 为红色
  }
  FastLED.show(); // 显示红色
  delay(1000);    // 保持 1 秒

  // 示例 2: 全部点亮为绿色
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(1000);

  // 示例 3: 全部点亮为蓝色
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  FastLED.show();
  delay(1000);

  // 示例 4: 全部关闭
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; // 或者 CRGB(0, 0, 0)
  }
  FastLED.show();
  delay(1000);

  // 示例 5：彩虹效果 (需要循环)
   for (int j = 0; j < 255; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((i * 255 / NUM_LEDS) + j, 255, 255);  // 使用 HSV 颜色空间
    }
    FastLED.show();
    delay(10); // 稍微延时，让动画更流畅
  }
}
