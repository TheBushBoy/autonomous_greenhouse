#ifndef SCREEN_H
#define SCREEN_H

#include <LiquidCrystal.h>
#include <string.h>

#include "pin.h"

class Screen {
public:
  Screen();
  void begin();
  void update();
  void printValue(char* new_name, float new_value, char new_unit);

private:
  LiquidCrystal lcd;
  char* name;
  float value;
  char unit;
};

#endif