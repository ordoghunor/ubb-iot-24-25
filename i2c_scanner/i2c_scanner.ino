#include <Wire.h>


int lcd_found;
int light_found;
int rtc_clock_found;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  while (!Serial);
  Serial.println("\nI2C Scanner");

}
 
void loop() {
  int nDevices = 0;
  Serial.println("Scanning...");
 
  // LCD screen - 0x27
  byte lcd_address = 39;
  Wire.beginTransmission(lcd_address);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("LCD device found at address 0x");
    Serial.println(lcd_address, HEX);
    lcd_found = 1;
  } else {
    lcd_found = 0;
  }

  // Fenyero mero - 0x39
  byte light_address = 57;
  Wire.beginTransmission(light_address);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("Light device found at address 0x");
    Serial.println(light_address, HEX);
    light_found = 1;
  } else {
    light_found = 0;
  }

  // RTC clock mero - 0x68
  byte rtc_address = 104;
  Wire.beginTransmission(rtc_address);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("RTC clock found at address 0x");
    Serial.println(rtc_address, HEX);
    rtc_clock_found = 1;
  } else {
    rtc_clock_found = 0;
  }

  if (lcd_found == 0) { 
    Serial.println("LCD device not found");
  } 
  if (light_found == 0) {
    Serial.println("Light device not found");
  } 
  if (rtc_clock_found == 0) {
    Serial.println("RTC clock not found");
  } 

  Serial.println("done\n");
  delay(1000);

}