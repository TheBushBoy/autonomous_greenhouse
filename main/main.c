#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "includes/dht22.h"
#include "includes/hw390.h"

#define PUMP_PIN GPIO_NUM_5
#define IRRIGATION_TIME_MS 3000
#define MOISTURE_THRESHOLD 20.0
#define READ_INTERVAL_MS 1000
#define MIN_IRRIGATION_INTERVAL_MS 6000

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
    
    gpio_set_direction(PUMP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP_PIN, 1);
    
    dht22_reading_t dh22_reading;
    hw390_reading_t hw390_reading;

    int lecture_count = 0;
    TickType_t last_irrigation_time = 0;
    
    while (1) {
        ESP_LOGI(TAG, "Read #%d", lecture_count);
        
        dht22_error_t result_dht22 = dht22_read(&dh22_reading);
        esp_err_t result_hw390 = hw390_read(&hw390_reading);
        
        if (result_dht22 == DHT22_OK) {
            ESP_LOGI(TAG, "Temperature : %4.1f °C", dh22_reading.temperature);
            ESP_LOGI(TAG, "Humidity    : %4.1f %%", dh22_reading.humidity);
        }

        if (result_hw390 == ESP_OK) {
            ESP_LOGI(TAG, "Moisture 1  : %4.1f %%", hw390_reading.moisture[0]);
            ESP_LOGI(TAG, "Moisture 2  : %4.1f %%", hw390_reading.moisture[1]);
            ESP_LOGI(TAG, "Moisture 3  : %4.1f %%", hw390_reading.moisture[2]);
        }
                  
        // TickType_t current_time = xTaskGetTickCount();
        // TickType_t time_since_last = current_time - last_irrigation_time;
        
        // if (moisture_result <= MOISTURE_THRESHOLD && 
        //     time_since_last >= pdMS_TO_TICKS(MIN_IRRIGATION_INTERVAL_MS)) {
            
        //     ESP_LOGI(TAG, "Starting irrigation...");
        //     gpio_set_level(PUMP_PIN, 0);
        //     vTaskDelay(pdMS_TO_TICKS(IRRIGATION_TIME_MS));
        //     gpio_set_level(PUMP_PIN, 1);
        //     ESP_LOGI(TAG, "Irrigation complete");
            
        //     last_irrigation_time = xTaskGetTickCount();
        // }
        
        lecture_count++;
        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}