#ifndef HW390
#define HW390

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define HW390_SENSOR_COUNT 3

typedef struct {
    float moisture[HW390_SENSOR_COUNT];
} hw390_reading_t;

esp_err_t hw390_init(void);
esp_err_t hw390_read(hw390_reading_t *reading);

#endif