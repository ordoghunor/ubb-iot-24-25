#include <Wire.h>


int lcd_found;
int light_found;
int rtc_clock_found;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  while (!Serial);
  Serial.println("\nI2C Scanner");

  // check devices
  // LCD screen - 0x27
  byte lcd_address = 39;
  Wire.beginTransmission(lcd_address);
  byte error_lcd = Wire.endTransmission();
  if (error_lcd == 0) {
    lcd_found = 1;
  } else {
    lcd_found = 0;
  }
  // Fenyero mero - 0x39
  byte light_address = 57;
  Wire.beginTransmission(light_address);
  byte error_ligt = Wire.endTransmission();
  if (error_ligt == 0) {
    light_found = 1;
  } else {
    light_found = 0;
  }
  // RTC clock mero - 0x68
  byte rtc_address = 104;
  Wire.beginTransmission(rtc_address);
  byte error_clock = Wire.endTransmission();
  if (error_clock == 0) {
    rtc_clock_found = 1;
  } else {
    rtc_clock_found = 0;
  }

  if (lcd_found == 1) { 
    Serial.println("LCD device found");
  } 
  if (light_found == 1) {
    Serial.println("Light device found");
  } 
  if (rtc_clock_found == 1) {
    Serial.println("RTC clock found");
  } 

}
 
void loop() {
  delay(5000);
}