#include "sensorManager.h"
#include "screen.h"

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
  delay(1000);
}
/*
// LCD : RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// DHT22 config
#define DHT22_PIN 7      // Broche DATA du DHT22
DHT dht22(DHT22_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  pinMode(8, OUTPUT);
  
  lcd.begin(16, 2);  // LCD 16 colonnes, 2 lignes
  dht22.begin();       // Initialisation du capteur
  lcd.print("Temp    Humidity");
}

void loop() {
  float temperature = dht22.readTemperature();
  float umidity = dht22.readHumidity();
  digitalWrite(8, HIGH);
  
  if (isnan(temperature)) {
    lcd.setCursor(0, 1);
    lcd.print("Erreur capteur");
  } else {
    lcd.setCursor(0, 1);
    lcd.print(temperature);
    lcd.print(" C  ");
    
    lcd.print(umidity);
    lcd.print (" %  "); // Espaces pour effacer les anciens caractères
  }

  if (temperature > TEMP_THRESHOLD) {
    digitalWrite(8, HIGH); 
  } else {
    digitalWrite(8, LOW);  
  }

  unsigned long now = millis();
  Serial.println(now);
  delay(1000); // Lecture toutes les 2 secondes
}
*/