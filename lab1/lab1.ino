#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Wire.h>
// clock
#include <DS3231.h>
// light sensor 
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#define DHTPIN A3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,20,4);

int gomb_allapota;
int gomb_regi_allapota;

int dht_fail;

int lcd_found;
int light_found;
int rtc_clock_found;

DS3231 myRTC;
bool century = false;
bool h12Flag;
bool pmFlag;

// light sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup() {
  pinMode(A0,INPUT_PULLUP);
  gomb_regi_allapota = digitalRead(A0);

  lcd.init();
  lcd.backlight();

  dht.begin();
  dht_fail = 0;

  Serial.begin(9600);

  Wire.begin();
  checkI2cDevices();

  gomb_allapota = digitalRead(A0);
  lcd.setCursor(12,1);
  lcd.print("| gomb=");
  lcd.print(gomb_allapota);

  lcd.setCursor(0,0);
  lcd.print("Humidity: ");

  lcd.setCursor(0,1);
  lcd.print("Temp: ");


}

void loop() {

  // ============ DHT ============
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    dht_fail = 1;
  }

  Serial.print("Humidity: ");
  lcd.setCursor(10,0);
  if (dht_fail) {
    if (lcd_found) {
      lcd.print("error"); 
    }
    Serial.println("error");
  } else {
    if (lcd_found) {
      lcd.print(h); 
    }
    Serial.println(h);
  }
  
  Serial.print("Temp: ");
  lcd.setCursor(6,1);
  if (dht_fail) {  
    if (lcd_found) {
      lcd.print("error");
    } 
    Serial.println("error");
  } else {
    if (lcd_found) {
      lcd.print(t); 
    }
    Serial.println(t);
  }

  // ============ Gomb ============
  gomb_allapota = digitalRead(A0);
  if (gomb_allapota!=gomb_regi_allapota)
  {
    Serial.print("gomb allapota = ");
    Serial.println(gomb_allapota);
    if (lcd_found) {
      lcd.setCursor(12,1);
      lcd.print("| gomb=");
      lcd.print(gomb_allapota);
    }
    gomb_regi_allapota = gomb_allapota;
  }
  // ============= RTC Clock =============
  if (rtc_clock_found) {
    printTime();
  }
  if (light_found) {
    printLight();
  }

  Serial.println();
  delay(250);
}


void printLight() {
  
}

void printTime() {
  String timeStr = "";
  timeStr += String(myRTC.getYear(), DEC);
  timeStr += ".";
  timeStr += String(myRTC.getMonth(century), DEC);
  timeStr += ".";
  timeStr += String(myRTC.getDate(), DEC);
  timeStr += " ";
  timeStr += String(myRTC.getHour(h12Flag, pmFlag), DEC); // 24-hr
  timeStr += ":";
  timeStr += String(myRTC.getMinute(), DEC);
  timeStr += ":";
  timeStr += String(myRTC.getSecond(), DEC);

  Serial.println(timeStr);
  if (lcd_found) {
    lcd.setCursor(0,2);
    lcd.print(timeStr);
  }
}


void checkI2cDevices() {
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
  Serial.println();
}