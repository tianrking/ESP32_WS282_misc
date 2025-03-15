#include <Arduino.h>
#include <FastLED.h>  // FASTLED 3.9.13

#define DATA_PIN D1   // WS2812 数据引脚
#define NUM_LEDS 300  // LED 数量
#define BRIGHTNESS 10 // 降低亮度设置（0-255）
#define SPEED 20      // 动画速度控制，数值越大速度越慢

CRGB leds[NUM_LEDS];  // 定义 FastLED 的 LED 数组

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); // 初始化 FastLED
  FastLED.setBrightness(BRIGHTNESS); // 设置较低的亮度
}

void loop() {
  // 彩虹流水灯效果
  static uint8_t hue = 0;  // 用于颜色循环的变量
  
  // 更新每个LED的颜色
  for (int i = 0; i < NUM_LEDS; i++) {
    // 创建彩虹效果，使每个LED之间有颜色差异
    // 255/NUM_LEDS 确保整个灯带能显示完整的颜色光谱
    leds[i] = CHSV(hue + (i * 255 / NUM_LEDS), 255, 255);
  }
  
  FastLED.show();  // 显示当前帧
  
  // 缓慢改变颜色基准点，使色彩看起来像在流动
  EVERY_N_MILLISECONDS(SPEED) {
    hue++;  // 每SPEED毫秒增加色调值，产生缓慢变化的效果
  }
  
  // 可选：如果不想使用EVERY_N_MILLISECONDS，可以用下面这行代替上面的代码
  // hue++; delay(SPEED);
}
