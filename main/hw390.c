/**
* Capacitive analogic moisture sensor HW390
*/

#include "includes/hw390.h"

#define HW390_ADC_UNIT     ADC_UNIT_1
#define HW390_ADC_ATTEN    ADC_ATTEN_DB_12
#define HW390_ADC_BITWIDTH ADC_BITWIDTH_12

#define HW390_SENSOR_COUNT 3

static const adc_channel_t hw390_adc_channels[HW390_SENSOR_COUNT] = {
    ADC_CHANNEL_4, // GPIO 32
    ADC_CHANNEL_6, // GPIO 34
    ADC_CHANNEL_7  // GPIO 35
};

#define HW390_VALUE_IN_WATER 1200
#define HW390_VALUE_IN_AIR   3000

static const char *TAG = "SOIL_MOISTURE";
static adc_oneshot_unit_handle_t adc_handle = NULL;

esp_err_t hw390_init(void) {
    // Initialize ADC unit
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = HW390_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC unit");
        return ret;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = HW390_ADC_BITWIDTH,
        .atten = HW390_ADC_ATTEN,
    };

    for (int i = 0; i < HW390_SENSOR_COUNT; i++) {
        ret = adc_oneshot_config_channel(adc_handle,
                                        hw390_adc_channels[i],
                                        &config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to config ADC channel %d", i);
            return ret;
        }
    }

    return ESP_OK;
}

esp_err_t hw390_read(hw390_reading_t *reading) {
    for (int i = 0; i < HW390_SENSOR_COUNT; i++) {
        int adc_raw = 0;
        esp_err_t ret = adc_oneshot_read(adc_handle,
                                        hw390_adc_channels[i],
                                        &adc_raw);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Read failed");
            return ret;
        }

        float percent = 100.0f - ((float)(adc_raw - HW390_VALUE_IN_WATER) /
                                (float)(HW390_VALUE_IN_AIR - HW390_VALUE_IN_WATER) * 100.0f);

        if (percent < 0.0f) percent = 0.0f;
        if (percent > 100.0f) percent = 100.0f;

        reading->moisture[i] = percent;
        vTaskDelay(pdMS_TO_TICKS(10)); // wait for adc to stabilize
    }

    return ESP_OK;
}