#include "DHT.h"
//#include <LiquidCrystal.h>
#define DHT22_PIN 7
#define LED_PIN 13
#define TEMP_THRESHOLD 23.0

DHT dht22(DHT22_PIN, DHT22);

//LiquidCrystal lcd(12,11,5,4,3,2);

void setup() {
  Serial.begin(9600);

  //lcd.begin(16,2);
  //lcd.setCursor(0,0);
  //lcd.print("Message");
  
  dht22.begin();
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  delay(2000);

  float tempC = dht22.readTemperature();

  if (isnan(tempC)) {
    Serial.println("Erreur de lecture du capteur DHT22 !");
    return;
  }

  Serial.print("Température: ");
  Serial.print(tempC);
  Serial.println("°C");

  if (tempC > TEMP_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH); // Allume la LED
  } else {
    digitalWrite(LED_PIN, LOW);  // Éteint la LED
  }
}
