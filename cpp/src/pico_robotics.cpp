#include "pico_robotics.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

KitronikPicoRobotics::KitronikPicoRobotics(i2c_inst_t *i2c_instance, uint sda, uint scl)
    : i2c_(i2c_instance), chip_address_(108) {
    i2c_init(i2c_, 100000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);

    init_pca();
}

void KitronikPicoRobotics::write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(i2c_, chip_address_, buf, 2, false);
}

void KitronikPicoRobotics::init_pca() {
    // Soft reset of the I2C chip
    uint8_t reset = 0x06;
    i2c_write_blocking(i2c_, 0x00, &reset, 1, false);

    // Set PWM frequency prescale value.
    write_reg(0xfe, kPrescaleVal);

    // Block write outputs to off.
    write_reg(0xfa, 0x00);
    write_reg(0xfb, 0x00);
    write_reg(0xfc, 0x00);
    write_reg(0xfd, 0x00);

    // Come out of sleep.
    write_reg(0x00, 0x01);

    sleep_us(500);
}

void KitronikPicoRobotics::motor_on(int motor, char direction, int speed) {
    if (speed < 0) {
        speed = 0;
    } else if (speed > 100) {
        speed = 100;
    }

    if (motor < 1 || motor > 4) {
        return;
    }

    uint8_t motor_reg = kMotRegBase + (2 * (motor - 1) * kRegOffset);
    int pwm_val = static_cast<int>(speed * 40.95f);
    uint8_t low_byte = pwm_val & 0xFF;
    uint8_t high_byte = (pwm_val >> 8) & 0xFF;

    if (direction == 'f') {
        write_reg(motor_reg, low_byte);
        write_reg(motor_reg + 1, high_byte);
        write_reg(motor_reg + 4, 0);
        write_reg(motor_reg + 5, 0);
    } else if (direction == 'r') {
        write_reg(motor_reg + 4, low_byte);
        write_reg(motor_reg + 5, high_byte);
        write_reg(motor_reg, 0);
        write_reg(motor_reg + 1, 0);
    } else {
        write_reg(motor_reg + 4, 0);
        write_reg(motor_reg + 5, 0);
        write_reg(motor_reg, 0);
        write_reg(motor_reg + 1, 0);
    }
}

void KitronikPicoRobotics::motor_off(int motor) {
    motor_on(motor, 'f', 0);
}
