#pragma once

#include <cstddef>

struct WifiConfig {
    char ssid[33];
    char password[65];
};

void wifi_config_default(WifiConfig *config);
bool wifi_config_is_valid(const WifiConfig &config);
bool wifi_config_load(WifiConfig *config);
bool wifi_config_save(const WifiConfig &config);
