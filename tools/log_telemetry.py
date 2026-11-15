#!/usr/bin/env python3
"""Capture MobilePlatform sample messages as CSV; firmware chatter is ignored."""
import argparse
import csv
import datetime as dt
import json
import math
from pathlib import Path
import sys
import time


def _reject_constant(value):
    raise ValueError("Non-finite JSON number: " + value)


def _finite_float(text):
    value = float(text)
    if not math.isfinite(value):
        raise ValueError("JSON number out of finite range")
    return value


def decode_sample(line):
    """Return a schema-1 sample or None, never treating diagnostic text as data."""
    try:
        value = json.loads(line, parse_constant=_reject_constant, parse_float=_finite_float)
    except (ValueError, TypeError):
        return None
    if not isinstance(value, dict) or value.get("type") != "sample":
        return None
    if type(value.get("schema")) is not int or value["schema"] != 1:
        return None
    if type(value.get("t_ms")) is not int or not 0 <= value["t_ms"] <= 0xFFFFFFFF:
        return None
    if type(value.get("orientation_valid")) is not bool:
        return None
    if value["orientation_valid"]:
        for field, length in (("q_sensor_wxyz", 4), ("q_body_wxyz", 4),
                              ("q_view_wxyz", 4), ("rpy_view_deg", 3)):
            array = value.get(field)
            if not isinstance(array, list) or len(array) != length:
                return None
            if not all(type(n) in (int, float) and math.isfinite(n) for n in array):
                return None
    else:
        for field in ("q_sensor_wxyz", "q_body_wxyz", "q_view_wxyz", "rpy_view_deg"):
            if value.get(field) is not None:
                return None
    return value


FIELDS = ["host_utc", "host_monotonic_ns", "t_ms", "orientation_valid", "tare_active",
          "orientation_age_ms", "report_status", "roll_deg", "pitch_deg", "yaw_deg",
          "qw", "qx", "qy", "qz", "payload_json"]


def csv_row(sample, utc=None, monotonic_ns=None):
    angles = sample.get("rpy_view_deg") or [None] * 3
    quat = sample.get("q_view_wxyz") or [None] * 4
    result = {key: sample.get(key) for key in FIELDS}
    result.update(host_utc=utc or dt.datetime.now(dt.timezone.utc).isoformat(),
                  host_monotonic_ns=monotonic_ns if monotonic_ns is not None else time.monotonic_ns(),
                  roll_deg=angles[0], pitch_deg=angles[1], yaw_deg=angles[2],
                  qw=quat[0], qx=quat[1], qy=quat[2], qz=quat[3],
                  payload_json=json.dumps(sample, separators=(",", ":"), allow_nan=False))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="COM5 on Windows, /dev/ttyUSB0 on Linux, for example")
    parser.add_argument("--output", type=Path, help="New CSV file; existing files are not overwritten")
    parser.add_argument("--seconds", type=float, default=0, help="Duration; 0 means until Ctrl+C")
    parser.add_argument("--tare", action="store_true", help="Tare after first fresh orientation; keep chassis still")
    parser.add_argument("--list-ports", action="store_true")
    args = parser.parse_args()
    if args.seconds < 0 or not math.isfinite(args.seconds):
        parser.error("--seconds must be finite and non-negative")
    try:
        import serial
        import serial.tools.list_ports
    except ImportError:
        parser.error("Install dependencies: python -m pip install -r requirements-dev.txt")
    if args.list_ports:
        for port in serial.tools.list_ports.comports():
            print(f"{port.device}: {port.description}")
        return 0
    if not args.port or not args.output:
        parser.error("--port and --output are required unless --list-ports is used")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    count = 0
    tare_sent = False
    try:
        with args.output.open("x", newline="", encoding="utf-8") as output:
            writer = csv.DictWriter(output, fieldnames=FIELDS)
            writer.writeheader()
            with serial.Serial(args.port, 115200, timeout=0.5) as port:
                start = time.monotonic()
                while not args.seconds or time.monotonic() - start < args.seconds:
                    line = port.read_until(b"\n", size=8192).decode("utf-8", errors="replace")
                    sample = decode_sample(line)
                    if sample is None:
                        continue
                    if args.tare and not tare_sent and sample["orientation_valid"]:
                        port.write(b"tare\n")
                        tare_sent = True
                    writer.writerow(csv_row(sample))
                    count += 1
                    output.flush()
    except KeyboardInterrupt:
        pass
    except (OSError, serial.SerialException) as exc:
        print(f"Capture failed: {exc}", file=sys.stderr)
        return 1
    print(f"Saved {count} samples to {args.output}")
    return 0 if count else 2


if __name__ == "__main__":
    raise SystemExit(main())
