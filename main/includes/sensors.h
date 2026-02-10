#ifndef SENSORS_H
#define SENSORS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
    float temperature;
    float humidity_air;
    float soil_moisture[3];
} sensor_data_t;

extern sensor_data_t g_sensor_data;
extern SemaphoreHandle_t sensor_mutex;
void update_sensor_data(float temp, float hum_air, float soil_moisture_0, float soil_moisture_1, float soil_moisture_2);

#endif