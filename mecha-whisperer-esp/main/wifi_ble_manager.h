#ifndef WIFI_BLE_MANAGER_H
#define WIFI_BLE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Wi-Fi SoftAP and embedded WebSocket server
esp_err_t wifi_ble_manager_init(void);

// Broadcast telemetry JSON string to all wireless Wi-Fi and BLE clients
void wifi_ble_broadcast_telemetry(const char* json_str);

// Get connection status
bool wifi_ble_has_clients(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_BLE_MANAGER_H
