# Development stages and boundaries

## Current: sensing-only ESP32 prototype

The user has a T3 V1.6.1 available now; the Teensy needs soldering equipment.
Use the ESP32 to establish a reproducible firmware build, IMU acquisition, OLED
feedback, mounting conventions and recorded measurements. No robot actuation
commands or PWM outputs are part of this application. It does not implement the
Teensy stop/arm circuit and must not be treated as its substitute.

`src/imu.cpp` adapts BNO085 SPI reports; `include/orientation.h` contains the
hardware-independent frame math; `src/display_ui.cpp` renders the display;
`src/main.cpp` coordinates polling and the serial interface. The Python logger
records samples on the development PC. Wi-Fi, LoRa and SD are unused.

An IMU initialization failure leaves the UI and serial commands usable. Invalid
or old orientation is not published as fresh data. Sensor-reset detection
invalidates all sample validity and clears tare before report reconfiguration.
This is a cooperative development loop, not a measured real-time guarantee.

## Later: Teensy motion controller

Keep the ESP32 sensing work modular so the report handling/frame math can be
adapted to Teensy hardware once available. Do not copy T3 pin numbers into the
Teensy design. The Raspberry Pi remains the planned Linux/camera/behaviour host.

Earlier electrical files are under `hardware/reference/`. They are design history,
not fabrication releases. The initial controller v0.1 brief predates the MDD3A
selection and is superseded where the later power and stop/arm documents differ.

Known open engineering work includes READY behaviour through all reset paths,
qualified arming near RUN_OK recovery, release-memory timing, motor braking energy,
fuse coordination, battery cell protection, physical pin/footprint verification
and actual bench measurements. The stop input's shown discharge path is
(R12 + R13) * C5 = 2 ms; actual stop delay is threshold-dependent. TPS3839K33 has
a nominal 2.93 V threshold, which must be coordinated with timing-voltage limits.

## Telemetry contract, schema 1

Samples are newline-delimited JSON at 115200 baud. Firmware/library startup
diagnostics can also appear; consumers must filter on `type == sample` and
`schema == 1`. Quaternion arrays use **w,x,y,z** order; angles use roll,pitch,yaw
degrees. Gyro vectors are rad/s; acceleration vectors are m/s^2 and include gravity.
Fields with invalid/stale measurements are null. Board milliseconds wrap after
about 49.7 days and restart on reset; CSV also records host UTC and monotonic time.

Body-frame names describe the configured transform, whose identity default still
needs physical validation. `q_view_wxyz` is relative to the tare pose when active;
otherwise it equals body orientation in the sensor fusion reference frame.
It must not be confused with compass heading or translational localization.
