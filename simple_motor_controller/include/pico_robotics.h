#pragma once

#include "hardware/i2c.h"

class KitronikPicoRobotics {
public:
    explicit KitronikPicoRobotics(i2c_inst_t *i2c_instance = i2c0, uint sda = 8, uint scl = 9);

    void motor_on(int motor, char direction, int speed);
    void motor_off(int motor);

private:
    void init_pca();
    void write_reg(uint8_t reg, uint8_t value);

    i2c_inst_t *i2c_;
    uint8_t chip_address_;

    static constexpr uint8_t kMotRegBase = 0x28;
    static constexpr uint8_t kRegOffset = 4;
    static constexpr uint8_t kPrescaleVal = 0x79;
};
