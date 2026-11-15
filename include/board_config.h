#pragma once

#ifndef MOBILE_IMU_ENABLED
#define MOBILE_IMU_ENABLED 1
#endif

// LilyGO T3 V1.6.1 / TTGO LoRa32 V2.1.6. Verify against your PCB labels.
// GPIO16/17 are associated with integrated flash on the PICO-D4 schematic.
// This profile borrows SD pins 13/14: LEAVE THE microSD SLOT EMPTY.
namespace board {
constexpr int kOledSda = 21;
constexpr int kOledScl = 22;
constexpr int kSpiSck = 5;
constexpr int kSpiMiso = 19;
constexpr int kSpiMosi = 27;
constexpr int kRadioCs = 18;
constexpr int kRadioReset = 23;
constexpr int kImuCs = 13;
constexpr int kImuInterrupt = 34;
constexpr int kImuReset = 14;
constexpr unsigned kReportIntervalUs = 20000;  // 50 Hz requested, not a guarantee
constexpr unsigned kTelemetryIntervalMs = 100;
constexpr unsigned kDisplayIntervalMs = 200;
constexpr unsigned kStaleMs = 300;

// Fixed mounting transform q_sensor_from_body, w/x/y/z.
// Identity initially: the sensor axes have NOT yet been aligned to the chassis.
// Body convention: +X forward, +Y left, +Z up. See docs/bringup.md.
constexpr float kMountW = 1.0f;
constexpr float kMountX = 0.0f;
constexpr float kMountY = 0.0f;
constexpr float kMountZ = 0.0f;
}  // namespace board

static_assert(board::kImuCs != board::kRadioCs, "Separate SPI chip selects required");
