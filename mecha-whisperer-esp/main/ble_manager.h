#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Advertised BLE name. Must match the Web Bluetooth namePrefix filter used by the dashboard.
#define BLE_DEVICE_NAME "MECHA-WHISPERER"

/**
 * Callback invoked when a BLE client writes an ASCII command to the control
 * characteristic (e.g. "CALIB", "FAULT:unbalance"). The string is NUL terminated.
 */
typedef void (*ble_command_cb_t)(const char *cmd);

// Start the NimBLE peripheral (GATT server + advertising). Safe to call once.
esp_err_t ble_manager_init(ble_command_cb_t on_command);

// Push a telemetry JSON payload to every subscribed BLE client.
void ble_manager_notify_telemetry(const char *json_str);

// True when at least one client has enabled notifications on the telemetry characteristic.
bool ble_manager_has_subscribers(void);

// Number of currently connected BLE clients.
uint8_t ble_manager_client_count(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_MANAGER_H
