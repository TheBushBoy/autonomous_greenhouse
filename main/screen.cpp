#include "screen.h"

Screen::Screen() : lcd(RS, E, D4, D5, D6, D7) {}

void Screen::begin() {
  lcd.begin(16, 2);
}

void Screen::update() {
  char value_str[16];

  if (isnan(this->value)) {
    value_str = "Err";
  }
  else {
    dtostrf(value, 1, 1, value_str);
    strcat(value_str, unit)
  }

  int name_len = strlen(name);
  int value_len = strlen(value_str);
  int space_len = 16 - name_len - value_len;
  if (space_len < 0) space_len = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(name);

  for (int i = 0; i < space_len; i++) {
    lcd.print(" ");
  }

  lcd.print(value_str);
}

void Screen::printValue(char* new_name, float new_value, char new_unit) {
  this->name = new_name;
  this->value = new_value;
  this->unit = new_unit;
}