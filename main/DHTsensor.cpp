#include "DHTsensor.h"

DHTSensor::DHTSensor(int pin) : dht(pin, DHT22) {}

void DHTSensor::begin() {
  dht.begin();
}

void DHTSensor::update() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

float DHTSensor::getTemperature() { return temperature; }
float DHTSensor::getHumidity() { return humidity; }

