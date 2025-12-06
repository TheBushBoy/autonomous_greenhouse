/**
* Capacitive analogic moisture sensor HW390
*/

#include "includes/hw390.h"

#define HW390_ADC_UNIT     ADC_UNIT_1
#define HW390_ADC_CHANNEL  ADC_CHANNEL_6 // GPIO 34
#define HW390_ADC_ATTEN    ADC_ATTEN_DB_12
#define HW390_ADC_BITWIDTH ADC_BITWIDTH_12

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

    // Configure ADC channel
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = HW390_ADC_BITWIDTH,
        .atten = HW390_ADC_ATTEN,
    };
    ret = adc_oneshot_config_channel(adc_handle, HW390_ADC_CHANNEL, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC channel");
        return ret;
    }

    return ESP_OK;
}

float hw390_read_moisture_percent(void) {
    int adc_raw = 0;
    esp_err_t ret = adc_oneshot_read(adc_handle, HW390_ADC_CHANNEL, &adc_raw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read failed");
        return -1.0f;
    }

    float percent = 100.0f - ((float)(adc_raw - HW390_VALUE_IN_WATER) / 
                              (float)(HW390_VALUE_IN_AIR - HW390_VALUE_IN_WATER) * 100.0f);
    
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    return percent;
}