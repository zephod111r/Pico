#pragma once

#include <cstdint>

constexpr uint8_t kI2cAddress = 0x12;
constexpr uint kI2cSdaPin = 2;
constexpr uint kI2cSclPin = 3;
constexpr uint kI2cBaudrate = 100000;

constexpr size_t kMaxMessageSize = 256;
