#include "sensor.h"
#include "sensorManager.h"
#include "DHTsensor.h"

void SensorManager::begin() {
  sensors[0] = new DHTSensor(DHT_pin);

  for (short i = 0; i < NB_SENSOR; i++) sensors[i]->begin();
}

void SensorManager::update() {
  for (short i = 0; i < NB_SENSOR; i++) sensors[i]->update();

  data.temperature = ((DHTSensor*)sensors[0])->getTemperature();
  data.humidity = ((DHTSensor*)sensors[0])->getHumidity();
}

SensorData SensorManager::getData() {
  return data;
}

