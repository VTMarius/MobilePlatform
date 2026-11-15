# Validation status

Prepared 15 November 2026 for the LilyGO T3 V1.6.1 sensing prototype.

| Check | Status |
| --- | --- |
| Python telemetry parser/CSV mapping checks | PASS: 5 tests |
| Native C++ quaternion/frame checks | PASS |
| PlatformIO default firmware build | IN PROGRESS: dependency/toolchain installation |
| Serial-only and display-only firmware builds | Pending default build |
| GitHub Actions | Configured; execution status must be checked on GitHub |
| Physical upload, OLED and BNO085 operation | Not tested: no connected board available here |
| Physical mounting, axis signs and sensor calibration | Not established |

Build logs and this status will be updated after firmware compilation. A build
does not establish hardware operation, real-time behaviour or motor safety.

Dependencies: PlatformIO Core 6.1.18, espressif32 platform 6.12.0 / Arduino core
2.0.17, Adafruit BNO08x 1.2.7, GFX Library 1.13.0 and SSD1306 2.5.17.
