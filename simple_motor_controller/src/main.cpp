#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/i2c_slave.h"

#include "config.h"
#include "pico_robotics.h"
#include "quaternion_robot.h"

namespace {
struct I2cState {
    volatile size_t rx_len = 0;
    volatile bool message_ready = false;
    char rx_buffer[kMaxMessageSize] = {0};

    size_t tx_len = 0;
    size_t tx_index = 0;
    char tx_buffer[kMaxMessageSize] = {0};
} g_i2c;

char g_status[kMaxMessageSize] = "{\"status\":\"idle\"}";

float parse_json_float(const char *body, const char *key, float fallback) {
    char needle[16];
    std::snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *pos = std::strstr(body, needle);
    if (!pos) {
        return fallback;
    }
    pos = std::strchr(pos, ':');
    if (!pos) {
        return fallback;
    }
    char *end = nullptr;
    float value = std::strtof(pos + 1, &end);
    if (end == pos + 1) {
        return fallback;
    }
    return value;
}

bool parse_stop(const char *body) {
    const char *pos = std::strstr(body, "\"stop\"");
    if (!pos) {
        return false;
    }
    pos = std::strchr(pos, ':');
    if (!pos) {
        return false;
    }
    while (*pos == ':' || *pos == ' ' || *pos == '\t') {
        ++pos;
    }
    return std::strncmp(pos, "true", 4) == 0 || std::strncmp(pos, "1", 1) == 0;
}

void update_status(const char *status) {
    std::snprintf(g_status, sizeof(g_status), "{\"status\":\"%s\"}", status);
    g_i2c.tx_len = std::strlen(g_status);
    if (g_i2c.tx_len >= sizeof(g_i2c.tx_buffer)) {
        g_i2c.tx_len = sizeof(g_i2c.tx_buffer) - 1;
    }
    std::memcpy(g_i2c.tx_buffer, g_status, g_i2c.tx_len);
    g_i2c.tx_index = 0;
}

void i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    if (event == I2C_SLAVE_RECEIVE) {
        while (i2c_get_read_available(i2c)) {
            uint8_t byte = i2c_read_byte_raw(i2c);
            if (g_i2c.rx_len < kMaxMessageSize - 1) {
                g_i2c.rx_buffer[g_i2c.rx_len++] = static_cast<char>(byte);
            }
        }
    } else if (event == I2C_SLAVE_REQUEST) {
        while (i2c_get_write_available(i2c) && g_i2c.tx_index < g_i2c.tx_len) {
            i2c_write_byte_raw(i2c, static_cast<uint8_t>(g_i2c.tx_buffer[g_i2c.tx_index++]));
        }
    } else if (event == I2C_SLAVE_FINISH) {
        if (g_i2c.rx_len > 0) {
            g_i2c.rx_buffer[g_i2c.rx_len] = '\0';
            g_i2c.message_ready = true;
        }
        g_i2c.rx_len = 0;
        g_i2c.tx_index = 0;
    }
}
}  // namespace

int main() {
    stdio_init_all();

    i2c_init(i2c1, kI2cBaudrate);
    gpio_set_function(kI2cSdaPin, GPIO_FUNC_I2C);
    gpio_set_function(kI2cSclPin, GPIO_FUNC_I2C);
    gpio_pull_up(kI2cSdaPin);
    gpio_pull_up(kI2cSclPin);

    i2c_slave_init(i2c1, kI2cAddress, i2c_handler);

    KitronikPicoRobotics robot;
    update_status("ready");

    while (true) {
        if (g_i2c.message_ready) {
            g_i2c.message_ready = false;
            const char *payload = g_i2c.rx_buffer;

            if (parse_stop(payload)) {
                QuaternionInput q{1.0f, 0.0f, 0.0f, 0.0f};
                quaternion_to_movement(robot, q);
                update_status("stopped");
            } else {
                QuaternionInput q{
                    parse_json_float(payload, "w", 1.0f),
                    parse_json_float(payload, "x", 0.0f),
                    parse_json_float(payload, "y", 0.0f),
                    parse_json_float(payload, "z", 0.0f),
                };
                quaternion_to_movement(robot, q);
                update_status("ok");
            }
        }
        sleep_ms(5);
    }

    return 0;
}
