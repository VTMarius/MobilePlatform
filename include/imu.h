#pragma once
#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include "orientation.h"

struct ImuSnapshot {
  bool connected = false;
  bool valid = false;
  bool tareActive = false;
  bool gyroValid = false;
  bool accelValid = false;
  uint32_t orientationMs = 0, gyroMs = 0, accelMs = 0;
  uint32_t reports = 0, resets = 0, rejected = 0;
  uint64_t sensorTimestampUs = 0;
  uint8_t reportStatus = 0;
  orientation::Quaternion sensor, body, view;
  orientation::Vector3 gyroBody{0,0,0}, accelBody{0,0,0};
};

class Imu {
 public:
  Imu();
  bool begin();
  void poll();
  bool tare(uint32_t now);
  void clearTare();
  const ImuSnapshot& snapshot() const { return state_; }
 private:
  bool configureReports();
  void updateView();
  Adafruit_BNO08x sensor_;
  ImuSnapshot state_;
  orientation::Quaternion mount_, reference_;
};
