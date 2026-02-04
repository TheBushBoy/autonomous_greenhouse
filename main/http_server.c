/**
* Wifi station and http server to get the values from the sensors
*  Wifi station example: https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32/api-guides/wifi.html
*  Thanks to Shawn Hymel on YouTube:
*   - https://www.youtube.com/watch?v=j1ve8mYjUoU for wifi station
 */

#include "includes/http_server.h"

static const char* TAG = "HTTP_SERVER";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    
    switch(event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Trying to reconnect to wifi");
            esp_wifi_connect();
            break;
            
        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Adress IP: " IPSTR, IP2STR(&event->ip_info.ip));
            break;
    }
}

void wifi_init_sta(void) {
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_STA_START,
                                               &wifi_event_handler,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_STA_DISCONNECTED,
                                               &wifi_event_handler,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler,
                                               NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wifi initialized");
}