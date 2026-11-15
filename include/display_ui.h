#pragma once
#include <Arduino.h>
#include "imu.h"
bool beginDisplay();
void updateDisplay(const ImuSnapshot& state, uint32_t now);
