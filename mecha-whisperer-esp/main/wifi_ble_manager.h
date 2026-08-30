#ifndef WIFI_BLE_MANAGER_H
#define WIFI_BLE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback invoked when a wireless client sends a control payload, either as a
 * WebSocket text frame or as the body of POST /api/control. The payload is the
 * raw NUL-terminated string, e.g. {"command":"SET_FAULT","param":"bearing"}.
 */
typedef void (*wifi_command_cb_t)(const char *payload);

// Initialize Wi-Fi SoftAP, the embedded HTTP/WebSocket server and the BLE peripheral.
esp_err_t wifi_ble_manager_init(wifi_command_cb_t on_command);

// Broadcast telemetry JSON string to all wireless Wi-Fi and BLE clients
void wifi_ble_broadcast_telemetry(const char* json_str);

// Get connection status
bool wifi_ble_has_clients(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_BLE_MANAGER_H
