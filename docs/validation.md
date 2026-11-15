# Validation status

Prepared 15 November 2026 for the LilyGO T3 V1.6.1 sensing prototype.

| Check | Status |
| --- | --- |
| Python telemetry parser/CSV mapping checks | PASS: 5 tests |
| Native C++ quaternion/frame checks | PASS |
| PlatformIO default firmware build | PASS: local build and GitHub Actions |
| Serial-only and display-only firmware builds | PASS: GitHub Actions |
| GitHub Actions | PASS: all three builds and both host check suites |
| Physical upload, OLED and BNO085 operation | Not tested: no connected board available here |
| Physical mounting, axis signs and sensor calibration | Not established |

Verified source commit: `697f42d66c72f16fcaeaf8a7b3b0933fad5f7127`.
[Successful CI run and build logs](https://github.com/VTMarius/MobilePlatform/actions/runs/43744661661).
The subsequent documentation-only commit records these results; it does not change firmware.
A build does not establish hardware operation, real-time behaviour or motor safety.

Dependencies: PlatformIO Core 6.1.18, espressif32 platform 6.12.0 / Arduino core
2.0.17, Adafruit BNO08x 1.2.7, GFX Library 1.13.0 and SSD1306 2.5.17.
