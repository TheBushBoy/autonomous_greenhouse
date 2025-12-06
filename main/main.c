#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "esp_log.h"

#include "includes/dht22.h"
#include "includes/hw390.h"

const char* TAG = "MAIN";

void app_main(void) {
    if (dht22_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error init DHT22");
        return;
    }

    if (hw390_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error init HW390");
        return;
    }
        
    dht22_reading_t reading;
    int lecture_count = 0;
    
    while (1) {
        ESP_LOGI(TAG, "Read #%d", lecture_count);
        
        dht22_error_t result = dht22_read(&reading);
        
        if (result == DHT22_OK) {
            ESP_LOGI(TAG, "Temperature : %4.1f °C", reading.temperature);
            ESP_LOGI(TAG, "Humidity    : %4.1f %%", reading.humidity);
            ESP_LOGI(TAG, "Moisture    : %4.1f %%", hw390_read_moisture_percent());
        } else {
            ESP_LOGE(TAG, "Reading error (code: %d)", result);
        }

        lecture_count++;
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}