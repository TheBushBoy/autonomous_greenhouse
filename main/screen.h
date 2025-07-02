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

private:
  LiquidCrystal lcd;
  char* name
  float value
};

#endif