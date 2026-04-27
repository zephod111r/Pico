#include "ble_provision.h"

#include <cstdlib>
#include <cstring>

#include "btstack.h"
#include "wifi_provision.h"

namespace {
BleProvision *g_instance = nullptr;

constexpr uint16_t kHandleSsidValue = ATT_CHARACTERISTIC_12345678_1234_5678_1234_56789abcdef1_SSID_VALUE_HANDLE;
constexpr uint16_t kHandlePasswordValue = ATT_CHARACTERISTIC_12345678_1234_5678_1234_56789abcdef2_PASSWORD_VALUE_HANDLE;
constexpr uint16_t kHandleApplyValue = ATT_CHARACTERISTIC_12345678_1234_5678_1234_56789abcdef3_APPLY_VALUE_HANDLE;
constexpr uint16_t kHandleControlValue = ATT_CHARACTERISTIC_12345678_1234_5678_1234_56789abcdef4_CONTROL_VALUE_HANDLE;
constexpr uint16_t kHandleStatusValue = ATT_CHARACTERISTIC_12345678_1234_5678_1234_56789abcdef5_STATUS_VALUE_HANDLE;

bool parse_floats(const char *data, size_t length, float &w, float &x, float &y, float &z) {
    char buffer[96];
    size_t count = length < sizeof(buffer) - 1 ? length : sizeof(buffer) - 1;
    std::memcpy(buffer, data, count);
    buffer[count] = '\0';
    for (size_t i = 0; i < count; ++i) {
        if (buffer[i] == ',') {
            buffer[i] = ' ';
        }
    }

    char *end = nullptr;
    w = std::strtof(buffer, &end);
    if (end == buffer) return false;
    x = std::strtof(end, &end);
    y = std::strtof(end, &end);
    z = std::strtof(end, &end);
    return true;
}

int att_write_callback(hci_con_handle_t, uint16_t att_handle, uint16_t transaction_mode,
                       uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    if (!g_instance || offset != 0) {
        return 0;
    }

    if (att_handle == kHandleSsidValue) {
        g_instance->set_ssid(reinterpret_cast<const char *>(buffer), buffer_size);
        g_instance->clear_pending_apply();
        return 0;
    }

    if (att_handle == kHandlePasswordValue) {
        g_instance->set_password(reinterpret_cast<const char *>(buffer), buffer_size);
        g_instance->clear_pending_apply();
        return 0;
    }

    if (att_handle == kHandleApplyValue) {
        g_instance->mark_pending_apply();
    }

    if (att_handle == kHandleControlValue) {
        g_instance->handle_control_write(reinterpret_cast<const char *>(buffer), buffer_size);
    }

    return 0;
}

uint16_t att_read_callback(hci_con_handle_t, uint16_t att_handle, uint16_t offset,
                           uint8_t *buffer, uint16_t buffer_size) {
    if (!g_instance) {
        return 0;
    }
    if (att_handle == kHandleStatusValue) {
        const char *status = g_instance->status();
        return att_read_callback_handle_blob(reinterpret_cast<const uint8_t *>(status),
                                             std::strlen(status), offset, buffer, buffer_size);
    }
    return 0;
}

void packet_handler(uint8_t packet_type, uint16_t, uint8_t *packet, uint16_t) {
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == BTSTACK_EVENT_STATE) {
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            bd_addr_t direct_address = {0};
            gap_advertisements_set_params(0x0030, 0x0030, 0, 0, direct_address, 0x07, 0);
            gap_advertisements_enable(1);
        }
    }
}

const uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    0x0f, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'i', 'c', 'o', ' ', 'W',
    'i', 'F', 'i', ' ', 'S', 'e', 't', 'u', 'p'};
}  // namespace

BleProvision::BleProvision() {
    wifi_config_default(&config_);
    g_instance = this;
}

void BleProvision::init() {
    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING);

    att_server_init(profile_data, att_read_callback, att_write_callback);

    btstack_packet_callback_registration_t hci_callback{};
    hci_callback.callback = &packet_handler;
    hci_add_event_handler(&hci_callback);

    gap_advertisements_set_data(sizeof(adv_data), const_cast<uint8_t *>(adv_data));
    bd_addr_t direct_address = {0};
    gap_advertisements_set_params(0x0030, 0x0030, 0, 0, direct_address, 0x07, 0);
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);
}

bool BleProvision::has_pending_apply() const {
    return pending_apply_;
}

void BleProvision::clear_pending_apply() {
    pending_apply_ = false;
}

WifiConfig BleProvision::current_config() const {
    return config_;
}

void BleProvision::set_ssid(const char *data, size_t length) {
    std::memset(config_.ssid, 0, sizeof(config_.ssid));
    size_t count = length < sizeof(config_.ssid) - 1 ? length : sizeof(config_.ssid) - 1;
    std::memcpy(config_.ssid, data, count);
}

void BleProvision::set_password(const char *data, size_t length) {
    std::memset(config_.password, 0, sizeof(config_.password));
    size_t count = length < sizeof(config_.password) - 1 ? length : sizeof(config_.password) - 1;
    std::memcpy(config_.password, data, count);
}

void BleProvision::mark_pending_apply() {
    pending_apply_ = true;
    if (apply_callback_) {
        apply_callback_(config_);
    }
}

void BleProvision::set_apply_callback(void (*callback)(const WifiConfig &config)) {
    apply_callback_ = callback;
}

void BleProvision::set_control_callback(void (*callback)(float w, float x, float y, float z)) {
    control_callback_ = callback;
}

void BleProvision::set_status(const char *status) {
    std::memset(status_, 0, sizeof(status_));
    size_t count = std::strlen(status);
    if (count >= sizeof(status_)) {
        count = sizeof(status_) - 1;
    }
    std::memcpy(status_, status, count);
}

void BleProvision::handle_control_write(const char *data, size_t length) {
    float w = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!parse_floats(data, length, w, x, y, z)) {
        set_status("control:bad");
        return;
    }
    if (control_callback_) {
        control_callback_(w, x, y, z);
        set_status("control:ok");
    }
}

const char *BleProvision::status() const {
    return status_;
}
