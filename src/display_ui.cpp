#include "display_ui.h"
#include "board_config.h"
#ifndef MOBILE_DISPLAY_ENABLED
#define MOBILE_DISPLAY_ENABLED 1
#endif
#if MOBILE_DISPLAY_ENABLED
#include <Adafruit_SSD1306.h>
#include <Wire.h>
namespace {
Adafruit_SSD1306 oled(128,64,&Wire,-1);
bool present = false;
}
#endif
bool beginDisplay() {
#if MOBILE_DISPLAY_ENABLED
  Wire.begin(board::kOledSda,board::kOledScl,400000);
  Wire.setTimeOut(20);
  uint8_t address = 0;
  for (uint8_t candidate : {uint8_t(0x3c),uint8_t(0x3d)}) {
    Wire.beginTransmission(candidate);
    if (Wire.endTransmission() == 0) { address = candidate; break; }
  }
  if (!address) return false;
  present = oled.begin(SSD1306_SWITCHCAPVCC,address,true,false);
  if (present) { oled.setTextColor(SSD1306_WHITE); oled.setTextSize(1); }
  return present;
#else
  return false;
#endif
}
void updateDisplay(const ImuSnapshot& s, uint32_t now) {
#if MOBILE_DISPLAY_ENABLED
  if (!present) return;
  oled.clearDisplay(); oled.setCursor(0,0);
  oled.println("MobilePlatform / T3");
#if !MOBILE_IMU_ENABLED
  oled.println("Display check only"); oled.println("IMU disabled by build");
  oled.println("USB serial: help");
#else
  if (!s.connected) {
    oled.println("BNO085 not ready"); oled.println("Check SPI + P0/P1"); oled.println("Serial: retry / help");
  } else if (!s.valid || uint32_t(now-s.orientationMs)>board::kStaleMs) {
    oled.println("IMU data STALE"); oled.println("Serial: status/retry");
  } else {
    const auto e = orientation::eulerDegrees(s.view);
    oled.printf("Roll  %7.1f deg\n",e.roll);
    oled.printf("Pitch %7.1f deg\n",e.pitch);
    oled.printf("Yaw   %7.1f deg\n",e.yaw);
    oled.printf("Tare:%s Status:%u\n",s.tareActive?"ON":"OFF",s.reportStatus);
    oled.println("Game RV: yaw drifts");
  }
#endif
  oled.display();
#else
  (void)s; (void)now;
#endif
}
