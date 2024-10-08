#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN A3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,20,4);

int gomb_allapota;
int gomb_regi_allapota;

int dht_fail;

void setup() {
  pinMode(A0,INPUT_PULLUP);
  gomb_regi_allapota = digitalRead(A0);

  lcd.init();
  lcd.backlight();

  dht.begin();
  dht_fail = 0;

  Serial.begin(9600);

  gomb_allapota = digitalRead(A0);
  lcd.setCursor(12,1);
  lcd.print("| gomb=");
  lcd.print(gomb_allapota);

  lcd.setCursor(0,0);
  lcd.print("Humidity: ");

  lcd.setCursor(0,1);
  lcd.print("Temp: ");

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    dht_fail = 1;
  }
}

void loop() {

  // ============ DHT ============

  Serial.print("Humidity: ");
  lcd.setCursor(10,0);
  if (dht_fail) {
    lcd.print("error");
    Serial.println("error");
  } else {
    lcd.print(h);
    Serial.println(h);
  }
  
  Serial.print("Temp: ");
  lcd.setCursor(6,1);
  if (dht_fail) {  
    lcd.print("error");
    Serial.println("error");
  } else {
    lcd.print(t);
    Serial.println(t);
  }
  Serial.println();

  // ============ Gomb ============
  gomb_allapota = digitalRead(A0);
  if (gomb_allapota!=gomb_regi_allapota)
  {
    Serial.print("gomb allapota = ");
    Serial.println(gomb_allapota);
    lcd.setCursor(12,1);
    lcd.print("| gomb=");
    lcd.print(gomb_allapota);
    gomb_regi_allapota = gomb_allapota;
  }

  delay(250);
}
