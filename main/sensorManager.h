#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include "sensor.h"
#include "pin.h"

#define NB_SENSOR 1

struct SensorData {
  float temperature;
  float humidity;
  // Add sensors here
};

class SensorManager {
public:
  void begin();
  void update();
  SensorData getData();

private:
  Sensor* sensors[NB_SENSOR];
  SensorData data;
};

#endif