#include "sensorManager.h"
#include "screen.h"
#include "pin.h"

#define TEMP_THRESHOLD 25.0

SensorManager sensorManager;
Screen screen;

void setup() {
  Serial.begin(9600);
  sensorManager.begin();
  screen.begin();
}

void loop() {
  sensorManager.update();
  screen.update();

  // Print temperature on screen
  screen.printValue("Temp", sensorManager.data.temperature, "C");

  // Turn the fan on when temperature exceed threshold
  digitalWrite(FAN_pin, LOW);

  if (temperature > TEMP_THRESHOLD) {
    digitalWrite(FAN_pin, HIGH); 
  } else {
    digitalWrite(FAN_pin, LOW);  
  }

  delay(1000);
}