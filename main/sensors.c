#include "includes/sensors.h"

sensor_data_t g_sensor_data = {0};
SemaphoreHandle_t sensor_mutex = NULL;

void update_sensor_data(float temp, float hum_air, float soil_moisture_0, float soil_moisture_1, float soil_moisture_2) {
    if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(50))) {
        g_sensor_data.temperature = temp;
        g_sensor_data.humidity_air = hum_air;
        g_sensor_data.soil_moisture[0] = soil_moisture_0;
        g_sensor_data.soil_moisture[1] = soil_moisture_1;
        g_sensor_data.soil_moisture[2] = soil_moisture_2;
        xSemaphoreGive(sensor_mutex);
    }
}
