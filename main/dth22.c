/**
* Made with documentation
* - https://www.alldatasheet.com/datasheet-pdf/view/1132459/ETC2/DHT22.html
*/

#include "includes/pins.h"
#include "includes/dht22.h"

#define DHT22_START_SIGNAL_US 1000
#define DHT22_TIMEOUT_US 100

static const char *TAG = "DHT22";

int dht22_wait_for_level(gpio_num_t pin, int level, int timeout_us) {
    int elapsed = 0;
    while (gpio_get_level(pin) != level) {
        if (elapsed > timeout_us) {
            return -1;
        }
        esp_rom_delay_us(1);
        elapsed++;
    }
    return elapsed;
}

esp_err_t dht22_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT22_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error GPIO");
        return ret;
    }
    
    gpio_set_level(DHT22_PIN, 1);
    return ESP_OK;
}

dht22_error_t dht22_read(dht22_reading_t *reading) {
    uint8_t data[5] = {0}; // Humidity, MSB and LSB, Temperature, MSB and LSB, and checksum
    
    // Pre-reading sequence
    portDISABLE_INTERRUPTS(); // for more precise timing
    
    gpio_set_level(DHT22_PIN, 0);
    esp_rom_delay_us(DHT22_START_SIGNAL_US);
    gpio_set_level(DHT22_PIN, 1);

    if (dht22_wait_for_level(DHT22_PIN, 0, DHT22_TIMEOUT_US) < 0) {
        portENABLE_INTERRUPTS();
        ESP_LOGE(TAG, "Timeout");
        return DHT22_TIMEOUT_ERROR;
    }
    
    if (dht22_wait_for_level(DHT22_PIN, 1, DHT22_TIMEOUT_US) < 0) {
        portENABLE_INTERRUPTS();
        ESP_LOGE(TAG, "Timeout");
        return DHT22_TIMEOUT_ERROR;
    }
    
    // Bits reading
    for (int i = 0; i < 40; i++) {
        if (dht22_wait_for_level(DHT22_PIN, 0, DHT22_TIMEOUT_US) < 0) {
            portENABLE_INTERRUPTS();
            ESP_LOGE(TAG, "Timeout reading bit %d (LOW)", i);
            return DHT22_TIMEOUT_ERROR;
        }
        
        if (dht22_wait_for_level(DHT22_PIN, 1, DHT22_TIMEOUT_US) < 0) {
            portENABLE_INTERRUPTS();
            ESP_LOGE(TAG, "Timeout reading bit %d (HIGH)", i);
            return DHT22_TIMEOUT_ERROR;
        }
        
        // Hold for 30µs HIGH is 0
        // Hold for 70µs HIGH is 1
        esp_rom_delay_us(50);
        
        int byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);
        
        if (gpio_get_level(DHT22_PIN) == 1) {
            data[byte_idx] |= (1 << bit_idx);
        }
        // else let bit index to 0
    }
    
    portENABLE_INTERRUPTS();
    
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Error checksum: calculé=0x%02x, reçu=0x%02x", checksum, data[4]);
        ESP_LOGE(TAG, "Data: %02x %02x %02x %02x %02x", 
                data[0], data[1], data[2], data[3], data[4]);
        return DHT22_CHECKSUM_ERROR;
    }

    uint16_t humidity_raw = (data[0] << 8) | data[1];
    reading->humidity = humidity_raw / 10.0f;
    
    uint16_t temperature_raw = ((data[2] & 0x7F) << 8) | data[3];
    reading->temperature = temperature_raw / 10.0f;

    if (reading->temperature < -40.0f || reading->temperature > 80.0f) {
        return DHT_OUT_LIMIT;
    }
    if (reading->humidity < 0.0f || reading->humidity > 100.0f) {
        return DHT_OUT_LIMIT;
    }
    
    if (data[2] & 0x80) {
        reading->temperature = -reading->temperature;
    }
    
    ESP_LOGD(TAG, "Data: %02x %02x %02x %02x [%02x]", 
            data[0], data[1], data[2], data[3], data[4]);
    
    return DHT22_OK;
}