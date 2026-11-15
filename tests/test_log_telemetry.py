import json
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from log_telemetry import csv_row, decode_sample


def sample():
    return {"type": "sample", "schema": 1, "t_ms": 42, "orientation_valid": True,
            "tare_active": True, "q_sensor_wxyz": [1, 0, 0, 0],
            "q_body_wxyz": [1, 0, 0, 0], "q_view_wxyz": [1, 0, 0, 0],
            "rpy_view_deg": [0, 0, 0]}


class TelemetryTests(unittest.TestCase):
    def test_diagnostics_and_other_messages_are_ignored(self):
        for line in ("Booting ESP32", '{"type":"info"}', "[]", "{broken", "null"):
            self.assertIsNone(decode_sample(line))

    def test_valid_sample_and_csv_mapping(self):
        message = sample()
        self.assertEqual(decode_sample(json.dumps(message)), message)
        row = csv_row(message, "2026-11-15T12:00:00+00:00", 123)
        self.assertEqual(row["qw"], 1)
        self.assertEqual(row["host_monotonic_ns"], 123)
        self.assertEqual(json.loads(row["payload_json"]), message)

    def test_invalid_quaternion_and_nan_are_rejected(self):
        message = sample()
        message["q_sensor_wxyz"] = [1, 0, 0]
        self.assertIsNone(decode_sample(json.dumps(message)))
        self.assertIsNone(decode_sample('{"type":"sample","schema":1,"t_ms":1,"orientation_valid":false,"bad":1e999}'))
        message = sample()
        message["q_view_wxyz"][0] = float("nan")
        self.assertIsNone(decode_sample(json.dumps(message)))

    def test_stale_samples_preserve_missing_values(self):
        message = {"type": "sample", "schema": 1, "t_ms": 0,
                   "orientation_valid": False, "rpy_view_deg": None, "q_view_wxyz": None}
        decoded = decode_sample(json.dumps(message))
        self.assertIsNotNone(decoded)
        self.assertIsNone(csv_row(decoded)["qw"])
        message["q_view_wxyz"] = [1]
        self.assertIsNone(decode_sample(json.dumps(message)))

    def test_unknown_schema_and_wrong_timestamp_are_rejected(self):
        message = sample()
        for timestamp in (-1, True, "42", 2**32):
            message["t_ms"] = timestamp
            self.assertIsNone(decode_sample(json.dumps(message)))
        message = sample()
        message["schema"] = 2
        self.assertIsNone(decode_sample(json.dumps(message)))


if __name__ == "__main__":
    unittest.main()
