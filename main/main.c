#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "includes/dht22.h"
#include "includes/hw390.h"
#include "includes/http_server.h"

#define READ_INTERVAL_MS 5000

#define PUMP_0_PIN GPIO_NUM_23
#define PUMP_1_PIN GPIO_NUM_19
#define PUMP_2_PIN GPIO_NUM_18

#define MOISTURE_THRESHOLD 25.0f
#define PUMP_ON_TIME_MS 200
#define SOAK_DELAY_MS 60000
#define TASK_PERIOD_MS 3600000

const char* TAG = "MAIN";

static const gpio_num_t pump_pins[3] = {
    PUMP_0_PIN,
    PUMP_1_PIN,
    PUMP_2_PIN
};

static inline void pump_set(gpio_num_t pin, bool on) {
    gpio_set_level(pin, on ? 0 : 1);
}


void sensor_task(void* pvParameters) {
    dht22_reading_t dh22_reading;
    hw390_reading_t hw390_reading;
    
    while (1) {
        dht22_error_t result_dht22 = dht22_read(&dh22_reading);
        esp_err_t result_hw390 = hw390_read(&hw390_reading);
        
        // Afficher les lectures
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

        float temperature = (result_dht22 == DHT22_OK) ? dh22_reading.temperature : 0.0f;
        float humidity_air = (result_dht22 == DHT22_OK) ? dh22_reading.humidity : 0.0f;
        float soil_1 = (result_hw390 == ESP_OK) ? hw390_reading.moisture[0] : 0.0f;
        float soil_2 = (result_hw390 == ESP_OK) ? hw390_reading.moisture[1] : 0.0f;
        float soil_3 = (result_hw390 == ESP_OK) ? hw390_reading.moisture[2] : 0.0f;
        
        update_sensor_data_http(temperature, humidity_air, soil_1, soil_2, soil_3);
        
        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}

void irrigation_task(void *pvParameters) {
    hw390_reading_t hw390_reading;

    for (int i = 0; i < 3; i++) {
        gpio_set_direction(pump_pins[i], GPIO_MODE_OUTPUT);
        pump_set(pump_pins[i], false);
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(TASK_PERIOD_MS));

        if (hw390_read(&hw390_reading) != ESP_OK)
            continue;

        for (int i = 0; i < 3; i++) {

            if (hw390_reading.moisture[i] < 0.0f)
                continue;

            if (hw390_reading.moisture[i] >= MOISTURE_THRESHOLD)
                continue;

            ESP_LOGI(TAG, "Irrigation zone %d", i + 1);

            pump_set(pump_pins[i], true);
            vTaskDelay(pdMS_TO_TICKS(PUMP_ON_TIME_MS));
            pump_set(pump_pins[i], false);

            vTaskDelay(pdMS_TO_TICKS(SOAK_DELAY_MS));
        }
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
    
    static httpd_handle_t server = NULL;
    wifi_init_sta(&server);
    
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 1, NULL);
}