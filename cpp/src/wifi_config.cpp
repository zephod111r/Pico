#include "wifi_config.h"

#include <cstring>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#include "config.h"

namespace {
constexpr uint32_t kMagic = 0x57494649;  // "WIFI"
constexpr size_t kConfigStorageSize = FLASH_SECTOR_SIZE;
constexpr uint32_t kFlashOffset = PICO_FLASH_SIZE_BYTES - kConfigStorageSize;

struct PersistedConfig {
    uint32_t magic;
    uint32_t crc32;
    WifiConfig config;
};

uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

const PersistedConfig *flash_config() {
    return reinterpret_cast<const PersistedConfig *>(XIP_BASE + kFlashOffset);
}
}

void wifi_config_default(WifiConfig *config) {
    if (!config) {
        return;
    }
    std::memset(config, 0, sizeof(WifiConfig));
    std::strncpy(config->ssid, WIFI_SSID, sizeof(config->ssid) - 1);
    std::strncpy(config->password, WIFI_PASSWORD, sizeof(config->password) - 1);
}

bool wifi_config_is_valid(const WifiConfig &config) {
    return config.ssid[0] != '\0';
}

bool wifi_config_load(WifiConfig *config) {
    if (!config) {
        return false;
    }
    const PersistedConfig *stored = flash_config();
    if (stored->magic != kMagic) {
        return false;
    }
    uint32_t expected_crc = crc32(reinterpret_cast<const uint8_t *>(&stored->config), sizeof(WifiConfig));
    if (expected_crc != stored->crc32) {
        return false;
    }
    *config = stored->config;
    return true;
}

bool wifi_config_save(const WifiConfig &config) {
    PersistedConfig persisted{};
    persisted.magic = kMagic;
    persisted.config = config;
    persisted.crc32 = crc32(reinterpret_cast<const uint8_t *>(&persisted.config), sizeof(WifiConfig));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(kFlashOffset, kConfigStorageSize);
    uint8_t page[FLASH_PAGE_SIZE];
    std::memset(page, 0xFF, sizeof(page));
    std::memcpy(page, &persisted, sizeof(PersistedConfig));
    flash_range_program(kFlashOffset, page, sizeof(page));
    restore_interrupts(ints);
    return true;
}
