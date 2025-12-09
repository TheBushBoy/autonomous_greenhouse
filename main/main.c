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
    
    dht22_reading_t reading;
    int lecture_count = 0;
    TickType_t last_irrigation_time = 0;
    
    while (1) {
        ESP_LOGI(TAG, "Read #%d", lecture_count);
        
        dht22_error_t result = dht22_read(&reading);
        float moisture_result = hw390_read_moisture_percent();
        
        if (result == DHT22_OK && moisture_result >= 0) {
            ESP_LOGI(TAG, "Temperature : %4.1f °C", reading.temperature);
            ESP_LOGI(TAG, "Humidity    : %4.1f %%", reading.humidity);
            ESP_LOGI(TAG, "Moisture    : %4.1f %%", moisture_result);
                   
            TickType_t current_time = xTaskGetTickCount();
            TickType_t time_since_last = current_time - last_irrigation_time;
            
            if (moisture_result <= MOISTURE_THRESHOLD && 
                time_since_last >= pdMS_TO_TICKS(MIN_IRRIGATION_INTERVAL_MS)) {
                
                ESP_LOGI(TAG, "Starting irrigation...");
                gpio_set_level(PUMP_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(IRRIGATION_TIME_MS));
                gpio_set_level(PUMP_PIN, 1);
                ESP_LOGI(TAG, "Irrigation complete");
                
                last_irrigation_time = xTaskGetTickCount();
            }
        }
        
        lecture_count++;
        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}