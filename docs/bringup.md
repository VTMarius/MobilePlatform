# ESP32 sensing bring-up

## Hardware assumptions

- Board identified by the user: **T3 V1.6.1**, mapped to the LilyGO T3 / TTGO
  LoRa32 v2.1.6 family. PlatformIO board ID: `ttgo-lora32-v21`.
- IMU: **Adafruit BNO085** breakout identified in the earlier photographs.
  A BNO055, MPU9250 or bare Bosch sensor requires a different driver/wiring.
- Display: the T3's usual **SSD1306, 128 x 64, I2C 0x3C**. Firmware also probes
  0x3D. An acknowledging device does not prove it is an SSD1306. If your screen
  uses SH1106 or another controller, replace the display driver after identification.

## Wiring - this ESP32 profile only

Power off before changing jumper wires. Initially power the T3 through its own USB
connector, and keep the motor circuit disconnected. Do not apply the robot's 3S
battery to the T3's battery connector: that connector is for its own single-cell supply.

| BNO085 breakout label | T3 connection | Purpose |
| --- | --- | --- |
| VIN | 3V3 | Power at the MCU logic voltage |
| GND | GND | Common reference |
| SCL | GPIO5 | SPI clock |
| SDA | GPIO19 | SPI MISO: sensor to ESP32 |
| DI | GPIO27 | SPI MOSI: ESP32 to sensor |
| CS | GPIO13 | IMU chip select |
| INT | GPIO34 | Active-low data-ready input |
| RST | GPIO14 | IMU reset output |
| P0 and P1 | 3V3 | Both high for SPI mode, present before sensor reset |
| 3Vo and BT | Unconnected | Not needed in this configuration |

**Leave the microSD slot empty.** GPIO13 and GPIO14 are connected to that slot;
this temporary profile deliberately uses them for the IMU. Do not initialize SD.
GPIO16/17 are associated with integrated flash in LilyGO's PICO-D4 schematic;
they are not used as spare IMU pins.

SPI clock/data are shared with the onboard radio. Firmware holds radio CS
(GPIO18) high and radio reset (GPIO23) low. LoRa is not initialized or transmitted.
Changing that later requires deliberate shared-bus ownership and pin planning.

GPIO34 is input-only and has no internal pull-up. Check that INT has the pull-up
required by the actual breakout; add one to 3V3 if the module requires it. Use
short jumpers with strain relief and an insulated mounting surface.

The OLED uses **SDA GPIO21 / SCL GPIO22**. A built-in screen needs no additional
signal wires. If using an external screen, also connect its specified power and
ground; avoid placing two displays with the same address on this bus.

## First upload

If only the T3 is ready, use `pio run -e lilygo_t3_display -t upload` first.
This displays a board-check screen and accepts serial commands without trying
to initialize an absent IMU. The sensing environments are separate build choices.

1. Build the default environment without hardware first.
2. Connect the board with a USB data cable. Find its port with `pio device list`.
3. Upload and open the 115200-baud monitor.
4. Look for `IMU reports enabled` and fresh `sample` messages. The OLED should show
   angles, or an IMU diagnostic if the sensor is absent.
5. Send `stream off` to stop periodic sample output, then `status` for a snapshot.
6. If initialization failed, correct wiring while powered off; reboot, or use
   `retry` once connections are correct. `retry` clears the viewing reference.

No IMU data? Check sensor model, P0/P1, SCL/SDA's SPI meanings, CS, INT, RST and
the 3.3 V supply. SPI is chosen because Adafruit documents BNO08x I2C problems
with ESP32. The display can still use ordinary I2C.

## Commands

| Command | Behaviour |
| --- | --- |
| `help` | Show available commands |
| `status` | Emit one JSON sample |
| `tare` | Set this fresh body orientation as the temporary viewing reference |
| `clear` | Remove the viewing reference |
| `retry` | Reinitialize the IMU and reports; clear tare |
| `stream on` / `stream off` | Enable/disable periodic serial samples |

Use a newline/Enter after each command. Requested sensor reports are 50 Hz,
serial snapshots 10 Hz and display refresh 5 Hz. Actual rates must be measured.
Orientation older than 300 ms is marked stale, not displayed as current data.

## Mounting and calibration are different operations

Choose a robot body frame: +X forward, +Y left, +Z up. Mark it on the chassis.
Mount the sensor rigidly and record the sensor axes relative to that frame.
The constant quaternion in `include/board_config.h` is **q_sensor_from_body**:
it rotates a body-frame vector into sensor coordinates. The implemented convention is:

```
q_world_from_body = q_world_from_sensor * q_sensor_from_body
q_reference_from_body = inverse(q_world_from_reference_body) * q_world_from_body
```

The second equation is used for the viewing quaternion only when tared. Check the
sensor report convention and expected signs with deliberate rotations about all
three axes before accepting the mount configuration. Identity is a starting
assumption, not evidence of alignment. The gyro and acceleration vectors use the
inverse mounting transform; accelerometer output includes gravity.

`tare` merely chooses a reference pose. It cannot identify a full mounting
rotation from a stationary pose, calibrate accelerometer/gyro biases, establish
magnetic north, or establish wheel distance. Tare is deliberately RAM-only:
power cycling, IMU reinitialization or sensor reset invalidates its world reference.

The game rotation vector uses accelerometer/gyro fusion, not magnetometer heading.
Yaw drift is expected. Report status is the sensor's 0-3 status code, not a
validated robot-calibration score. Explicit BNO085 calibration/persistence and
magnetometer assessment are later experiments, not implemented by `tare`.

## First recorded experiment

Record stationary data, then separate hand-applied roll, pitch and yaw movements.
Check axis signs, rate, noise, drift and stale-data behaviour. Keep motors unpowered.
Commit the mounting configuration and notes once established; captures remain local.

## Primary references

- [LilyGO board definitions](https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/ArduinoLoRa/LoRaSender/utilities.h)
- [LilyGO T3 V1.6.1 schematic](https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_V1.6.1.pdf)
- [PlatformIO board definition](https://docs.platformio.org/en/latest/boards/espressif32/ttgo-lora32-v21.html)
- [Adafruit BNO085 pinouts](https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085/pinouts)
- [Adafruit BNO085 I2C limitations](https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085)
- [BNO08x library and SH-2 report definitions](https://github.com/adafruit/Adafruit_BNO08x)
