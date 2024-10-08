#include <Wire.h>
 
void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  Wire.begin();
  while (!Serial);
  Serial.println("\nI2C Scanner");

}
 
void loop() {
  int nDevices = 0;
  Serial.println("Scanning...");
  lcd.setCursor(0,0);
 
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
 
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      lcd.print("I2C device: ")
      if (address < 16) {
        Serial.print("0");
        lcd.print("0")
      }
      Serial.print(address, HEX);
      lcd.print(address, HEX)
      Serial.println("  !");
 
      ++nDevices;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    lcd.print("No I2C devices found.")
  } else {
    Serial.println("done\n");
  }
  delay(1000);

  lcd.clear();
}