#include <Arduino.h>
#include <cstring>
#include "board_config.h"
#include "display_ui.h"
#include "imu.h"

namespace {
Imu imu;
bool streaming = true;
uint32_t lastTelemetry = 0, lastDisplay = 0;
char command[48]; size_t commandLength = 0; bool commandOverflow = false;
void info(const char* message) { Serial.printf("{\"type\":\"info\",\"message\":\"%s\"}\n",message); }
void quaternion(const orientation::Quaternion& q) {
  Serial.printf("[%.7f,%.7f,%.7f,%.7f]",q.w,q.x,q.y,q.z);
}
void vector(const orientation::Vector3& v) { Serial.printf("[%.6f,%.6f,%.6f]",v.x,v.y,v.z); }
void telemetry(uint32_t now) {
  const auto& s = imu.snapshot();
  const bool fresh = s.valid && uint32_t(now-s.orientationMs)<=board::kStaleMs;
  Serial.printf("{\"type\":\"sample\",\"schema\":1,\"t_ms\":%lu,\"imu_ok\":%s,\"orientation_valid\":%s,\"reports\":%lu,\"resets\":%lu,\"rejected\":%lu,\"tare_active\":%s,\"report_status\":%u,\"orientation_age_ms\":",
    static_cast<unsigned long>(now),s.connected?"true":"false",fresh?"true":"false",
    static_cast<unsigned long>(s.reports),static_cast<unsigned long>(s.resets),
    static_cast<unsigned long>(s.rejected),s.tareActive?"true":"false",s.reportStatus);
  if (s.valid) Serial.print(static_cast<unsigned long>(now-s.orientationMs)); else Serial.print("null");
  Serial.printf(",\"sensor_timestamp_us\":%llu,\"q_sensor_wxyz\":",static_cast<unsigned long long>(s.sensorTimestampUs));
  if (fresh) quaternion(s.sensor); else Serial.print("null");
  Serial.print(",\"q_body_wxyz\":"); if (fresh) quaternion(s.body); else Serial.print("null");
  Serial.print(",\"q_view_wxyz\":"); if (fresh) quaternion(s.view); else Serial.print("null");
  Serial.print(",\"rpy_view_deg\":");
  if (fresh) { const auto e=orientation::eulerDegrees(s.view); Serial.printf("[%.3f,%.3f,%.3f]",e.roll,e.pitch,e.yaw); }
  else Serial.print("null");
  Serial.print(",\"gyro_body_rad_s\":");
  if (s.gyroValid && uint32_t(now-s.gyroMs)<=board::kStaleMs) vector(s.gyroBody); else Serial.print("null");
  Serial.print(",\"accel_body_m_s2\":");
  if (s.accelValid && uint32_t(now-s.accelMs)<=board::kStaleMs) vector(s.accelBody); else Serial.print("null");
  Serial.println("}");
}
void executeCommand(const char* text) {
  if (!std::strcmp(text,"help")) info("Commands: help | status | tare | clear | retry | stream on | stream off");
  else if (!std::strcmp(text,"status")) telemetry(millis());
  else if (!std::strcmp(text,"tare")) info(imu.tare(millis())?"Tare set for this sensor session":"Tare rejected: no fresh orientation");
  else if (!std::strcmp(text,"clear")) { imu.clearTare(); info("Tare cleared"); }
  else if (!std::strcmp(text,"retry")) info(imu.begin()?"IMU reports enabled; tare cleared":"IMU initialization failed");
  else if (!std::strcmp(text,"stream on")) { streaming=true; info("Streaming enabled"); }
  else if (!std::strcmp(text,"stream off")) { streaming=false; info("Streaming disabled"); }
  else info("Unknown command; enter help");
}
void pollCommands() {
  for (unsigned count=0; count<64 && Serial.available(); ++count) {
    const char ch=char(Serial.read());
    if (ch=='\r') continue;
    if (ch=='\n') {
      if (commandOverflow) info("Command too long");
      else if (commandLength) { command[commandLength]='\0'; executeCommand(command); }
      commandLength=0; commandOverflow=false;
    } else if (commandLength+1<sizeof(command)) command[commandLength++]=ch;
    else commandOverflow=true;
  }
}
}  // namespace
void setup() {
  Serial.begin(115200); delay(250);
  info("MobilePlatform: LilyGO T3 V1.6.1, BNO085 SPI, sensing only");
  info(beginDisplay()?"SSD1306 display initialized":"OLED unavailable or disabled; serial remains active");
#if MOBILE_IMU_ENABLED
  info(imu.begin()?"IMU reports enabled":"IMU initialization failed; check wiring then enter retry");
#else
  info("Display-only build: IMU initialization disabled");
#endif
  info("Use help for commands; tare is a display reference, not sensor calibration");
}
void loop() {
  imu.poll(); pollCommands();
  const uint32_t now=millis();
  if (streaming && uint32_t(now-lastTelemetry)>=board::kTelemetryIntervalMs) { lastTelemetry=now; telemetry(now); }
  if (uint32_t(now-lastDisplay)>=board::kDisplayIntervalMs) { lastDisplay=now; updateDisplay(imu.snapshot(),now); }
  delay(1);
}
