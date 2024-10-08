#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN A3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,20,4);

int dht_fail;

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  dht.begin();
  dht_fail = 0;
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    dht_fail = 1;
  }

  lcd.setCursor(0,0);
  lcd.print("Humidity: "); Serial.print("Humidity: ");
  if (dht_fail) {
    lcd.print("NaN"); Serial.println("NaN");
  } else {
    lcd.print(h); Serial.println(h);
  }

  lcd.setCursor(0,1);
  lcd.print("Temp: "); Serial.print("Temp: ");
  if (dht_fail) {  
    lcd.print("NaN"); Serial.println("NaN");
  } else {
    lcd.print(t); Serial.println(t);
  }

  Serial.println();

  delay(2000);
  lcd.clear();
}
