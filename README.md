# MobilePlatform

Devastator ROB0128 robotics development: sensing and chassis alignment first,
followed by a separately validated motion controller and camera-based behaviours.

**Current target:** LilyGO **T3 V1.6.1** (ESP32), Adafruit **BNO085 over SPI**, and
a **128 x 64 SSD1306 I2C OLED**. The default environment uses the T3's usual OLED
pins. The display controller and physical wiring still need checking on the actual unit.

This firmware reads orientation, angular velocity and acceleration, displays
roll/pitch/yaw, and streams timestamped JSON over USB serial. It has a session-only
`tare` command and a CSV capture tool. **Motor control is not implemented.**
The Teensy stop/arm design is preserved as a draft reference, not ESP32 firmware.

## Start in VS Code + PlatformIO

```sh
git clone https://github.com/VTMarius/MobilePlatform.git
cd MobilePlatform
code .
```

1. Install the **PlatformIO IDE** extension when prompted.
2. Let PlatformIO initialize, then select environment **`lilygo_t3`**.
3. Click **Build**. You can do this without connecting the board or IMU.
4. Check the [wiring table](docs/bringup.md), keep the microSD slot empty, then
   connect the T3 to USB with a data cable and select **Upload**.
5. Open **Monitor**, 115200 baud. Send `help` followed by Enter for commands.

CLI equivalent, from a terminal where PlatformIO is installed:

```sh
pio run -e lilygo_t3
pio run -e lilygo_t3 -t upload
pio device monitor -b 115200 --echo
```

PlatformIO normally detects the upload port. Add `--upload-port COM5` (Windows)
or your Linux serial device if needed. If upload times out, retry at 115200 baud
by changing `upload_speed` in `platformio.ini`.

The firmware continues with serial diagnostics if no OLED or IMU is detected.
`lilygo_t3_serial` builds the same sensing application without OLED initialization.
**No IMU wired yet?** Select **`lilygo_t3_display`** to upload a display/serial
check immediately, without probing the IMU. Return to `lilygo_t3` once wired.

## First chassis experiment

Secure the IMU rigidly, note its axis markings, and leave the motor power disconnected.
Use the OLED/serial output to check hand-applied roll, pitch and yaw. While the robot
is stationary in a known pose, send `tare` to establish a temporary viewing reference.
This is **not** sensor bias calibration or automatic mounting-axis identification.

The game rotation vector deliberately avoids magnetometer heading. Yaw can drift;
it is not compass north. The mounting transform defaults to identity and must be
verified before interpreting the data as robot-body coordinates.

See [bring-up and mounting](docs/bringup.md) and [architecture and limitations](docs/architecture.md).

## Record measurements

Close PlatformIO Monitor first; only one program should own the serial port.

```sh
python -m pip install -r requirements-dev.txt
python tools/log_telemetry.py --list-ports
python tools/log_telemetry.py --port COM5 --output captures/bench-01.csv --seconds 60
```

Replace `COM5` with the listed port. Optional `--tare` sends a tare command after
the first fresh orientation arrives; keep the chassis stationary. Opening the port
may reset the T3. Raw device messages are retained in each CSV row's `payload_json`.
Captures are ignored by Git. Existing capture files are not overwritten.

## Repository map

| Path | Purpose |
| --- | --- |
| `platformio.ini` | Pinned ESP32 platform, libraries and build environments |
| `include/board_config.h` | T3 pin mapping, rates and fixed mounting transform |
| `include/orientation.h` | Platform-independent quaternion/frame calculations |
| `src/` | IMU acquisition, OLED display and serial commands |
| `tools/log_telemetry.py` | Host-side CSV recording |
| `tests/` | Quaternion/frame and telemetry-decoder checks |
| `docs/` | Wiring, development workflow and design status |
| `hardware/reference/` | Earlier power and Teensy stop/arm design files; draft |

## Host checks

```sh
python -m unittest discover -s tests -p "test_*.py" -v
g++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/test_orientation.cpp -o /tmp/test_orientation
/tmp/test_orientation
```

The C++ command is for a Linux/macOS shell with a compiler. On Windows you can use
WSL or another C++ toolchain; PlatformIO firmware builds work directly in VS Code.
GitHub Actions runs both firmware environments and the host checks.

Build success proves compilation, not wiring, axis alignment, timing or hardware
operation. Initial validation results are recorded in [docs/validation.md](docs/validation.md).
