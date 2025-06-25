#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include "sensor.h"
#include <DHT.h>

class DHTSensor : public Sensor {
public:
  DHTSensor(int pin);
  void begin() override;
  void update() override;
  float getTemperature();
  float getHumidity();

private:
  DHT dht;
  float temperature, humidity;
};

#endif
