#include <cstdio>
#include <cstdlib>
#include <string>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"

#include "ble_provision.h"
#include "http_server.h"
#include "index_html.h"
#include "pico_robotics.h"
#include "quaternion_robot.h"
#include "wifi_config.h"

namespace {
std::string http_response(int status, const char *content_type, const std::string &body) {
    const char *status_text = "OK";
    switch (status) {
        case 200:
            status_text = "OK";
            break;
        case 400:
            status_text = "Bad Request";
            break;
        case 404:
            status_text = "Not Found";
            break;
        case 405:
            status_text = "Method Not Allowed";
            break;
        case 500:
            status_text = "Internal Server Error";
            break;
        default:
            status_text = "Unknown";
            break;
    }

    std::string response = "HTTP/1.1 ";
    response += std::to_string(status);
    response += " ";
    response += status_text;
    response += "\r\nContent-Type: ";
    response += content_type;
    response += "\r\nContent-Length: ";
    response += std::to_string(body.size());
    response += "\r\n\r\n";
    response += body;
    return response;
}

float parse_json_float(const std::string &body, const char *key, float fallback) {
    std::string needle = "\"" + std::string(key) + "\"";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) {
        return fallback;
    }
    pos = body.find(':', pos);
    if (pos == std::string::npos) {
        return fallback;
    }
    const char *start = body.c_str() + pos + 1;
    char *end = nullptr;
    float value = strtof(start, &end);
    if (end == start) {
        return fallback;
    }
    return value;
}

std::string handle_request_factory(KitronikPicoRobotics &robot, const HttpRequest &request) {
    if (request.method == "GET" && request.path == "/") {
        return http_response(200, "text/html", kIndexHtml);
    }

    if (request.method == "POST" && request.path == "/api/motors") {
        if (request.body.empty()) {
            return http_response(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }

        QuaternionInput q{
            parse_json_float(request.body, "w", 1.0f),
            parse_json_float(request.body, "x", 0.0f),
            parse_json_float(request.body, "y", 0.0f),
            parse_json_float(request.body, "z", 0.0f),
        };
        quaternion_to_movement(robot, q);
        return http_response(200, "application/json", "{\"status\":\"success\"}");
    }

    return http_response(404, "application/json", "{\"error\":\"Not Found\"}");
}

KitronikPicoRobotics *g_robot = nullptr;
HttpServer *g_server = nullptr;
bool g_server_started = false;
BleProvision *g_ble = nullptr;

void apply_wifi_config(const WifiConfig &config) {
    if (!wifi_config_is_valid(config)) {
        return;
    }
    wifi_config_save(config);
    if (g_ble) {
        g_ble->set_status("wifi:connecting");
    }

    int result = cyw43_arch_wifi_connect_timeout_ms(
        config.ssid, config.password, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (result != 0) {
        if (g_ble) {
            g_ble->set_status("wifi:failed");
        }
        return;
    }

    if (g_server && !g_server_started) {
        if (g_server->start(80)) {
            g_server_started = true;
        }
    }
    if (g_ble) {
        g_ble->set_status("wifi:connected");
    }
}

void ble_control_callback(float w, float x, float y, float z) {
    if (!g_robot) {
        return;
    }
    QuaternionInput q{w, x, y, z};
    quaternion_to_movement(*g_robot, q);
}
}  // namespace

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (!btstack_cyw43_init(cyw43_arch_async_context())) {
        cyw43_arch_deinit();
        return 1;
    }

    KitronikPicoRobotics robot;
    g_robot = &robot;

    HttpServer server([&robot](const HttpRequest &request) {
        return handle_request_factory(robot, request);
    });
    g_server = &server;

    BleProvision ble;
    ble.set_apply_callback(apply_wifi_config);
    ble.set_control_callback(ble_control_callback);
    ble.set_status("ble:ready");
    ble.init();
    g_ble = &ble;

    WifiConfig config{};
    if (wifi_config_load(&config)) {
        apply_wifi_config(config);
    }

    while (true) {
        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}
