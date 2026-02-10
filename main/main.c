#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "includes/dht22.h"
#include "includes/hw390.h"
#include "includes/http_server.h"
#include "includes/logs.h"
#include "includes/sensors.h"

#define READ_INTERVAL_MS 5000

#define PUMP_0_PIN GPIO_NUM_23
#define PUMP_1_PIN GPIO_NUM_19
#define PUMP_2_PIN GPIO_NUM_18

#define FAN_0_PIN GPIO_NUM_16
#define FAN_1_PIN GPIO_NUM_17

#define TEMP_THRESHOLD 30.0f // °C
#define FAN_TASK_PERIOD_MS 2000 // 2s

#define MOISTURE_THRESHOLD 25.0f
#define PUMP_ON_TIME_MS 200
#define SOAK_DELAY_MS 60000 // 1 minute
#define IRRIGATION_TASK_PERIOD_MS 3600000 // 1 hour

const char* TAG = "MAIN";

static const gpio_num_t pump_pins[3] = {
    PUMP_0_PIN,
    PUMP_1_PIN,
    PUMP_2_PIN
};


void sensor_task(void* pvParameters) {
    dht22_reading_t dht22_reading;
    hw390_reading_t hw390_reading;

    while (1) {
        dht22_error_t result_dht22 = dht22_read(&dht22_reading);
        esp_err_t result_hw390 = hw390_read(&hw390_reading);

        if (result_dht22 == DHT22_OK) {
            ESP_LOGD(TAG, "Temperature : %4.1f °C", dht22_reading.temperature);
            ESP_LOGD(TAG, "Humidity    : %4.1f %%", dht22_reading.humidity);
        } else {
            ESP_LOGW(TAG, "Temperature/humidity sensor DHT22 reading error");
        }
        
        if (result_hw390 == ESP_OK) {
            ESP_LOGD(TAG, "Moisture 1  : %4.1f %%", hw390_reading.moisture[0]);
            ESP_LOGD(TAG, "Moisture 2  : %4.1f %%", hw390_reading.moisture[1]);
            ESP_LOGD(TAG, "Moisture 3  : %4.1f %%", hw390_reading.moisture[2]);
        } else {
            ESP_LOGW(TAG, "Moisture sensor HW390 reading error");
        }

        if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            g_sensor_data.temperature = (result_dht22 == DHT22_OK) ? dht22_reading.temperature : 0.0f;
            g_sensor_data.humidity_air = (result_dht22 == DHT22_OK) ? dht22_reading.humidity : 0.0f;
            for (int i = 0; i < 3; i++)
                g_sensor_data.soil_moisture[i] = (result_hw390 == ESP_OK) ? hw390_reading.moisture[i] : 0.0f;
            xSemaphoreGive(sensor_mutex);
        }

        update_sensor_data(
            (result_dht22 == DHT22_OK) ? dht22_reading.temperature : 0.0f,
            (result_dht22 == DHT22_OK) ? dht22_reading.humidity : 0.0f,
            (result_hw390 == ESP_OK) ? hw390_reading.moisture[0] : 0.0f,
            (result_hw390 == ESP_OK) ? hw390_reading.moisture[1] : 0.0f,
            (result_hw390 == ESP_OK) ? hw390_reading.moisture[2] : 0.0f
        );

        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}

void irrigation_task(void *pvParameters) {
    for (int i = 0; i < 3; i++) {
        gpio_set_direction(pump_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(pump_pins[i], 0);
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(IRRIGATION_TASK_PERIOD_MS));

        sensor_data_t local_data;
        if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            local_data = g_sensor_data;
            xSemaphoreGive(sensor_mutex);
        }

        for (int i = 0; i < 3; i++) {
            if (local_data.soil_moisture[i] < 0.0f)
                continue;
            if (local_data.soil_moisture[i] >= MOISTURE_THRESHOLD)
                continue;

            ESP_LOGI(TAG, "Irrigation zone %d", i + 1);

            gpio_set_level(pump_pins[i], 1);
            vTaskDelay(pdMS_TO_TICKS(PUMP_ON_TIME_MS));
            gpio_set_level(pump_pins[i], 0);

            vTaskDelay(pdMS_TO_TICKS(SOAK_DELAY_MS));
        }
    }
}

void fan_task(void *pvParameters) {
    gpio_set_direction(FAN_0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(FAN_1_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(FAN_0_PIN, 0);
    gpio_set_level(FAN_1_PIN, 0);

    while (1) {
        float temp;
        if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            temp = g_sensor_data.temperature;
            xSemaphoreGive(sensor_mutex);
        } else {
            temp = 0.0f;
        }

        uint32_t state = temp > TEMP_THRESHOLD ? 1 : 0;
        gpio_set_level(FAN_0_PIN, state);
        gpio_set_level(FAN_1_PIN, state);

        ESP_LOGI(TAG, "Fans turned %s", state ? "on" : "off");

        vTaskDelay(pdMS_TO_TICKS(FAN_TASK_PERIOD_MS));
    }
}

void app_main(void) {
    // Log redirection
    logs_init();

    // NVS init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    sensor_mutex = xSemaphoreCreateMutex();
    if (!sensor_mutex) {
        ESP_LOGE(TAG, "Failed to create sensor mutex");
        return;
    }

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
    
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(irrigation_task, "irrigation_task", 4096, NULL, 4, NULL);
    xTaskCreate(fan_task, "fan_task", 4096, NULL, 4, NULL);
}
