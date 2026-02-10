/**
* Wifi station and http server to get the values from the sensors
*  Wifi station example: https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32/api-guides/wifi.html
*  Thanks to Shawn Hymel on YouTube:
*   - https://www.youtube.com/watch?v=j1ve8mYjUoU for wifi station
*   - https://www.youtube.com/watch?v=ZQatV_Fi0Vc for http server
*/

#include "includes/http_server.h"
#include "includes/logs.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"

#define OTA_BUF_SIZE 1024
#define LED_OTA_PIN GPIO_NUM_2

static const char* TAG = "HTTP_SERVER";

static sensor_data_t sensor_data = {0};

/* GET /api/sensors */
static esp_err_t sensors_get_handler(httpd_req_t* req) {
    cJSON* root = cJSON_CreateObject();
    
    cJSON_AddNumberToObject(root, "temperature", sensor_data.temperature);
    cJSON_AddNumberToObject(root, "humidity_air", sensor_data.humidity_air);
    
    cJSON* soil_moisture = cJSON_CreateArray();
    cJSON_AddItemToArray(soil_moisture, cJSON_CreateNumber(sensor_data.soil_moisture_1));
    cJSON_AddItemToArray(soil_moisture, cJSON_CreateNumber(sensor_data.soil_moisture_2));
    cJSON_AddItemToArray(soil_moisture, cJSON_CreateNumber(sensor_data.soil_moisture_3));
    cJSON_AddItemToObject(root, "soil_moisture", soil_moisture);
    
    char* json_string = cJSON_Print(root);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, json_string);
    
    free(json_string);
    cJSON_Delete(root);
    
    return ESP_OK;
}

/* GET /api/logs */
static esp_err_t logs_get_handler(httpd_req_t* req) {
    size_t log_count = logs_get_count();
    
    if (log_count == 0) {
        httpd_resp_set_type(req, "text/plain; charset=utf-8");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_sendstr(req, "No logs available\n");
        return ESP_OK;
    }
    
    char* log_buffer = malloc(log_count + 1);
    if (!log_buffer) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }
    
    size_t copied = logs_get_content(log_buffer, log_count + 1);
    
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    if (copied > 0) {
        httpd_resp_sendstr(req, log_buffer);
    } else {
        httpd_resp_sendstr(req, "Error reading logs\n");
    }
    
    free(log_buffer);
    return ESP_OK;
}

/* POST /ota */
esp_err_t ota_upload_handler(httpd_req_t *req) {
    esp_ota_handle_t ota_handle = 0;
    esp_err_t err;
    
    gpio_set_direction(LED_OTA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_OTA_PIN, 1);

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "No OTA partition found");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition available");
        return ESP_FAIL;
    }
    
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA begin failed");
        return ESP_FAIL;
    }
    
    char buf[OTA_BUF_SIZE]; // Temporary buffer in RAM before writing to flash
    int received;           // Number of bytes received in current iteration

    while ((received = httpd_req_recv(req, buf, sizeof(buf))) > 0) {
        err = esp_ota_write(ota_handle, (const void *)buf, received);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed: %s", esp_err_to_name(err));
            esp_ota_abort(ota_handle);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA write failed");
            return ESP_FAIL;
        }
    }
    
    if (received < 0) {
        ESP_LOGE(TAG, "HTTP receive failed");
        esp_ota_abort(ota_handle);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Reception failed");
        return ESP_FAIL;
    }
        
    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA validation failed");
        return ESP_FAIL;
    }
    
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set boot partition");
        return ESP_FAIL;
    }
    
    httpd_resp_sendstr(req, "OTA update successful, rebooting...");
    
    // Wait for response before rebooting, blinking led
    for (int i = 0; i < 10; i++) {
        gpio_set_level(LED_OTA_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_OTA_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    gpio_set_level(LED_OTA_PIN, 0);
    esp_restart();
    
    return ESP_OK;
}

httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "HTTP server on port: '%d'", config.server_port);

    const httpd_uri_t uri_get_sensors = {
        .uri       = "/api/sensors",
        .method    = HTTP_GET,
        .handler   = sensors_get_handler,
        .user_ctx  = NULL
    };

    const httpd_uri_t uri_get_logs = {
        .uri       = "/api/logs",
        .method    = HTTP_GET,
        .handler   = logs_get_handler,
        .user_ctx  = NULL
    };

    const httpd_uri_t uri_ota = {
        .uri       = "/ota",
        .method    = HTTP_POST,
        .handler   = ota_upload_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get_sensors);
        httpd_register_uri_handler(server, &uri_get_logs);
        httpd_register_uri_handler(server, &uri_ota);
        return server;
    }

    ESP_LOGI(TAG, "Error while starting the web server");
    return NULL;
}

void stop_webserver(httpd_handle_t server) {
    httpd_stop(server);
}

void update_sensor_data_http(float temp, float hum_air, float soil1, float soil2, float soil3) {
    sensor_data.temperature = temp;
    sensor_data.humidity_air = hum_air;
    sensor_data.soil_moisture_1 = soil1;
    sensor_data.soil_moisture_2 = soil2;
    sensor_data.soil_moisture_3 = soil3;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    
    switch(event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Trying to reconnect to wifi");
            esp_wifi_connect();
            if (*server) {
                stop_webserver(*server);
                *server = NULL;
            }
            break;
            
        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Adress IP: " IPSTR, IP2STR(&event->ip_info.ip));
            if (*server == NULL) {
                *server = start_webserver();
            }
            break;
    }
}

void wifi_init_sta(httpd_handle_t* server) {
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                            WIFI_EVENT_STA_START,
                                            &wifi_event_handler,
                                            server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                            WIFI_EVENT_STA_DISCONNECTED,
                                            &wifi_event_handler,
                                            server));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                            IP_EVENT_STA_GOT_IP,
                                            &wifi_event_handler,
                                            server));

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

    ESP_LOGI(TAG, "Http server initialized");
    ESP_LOGI(TAG, "GET http://<IP_ESP>/api/sensors");
    ESP_LOGI(TAG, "GET http://<IP_ESP>/api/logs");
}