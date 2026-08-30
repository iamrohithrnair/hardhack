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
static char s_latest_telemetry[256] = "{\"rpm\":2910,\"f0\":48.5,\"rms\":0.082,\"kurt\":2.94,\"iso\":0.16,\"score\":98,\"state\":1}";

// Embedded Standalone Mobile/Desktop Dashboard HTML
static const char INDEX_HTML[] = 
"<!DOCTYPE html>"
"<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>MECHA-WHISPERER | Machine Stethoscope</title>"
"<style>"
"body{background:#0D1017;color:#F3F4F6;font-family:system-ui,-apple-system,sans-serif;margin:0;padding:16px;display:flex;flex-direction:column;align-items:center;}"
".container{max-width:480px;width:100%;display:flex;flex-direction:column;gap:14px;}"
".card{background:#161B26;border:1px solid #283042;border-radius:20px;padding:16px;box-shadow:0 8px 24px rgba(0,0,0,0.3);}"
".header{display:flex;justify-content:space-between;align-items:center;}"
".badge{background:#00F0FF22;color:#00F0FF;padding:4px 10px;border-radius:20px;font-size:11px;font-weight:bold;}"
".score-val{font-size:44px;font-weight:900;color:#00F0FF;text-shadow:0 0 20px rgba(0,240,255,0.4);}"
".scope{width:100%;height:140px;background:#0A0D14;border-radius:14px;display:block;margin-top:8px;}"
".grid{display:grid;grid-template-cols:1fr 1fr;gap:10px;}"
".metric-box{background:#1F2636;padding:12px;border-radius:12px;display:flex;flex-direction:column;}"
".metric-title{font-size:10px;color:#8B98AD;font-weight:bold;text-transform:uppercase;}"
".metric-val{font-size:20px;font-weight:bold;color:#FFF;margin-top:2px;font-family:monospace;}"
".btn{background:#00F0FF;color:#000;border:none;padding:12px;border-radius:12px;font-weight:bold;cursor:pointer;width:100%;font-size:14px;}"
"</style></head><body>"
"<div class='container'>"
"<div class='card header'>"
"  <div><strong style='font-size:16px;'>MECHA-WHISPERER</strong><div style='font-size:11px;color:#8B98AD;'>Wi-Fi Stethoscope Live Stream</div></div>"
"  <div class='badge' id='status-badge'>LIVE 250Hz</div>"
"</div>"
"<div class='card' style='text-align:center;'>"
"  <div style='font-size:11px;color:#8B98AD;font-weight:bold;'>MACHINE HEALTH SCORE</div>"
"  <div class='score-val' id='score'>98%</div>"
"  <div id='diag' style='font-size:13px;font-weight:bold;color:#FFF;'>NOMINAL HARMONIC</div>"
"</div>"
"<div class='card'>"
"  <div style='font-size:11px;color:#8B98AD;font-weight:bold;'>REAL-TIME VIBRATION TRANSDUCER</div>"
"  <canvas id='scope' class='scope' width='440' height='140'></canvas>"
"</div>"
"<div class='grid'>"
"  <div class='metric-box'><span class='metric-title'>RMS ACCEL</span><span class='metric-val' id='rms'>0.082g</span></div>"
"  <div class='metric-box'><span class='metric-title'>ESTIMATED RPM</span><span class='metric-val' id='rpm'>2910</span></div>"
"  <div class='metric-box'><span class='metric-title'>KURTOSIS</span><span class='metric-val' id='kurt'>2.94</span></div>"
"  <div class='metric-box'><span class='metric-title'>ISO-10816</span><span class='metric-val' id='iso' style='color:#00FF88;'>CLASS A</span></div>"
"</div>"
"</div>"
"<script>"
"const cvs=document.getElementById('scope'),ctx=cvs.getContext('2d');"
"let history=new Array(100).fill(0),phase=0;"
"function draw(){"
"  ctx.fillStyle='#0A0D14';ctx.fillRect(0,0,cvs.width,cvs.height);"
"  ctx.strokeStyle='#00F0FF';ctx.lineWidth=2;ctx.beginPath();"
"  let slice=cvs.width/(history.length-1);"
"  for(let i=0;i<history.length;i++){"
"    let y=cvs.height/2-history[i]*40;"
"    if(i===0)ctx.moveTo(0,y);else ctx.lineTo(i*slice,y);"
"  }"
"  ctx.stroke();"
"  requestAnimationFrame(draw);"
"}"
"draw();"
"setInterval(async()=>{"
"  try{"
"    let r=await fetch('/api/telemetry');"
"    if(r.ok){"
"      let d=await r.json();"
"      document.getElementById('score').innerText=d.score+'%';"
"      document.getElementById('rms').innerText=d.rms.toFixed(3)+'g';"
"      document.getElementById('rpm').innerText=d.rpm;"
"      document.getElementById('kurt').innerText=d.kurt.toFixed(2);"
"      document.getElementById('diag').innerText=d.score>70?'NOMINAL HARMONIC':d.score>40?'WARNING: ELEVATED VIBRATION':'CRITICAL UNBALANCE';"
"      phase+=0.2;"
"      let s=(d.rms*3.0)*Math.sin(phase)+(Math.random()-0.5)*0.1;"
"      history.shift();history.push(s);"
"    }"
"  }catch(e){}"
"},100);"
"</script>"
"</body></html>";

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t options_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake completed");
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
            free(buf);
        }
    }
    return ESP_OK;
}

static esp_err_t telemetry_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
    return httpd_resp_send(req, s_latest_telemetry, HTTPD_RESP_USE_STRLEN);
}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP/WebSocket server on port %d...", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Root UI
        httpd_uri_t root_uri = {
            .uri        = "/",
            .method     = HTTP_GET,
            .handler    = root_get_handler,
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        // Options pre-flight
        httpd_uri_t options_uri = {
            .uri        = "/api/telemetry",
            .method     = HTTP_OPTIONS,
            .handler    = options_handler,
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &options_uri);

        // REST /api/telemetry
        httpd_uri_t telemetry_uri = {
            .uri        = "/api/telemetry",
            .method     = HTTP_GET,
            .handler    = telemetry_get_handler,
            .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &telemetry_uri);

        // WebSocket /ws
        httpd_uri_t ws_uri = {
            .uri        = "/ws",
            .method     = HTTP_GET,
            .handler    = ws_handler,
            .user_ctx   = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);

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
    ESP_LOGI(TAG, "  Direct Web Dashboard at: http://192.168.4.1/");
    ESP_LOGI(TAG, "  REST Telemetry Stream at: http://192.168.4.1/api/telemetry");
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
