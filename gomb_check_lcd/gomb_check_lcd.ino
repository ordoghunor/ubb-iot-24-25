#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

int gomb_allapota;
int gomb_regi_allapota;

void setup() {
  pinMode(A0,INPUT_PULLUP);
  gomb_regi_allapota = digitalRead(A0);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
}

void loop() {
  gomb_allapota = digitalRead(A0);
  if (gomb_allapota!=gomb_regi_allapota)
  {
    lcd.setCursor(0,0);
    lcd.print("gomb allapota = ");
    lcd.print(gomb_allapota);
    Serial.print("gomb allapota = ");
    Serial.println(gomb_allapota);
    gomb_regi_allapota = gomb_allapota;
  }
  delay(100);
}
