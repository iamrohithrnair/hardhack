#include "wifi_ble_manager.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

static const char *TAG = "wifi_wireless";

#define WIFI_AP_SSID      "MECHA-WHISPERER"
#define WIFI_AP_PASS      "" // Open network for instant pairing
#define MAX_STA_CONN      4
#define MAX_WS_CLIENTS    4

static httpd_handle_t s_http_server = NULL;
static int s_ws_fds[MAX_WS_CLIENTS] = {-1, -1, -1, -1};
static char s_latest_telemetry[256] = "{\"status\":\"init\"}";

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake done, new client connected");
        int fd = httpd_req_to_sockfd(req);
        for (int i = 0; i < MAX_WS_CLIENTS; i++) {
            if (s_ws_fds[i] == -1 || s_ws_fds[i] == fd) {
                s_ws_fds[i] = fd;
                break;
            }
        }
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) return ret;

    if (ws_pkt.len) {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf) {
            ws_pkt.payload = buf;
            ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "WS Received: %s", (char*)buf);
            }
            free(buf);
        }
    }
    return ESP_OK;
}

static esp_err_t telemetry_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, s_latest_telemetry, HTTPD_RESP_USE_STRLEN);
}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;

    ESP_LOGI(TAG, "Starting HTTP/WebSocket server on port %d...", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register WebSocket /ws
        httpd_uri_t ws_uri = {
            .uri        = "/ws",
            .method     = HTTP_GET,
            .handler    = ws_handler,
            .user_ctx   = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);

        // Register REST /api/telemetry
        httpd_uri_t telemetry_uri = {
            .uri        = "/api/telemetry",
            .method     = HTTP_GET,
            .handler    = telemetry_get_handler,
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &telemetry_uri);

        return server;
    }

    ESP_LOGE(TAG, "Error starting HTTP server!");
    return NULL;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Wireless client joined: AID=%d", event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Wireless client left: AID=%d", event->aid);
    }
}

esp_err_t wifi_ble_manager_init(void) {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Netif & Events
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // 3. Configure Wi-Fi SoftAP
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = 1,
            .password = WIFI_AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_OPEN
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "  Wi-Fi SoftAP Started: SSID='%s' IP=192.168.4.1", WIFI_AP_SSID);
    ESP_LOGI(TAG, "  WebSocket Live Stream on: ws://192.168.4.1/ws");
    ESP_LOGI(TAG, "=================================================");

    // 4. Start HTTP / WebSocket Server
    s_http_server = start_webserver();

    return ESP_OK;
}

void wifi_ble_broadcast_telemetry(const char* json_str) {
    if (!json_str || !s_http_server) return;

    strncpy(s_latest_telemetry, json_str, sizeof(s_latest_telemetry) - 1);
    s_latest_telemetry[sizeof(s_latest_telemetry) - 1] = '\0';

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)json_str;
    ws_pkt.len = strlen(json_str);

    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (s_ws_fds[i] != -1) {
            esp_err_t ret = httpd_ws_send_frame_async(s_http_server, s_ws_fds[i], &ws_pkt);
            if (ret != ESP_OK) {
                s_ws_fds[i] = -1; // Client disconnected
            }
        }
    }
}

bool wifi_ble_has_clients(void) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (s_ws_fds[i] != -1) return true;
    }
    return false;
}
