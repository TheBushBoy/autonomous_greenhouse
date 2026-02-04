#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/event_groups.h"
#include <cJSON.h>

#include "esp_http_server.h"

typedef struct {
    float temperature;
    float humidity_air;
    float soil_moisture_1;
    float soil_moisture_2;
    float soil_moisture_3;
} sensor_data_t;

httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
void update_sensor_data_http(float temp, float hum_air, float soil1, float soil2, float soil3);
void wifi_init_sta(httpd_handle_t* server);

#endif