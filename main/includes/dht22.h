#ifndef DTH22
#define DTH22

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "rom/ets_sys.h"
#include "esp_log.h"

typedef enum {
    DHT22_OK = 0,
    DHT22_TIMEOUT_ERROR,
    DHT22_CHECKSUM_ERROR,
    DHT22_INIT_ERROR,
    DHT_OUT_LIMIT
} dht22_error_t;

typedef struct {
    float temperature;
    float humidity;
} dht22_reading_t;

int dht22_wait_for_level(gpio_num_t pin, int level, int timeout_us);
esp_err_t dht22_init(void);
dht22_error_t dht22_read(dht22_reading_t *reading);

#endif