#ifndef SCREEN_H
#define SCREEN_H

#include <LiquidCrystal.h>
#include <string.h>

#define RS 12
#define E 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2

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