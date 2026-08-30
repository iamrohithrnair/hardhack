#include "ble_manager.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "host/ble_att.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "ble_wireless";

#define MAX_BLE_CONNECTIONS 2

/*
 * Service   4fafc201-1fb5-459e-8fcc-c5c9c331914b
 * Telemetry beb5483e-36e1-4688-b7f5-ea07361b26a8  (READ | NOTIFY)
 * Control   beb5483e-36e1-4688-b7f5-ea07361b26a9  (WRITE)
 *
 * BLE_UUID128_INIT takes bytes least-significant-first, i.e. the printed UUID
 * reversed byte by byte.
 */
static const ble_uuid128_t s_svc_uuid =
    BLE_UUID128_INIT(0x4b, 0x91, 0x31, 0xc3, 0xc9, 0xc5, 0xcc, 0x8f,
                     0x9e, 0x45, 0xb5, 0x1f, 0x01, 0xc2, 0xaf, 0x4f);

static const ble_uuid128_t s_telemetry_uuid =
    BLE_UUID128_INIT(0xa8, 0x26, 0x1b, 0x36, 0x07, 0xea, 0xf5, 0xb7,
                     0x88, 0x46, 0xe1, 0x36, 0x3e, 0x48, 0xb5, 0xbe);

static const ble_uuid128_t s_control_uuid =
    BLE_UUID128_INIT(0xa9, 0x26, 0x1b, 0x36, 0x07, 0xea, 0xf5, 0xb7,
                     0x88, 0x46, 0xe1, 0x36, 0x3e, 0x48, 0xb5, 0xbe);

typedef struct {
    uint16_t conn_handle;
    bool notify_enabled;
} ble_client_t;

static ble_client_t s_clients[MAX_BLE_CONNECTIONS];
static uint16_t s_telemetry_val_handle = 0;
static uint8_t s_own_addr_type = 0;
static bool s_ble_ready = false;
static ble_command_cb_t s_command_cb = NULL;
static SemaphoreHandle_t s_client_mutex = NULL;

static char s_last_telemetry[512] =
    "{\"rpm\":2910,\"f0\":48.5,\"rms\":0.082,\"kurt\":2.94,\"iso\":0.16,\"score\":98,\"state\":1}\n";

static void ble_advertise(void);

static int gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR: {
        if (ble_uuid_cmp(ctxt->chr->uuid, &s_telemetry_uuid.u) == 0) {
            int rc = os_mbuf_append(ctxt->om, s_last_telemetry, strlen(s_last_telemetry));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return BLE_ATT_ERR_UNLIKELY;
    }

    case BLE_GATT_ACCESS_OP_WRITE_CHR: {
        if (ble_uuid_cmp(ctxt->chr->uuid, &s_control_uuid.u) != 0) {
            return BLE_ATT_ERR_UNLIKELY;
        }
        char cmd[64] = {0};
        uint16_t len = 0;
        int rc = ble_hs_mbuf_to_flat(ctxt->om, cmd, sizeof(cmd) - 1, &len);
        if (rc != 0) {
            return BLE_ATT_ERR_UNLIKELY;
        }
        cmd[len] = '\0';
        ESP_LOGI(TAG, "BLE control command: %s", cmd);
        if (s_command_cb) {
            s_command_cb(cmd);
        }
        return 0;
    }

    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

static const struct ble_gatt_svc_def s_gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &s_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &s_telemetry_uuid.u,
                .access_cb = gatt_access_cb,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &s_telemetry_val_handle,
            },
            {
                .uuid = &s_control_uuid.u,
                .access_cb = gatt_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                0,
            },
        },
    },
    {
        0,
    },
};

static void client_add(uint16_t conn_handle) {
    xSemaphoreTake(s_client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle == BLE_HS_CONN_HANDLE_NONE) {
            s_clients[i].conn_handle = conn_handle;
            s_clients[i].notify_enabled = false;
            break;
        }
    }
    xSemaphoreGive(s_client_mutex);
}

static void client_remove(uint16_t conn_handle) {
    xSemaphoreTake(s_client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle == conn_handle) {
            s_clients[i].conn_handle = BLE_HS_CONN_HANDLE_NONE;
            s_clients[i].notify_enabled = false;
        }
    }
    xSemaphoreGive(s_client_mutex);
}

static void client_set_notify(uint16_t conn_handle, bool enabled) {
    xSemaphoreTake(s_client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle == conn_handle) {
            s_clients[i].notify_enabled = enabled;
        }
    }
    xSemaphoreGive(s_client_mutex);
}

static bool has_free_slot(void) {
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle == BLE_HS_CONN_HANDLE_NONE) return true;
    }
    return false;
}

static int gap_event_cb(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            ESP_LOGI(TAG, "BLE client connected; conn_handle=%d", event->connect.conn_handle);
            client_add(event->connect.conn_handle);
        } else {
            ESP_LOGW(TAG, "BLE connection failed; status=%d", event->connect.status);
        }
        // Keep advertising so additional clients (or a retry) can still find us.
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE client disconnected; reason=%d", event->disconnect.reason);
        client_remove(event->disconnect.conn.conn_handle);
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.attr_handle == s_telemetry_val_handle) {
            ESP_LOGI(TAG, "Telemetry notifications %s for conn_handle=%d",
                     event->subscribe.cur_notify ? "ENABLED" : "disabled",
                     event->subscribe.conn_handle);
            client_set_notify(event->subscribe.conn_handle, event->subscribe.cur_notify != 0);
        }
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "BLE MTU negotiated: %d bytes (conn_handle=%d)",
                 event->mtu.value, event->mtu.conn_handle);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ble_advertise();
        return 0;

    default:
        return 0;
    }
}

static void ble_advertise(void) {
    if (!has_free_slot()) {
        return;
    }

    struct ble_hs_adv_fields fields;
    struct ble_hs_adv_fields rsp_fields;
    struct ble_gap_adv_params adv_params;
    int rc;

    /*
     * A legacy advertising payload is capped at 31 bytes. Flags (3) plus one
     * 128-bit service UUID (18) leaves no room for the 15-character name, so
     * the name is carried in the scan response instead.
     */
    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128 = (ble_uuid128_t *)&s_svc_uuid;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields failed; rc=%d", rc);
        return;
    }

    memset(&rsp_fields, 0, sizeof rsp_fields);
    rsp_fields.name = (uint8_t *)BLE_DEVICE_NAME;
    rsp_fields.name_len = strlen(BLE_DEVICE_NAME);
    rsp_fields.name_is_complete = 1;
    rsp_fields.tx_pwr_lvl_is_present = 1;
    rsp_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_rsp_set_fields failed; rc=%d", rc);
        return;
    }

    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, gap_event_cb, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "ble_gap_adv_start failed; rc=%d", rc);
    }
}

static void on_host_sync(void) {
    int rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_util_ensure_addr failed; rc=%d", rc);
        return;
    }

    rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed; rc=%d", rc);
        return;
    }

    uint8_t addr[6] = {0};
    ble_hs_id_copy_addr(s_own_addr_type, addr, NULL);
    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "  BLE Peripheral Ready: '%s'", BLE_DEVICE_NAME);
    ESP_LOGI(TAG, "  Address: %02x:%02x:%02x:%02x:%02x:%02x",
             addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    ESP_LOGI(TAG, "  Service: 4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    ESP_LOGI(TAG, "=================================================");

    s_ble_ready = true;
    ble_advertise();
}

static void on_host_reset(int reason) {
    ESP_LOGW(TAG, "NimBLE host reset; reason=%d", reason);
    s_ble_ready = false;
}

static void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_manager_init(ble_command_cb_t on_command) {
    s_command_cb = on_command;
    s_client_mutex = xSemaphoreCreateMutex();
    if (s_client_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        s_clients[i].conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_clients[i].notify_enabled = false;
    }

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ble_hs_cfg.sync_cb = on_host_sync;
    ble_hs_cfg.reset_cb = on_host_reset;
    ble_hs_cfg.gatts_register_cb = NULL;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 0;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 0;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(s_gatt_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed; rc=%d", rc);
        return ESP_FAIL;
    }

    rc = ble_gatts_add_svcs(s_gatt_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed; rc=%d", rc);
        return ESP_FAIL;
    }

    rc = ble_svc_gap_device_name_set(BLE_DEVICE_NAME);
    if (rc != 0) {
        ESP_LOGW(TAG, "ble_svc_gap_device_name_set failed; rc=%d", rc);
    }

    // Ask for a large ATT MTU so a whole telemetry frame fits in one notification.
    rc = ble_att_set_preferred_mtu(256);
    if (rc != 0) {
        ESP_LOGW(TAG, "ble_att_set_preferred_mtu failed; rc=%d", rc);
    }

    nimble_port_freertos_init(ble_host_task);
    return ESP_OK;
}

void ble_manager_notify_telemetry(const char *json_str) {
    if (!json_str || !s_ble_ready || s_telemetry_val_handle == 0) return;

    size_t len = strnlen(json_str, sizeof(s_last_telemetry) - 2);
    memcpy(s_last_telemetry, json_str, len);
    // Records are newline delimited so a client can reassemble frames that were
    // split across several notifications on a small-MTU link.
    if (len == 0 || s_last_telemetry[len - 1] != '\n') {
        s_last_telemetry[len++] = '\n';
    }
    s_last_telemetry[len] = '\0';

    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        uint16_t conn_handle;
        bool notify_enabled;

        xSemaphoreTake(s_client_mutex, portMAX_DELAY);
        conn_handle = s_clients[i].conn_handle;
        notify_enabled = s_clients[i].notify_enabled;
        xSemaphoreGive(s_client_mutex);

        if (conn_handle == BLE_HS_CONN_HANDLE_NONE || !notify_enabled) continue;

        uint16_t mtu = ble_att_mtu(conn_handle);
        size_t chunk_max = (mtu > 3) ? (size_t)(mtu - 3) : 20;

        for (size_t offset = 0; offset < len; offset += chunk_max) {
            size_t chunk = len - offset;
            if (chunk > chunk_max) chunk = chunk_max;

            struct os_mbuf *om = ble_hs_mbuf_from_flat(s_last_telemetry + offset, chunk);
            if (om == NULL) break;

            // ble_gatts_notify_custom takes ownership of the mbuf in all cases.
            int rc = ble_gatts_notify_custom(conn_handle, s_telemetry_val_handle, om);
            if (rc != 0) {
                ESP_LOGD(TAG, "notify failed conn_handle=%d rc=%d", conn_handle, rc);
                break;
            }
        }
    }
}

bool ble_manager_has_subscribers(void) {
    bool found = false;
    if (!s_client_mutex) return false;
    xSemaphoreTake(s_client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle != BLE_HS_CONN_HANDLE_NONE && s_clients[i].notify_enabled) {
            found = true;
            break;
        }
    }
    xSemaphoreGive(s_client_mutex);
    return found;
}

uint8_t ble_manager_client_count(void) {
    uint8_t count = 0;
    if (!s_client_mutex) return 0;
    xSemaphoreTake(s_client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_BLE_CONNECTIONS; i++) {
        if (s_clients[i].conn_handle != BLE_HS_CONN_HANDLE_NONE) count++;
    }
    xSemaphoreGive(s_client_mutex);
    return count;
}
