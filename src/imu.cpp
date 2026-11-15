#include "imu.h"
#include <SPI.h>
#include "board_config.h"

Imu::Imu() : sensor_(board::kImuReset),
    mount_(board::kMountW,board::kMountX,board::kMountY,board::kMountZ) {}

bool Imu::configureReports() {
  const bool orientation = sensor_.enableReport(SH2_GAME_ROTATION_VECTOR,board::kReportIntervalUs);
  const bool gyro = sensor_.enableReport(SH2_GYROSCOPE_CALIBRATED,board::kReportIntervalUs);
  const bool accel = sensor_.enableReport(SH2_ACCELEROMETER,board::kReportIntervalUs);
  return orientation && gyro && accel;
}
bool Imu::begin() {
  state_ = ImuSnapshot{};
#if !MOBILE_IMU_ENABLED
  return false;
#endif
  if (!orientation::normalize(mount_,mount_)) return false;
  // The radio and BNO085 share the T3 SPI wires, with different chip selects.
  // Leave the onboard radio deselected and held in reset; no LoRa driver runs.
  digitalWrite(board::kRadioCs,HIGH); pinMode(board::kRadioCs,OUTPUT);
  digitalWrite(board::kRadioReset,LOW); pinMode(board::kRadioReset,OUTPUT);
  digitalWrite(board::kImuCs,HIGH); pinMode(board::kImuCs,OUTPUT);
  pinMode(board::kImuInterrupt,INPUT);  // GPIO34 has no internal pull-up.
  SPI.begin(board::kSpiSck,board::kSpiMiso,board::kSpiMosi,board::kImuCs);
  if (!sensor_.begin_SPI(board::kImuCs,board::kImuInterrupt,&SPI)) return false;
  state_.connected = configureReports();
  return state_.connected;
}
void Imu::poll() {
  if (!state_.connected) return;
  if (sensor_.wasReset()) {
    ++state_.resets;
    state_.valid = state_.gyroValid = state_.accelValid = state_.tareActive = false;
    state_.connected = configureReports();
    if (!state_.connected) return;
  }
  // Limit events per loop so input, logging and OLED still get serviced.
  for (int i=0; i<6; ++i) {
    if (digitalRead(board::kImuInterrupt) != LOW) break;
    sh2_SensorValue_t event{};
    if (!sensor_.getSensorEvent(&event)) break;
    const uint32_t now = millis();
    if (event.sensorId == SH2_GAME_ROTATION_VECTOR) {
      const auto& v = event.un.gameRotationVector;
      orientation::Quaternion normalized;
      if (!orientation::normalize({v.real,v.i,v.j,v.k},normalized)) { ++state_.rejected; continue; }
      state_.sensor = normalized;
      state_.body = orientation::bodyOrientation(normalized,mount_);
      state_.orientationMs = now;
      state_.sensorTimestampUs = event.timestamp;
      state_.reportStatus = event.status & 3;
      state_.valid = true;
      ++state_.reports;
      updateView();
    } else if (event.sensorId == SH2_GYROSCOPE_CALIBRATED) {
      const auto& v = event.un.gyroscope;
      if (!(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z))) continue;
      state_.gyroBody = orientation::rotate(orientation::conjugate(mount_),{v.x,v.y,v.z});
      state_.gyroMs = now; state_.gyroValid = true;
    } else if (event.sensorId == SH2_ACCELEROMETER) {
      const auto& v = event.un.accelerometer;
      if (!(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z))) continue;
      state_.accelBody = orientation::rotate(orientation::conjugate(mount_),{v.x,v.y,v.z});
      state_.accelMs = now; state_.accelValid = true;
    }
  }
}
void Imu::updateView() {
  state_.view = state_.tareActive ? orientation::relativeTo(reference_,state_.body) : state_.body;
}
bool Imu::tare(uint32_t now) {
  if (!state_.valid || uint32_t(now-state_.orientationMs) > board::kStaleMs) return false;
  reference_ = state_.body; state_.tareActive = true; updateView(); return true;
}
void Imu::clearTare() { state_.tareActive = false; updateView(); }
