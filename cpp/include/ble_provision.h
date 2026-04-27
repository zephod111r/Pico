#pragma once

#include <cstddef>

#include "wifi_config.h"

class BleProvision {
public:
    BleProvision();

    void init();
    bool has_pending_apply() const;
    void clear_pending_apply();
    WifiConfig current_config() const;
    void set_ssid(const char *data, size_t length);
    void set_password(const char *data, size_t length);
    void mark_pending_apply();
    void set_apply_callback(void (*callback)(const WifiConfig &config));
    void set_control_callback(void (*callback)(float w, float x, float y, float z));
    void set_status(const char *status);
    void handle_control_write(const char *data, size_t length);
    const char *status() const;

private:
    WifiConfig config_{};
    bool pending_apply_ = false;
    void (*apply_callback_)(const WifiConfig &config) = nullptr;
    void (*control_callback_)(float w, float x, float y, float z) = nullptr;
    char status_[64] = "idle";
};
