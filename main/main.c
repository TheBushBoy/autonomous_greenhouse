#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "http_server.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "includes/dht22.h"
#include "includes/hw390.h"
#include "includes/http_server.h"

#define READ_INTERVAL_MS 5000

const char* TAG = "MAIN";

void sensor_task(void* pvParameters) {
    dht22_reading_t dh22_reading;
    hw390_reading_t hw390_reading;
    
    while (1) {
        dht22_error_t result_dht22 = dht22_read(&dh22_reading);
        esp_err_t result_hw390 = hw390_read(&hw390_reading);
        
        if (result_dht22 == DHT22_OK) {
            ESP_LOGI(TAG, "Temperature : %4.1f °C", dh22_reading.temperature);
            ESP_LOGI(TAG, "Humidity    : %4.1f %%", dh22_reading.humidity);
        } else {
            ESP_LOGW(TAG, "Temperature/humidity sensor DHT22 reading error");
        }
        
        if (result_hw390 == ESP_OK) {
            ESP_LOGI(TAG, "Moisture 1  : %4.1f %%", hw390_reading.moisture[0]);
            ESP_LOGI(TAG, "Moisture 2  : %4.1f %%", hw390_reading.moisture[1]);
            ESP_LOGI(TAG, "Moisture 3  : %4.1f %%", hw390_reading.moisture[2]);
        } else {
            ESP_LOGW(TAG, "Moisture sensor HW390 reading error");
        }
        
        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}

void app_main(void) {
    // NVS init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    if (dht22_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error init DHT22");
        return;
    }
    
    if (hw390_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error init HW390");
        return;
    }
    
    wifi_init_sta();

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 1, NULL);
}