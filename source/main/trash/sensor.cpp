#include "sensor.h"
#include "DHTsensor.h"

#define DHT_pin D2

void SensorManager::begin() {
  sensors.push_back(new DHTSensor(DHT_pin));

  for (Sensor* s : sensors) s->begin();
}

void SensorManager::update() {
  for (Sensor* s : sensors) s->update();

  data.temperature = ((DHTSensor*)sensors[0])->getTemperature();
  data.humidity    = ((DHTSensor*)sensors[0])->getHumidity();
}

SensorData SensorManager::getData() {
  return data;
}

