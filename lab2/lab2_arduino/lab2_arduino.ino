#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Wire.h>
// clock
#include <DS3231.h>
// light sensor 
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
// motor
#include <Stepper.h>

#include <SPI.h>


#define DHTPIN A3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27,20,4);

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

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

int manual_mode = 0;
unsigned long manual_start_time = 0;

// motor
const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

// data

byte year;
byte month;
byte day;
byte hour;
byte minute;
byte second;
byte light;
byte humidity;
byte temperature;
byte button;

byte checksum;

bool motor_started = false;
unsigned long motor_started_time = 0;
unsigned long motor_run_time;

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
  lcd.print("Hum: ");

  lcd.setCursor(0,1);
  lcd.print("Temp: ");

  if(light_found) {
    tsl.begin();
    configureTslSensor();
  }

  myStepper.setSpeed(60);

  // open SPI port for com with nodeMCU
  SPCR |= bit(SPE); 
  // arduino is slave
  pinMode(MISO, OUTPUT);   
  SPI.attachInterrupt();   
}

void loop() {

  // ============ DHT ============
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  dht_fail = 0;
  if (isnan(h) || isnan(t)) {
    dht_fail = 1;
  }

  Serial.print("Humidity: ");
  lcd.setCursor(5,0);
  if (dht_fail) {
    if (lcd_found) {
      lcd.print("error"); 
    }
    Serial.println("error");
  } else {
    humidity = h;
    if (lcd_found) {
      lcd.print(h); 
      lcd.print("% | "); 
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
    temperature = t;
    if (lcd_found) {
      lcd.print(t); 
    }
    Serial.println(t);
  }

  // ============ Gomb ============
  gomb_allapota = digitalRead(A0);
  if (gomb_allapota!=gomb_regi_allapota) {
    Serial.print("gomb allapota = ");
    Serial.println(gomb_allapota);
    if (lcd_found) {
      lcd.setCursor(12,1);
      lcd.print("| gomb:");
      lcd.print(gomb_allapota);
    }
    gomb_regi_allapota = gomb_allapota;
    button = gomb_allapota;
    // start manual mode
    manual_mode = 1;
    printMode();
    Serial.println("Manualis modra valtva");
    manual_start_time = millis();
    // motor step
    myStepper.step(stepsPerRevolution);
  }
  // ============= RTC Clock =============
  if (rtc_clock_found) {
    printTime();
  }
  // ============= Light sensor =============
  if (light_found) {
    checkLight();
  }
  // ============= Mode =============
  printMode();

  // check manual mode end, reset after 3 seconds
  if (manual_mode == 1 && millis() - manual_start_time >= 3000) {
    manual_mode = 0;  
    Serial.println("Manualis mod vege");
  }
  if (motor_started) {
    myStepper.step(stepsPerRevolution);
  }
  // motor started from node
  if (motor_started == true && millis() - motor_started_time >= motor_run_time) {
    motor_started = false;  
    Serial.println("Stopping motor");
  }

  Serial.println();
  delay(200);
}


void printMode() {
  if (manual_mode == 1) {
    Serial.println("Manualis mod");
    if (lcd_found) {
    lcd.setCursor(14,0);
    lcd.print("Mod: M");
  }
  } else {
    Serial.println("Automata mod");
    if (lcd_found) {
    lcd.setCursor(14,0);
    lcd.print("Mod: A");
  }
  }
  
}

void checkLight() {
  sensors_event_t event;
  tsl.getEvent(&event);
  if (event.light) {
    light = event.light;
    Serial.print(event.light); Serial.println(" lux");
    if (lcd_found) {
      lcd.setCursor(0,2);
      lcd.print("Feny: ");
      lcd.print(event.light);
      lcd.print(" lux   ");
    }
    if (event.light > 1000) {
      // over 1000 - start motor
      // motor step
      myStepper.step(stepsPerRevolution);
    }
  } else {
    Serial.println("Sensor overload");
    if (lcd_found) {
      lcd.setCursor(0,2);
      lcd.print("Feny: error");
    }
  }
}


void configureTslSensor(void) {
  tsl.enableAutoRange(true); 
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  Serial.println("Tsl Gain: Auto");
  Serial.println("Timing: 13 ms");
}


void printTime() {
  String timeStr = "20";
  year = myRTC.getYear();
  timeStr += String(year, DEC);
  timeStr += ".";
  month = myRTC.getMonth(century);
  timeStr += String(month, DEC);
  timeStr += ".";
  day = myRTC.getDate();
  timeStr += String(day, DEC);
  timeStr += " ";
  hour = myRTC.getHour(h12Flag, pmFlag); // 24-hr
  timeStr += String(hour, DEC); 
  timeStr += ":";
  minute = myRTC.getMinute();
  String min = String(minute, DEC);
  if (min.toInt() < 10) {
    timeStr += "0";
  }
  timeStr += min;
  timeStr += ":";
  second = myRTC.getSecond();
  String sec = String(second, DEC);
  if (sec.toInt() < 10) {
    timeStr += "0";
  }
  timeStr += sec;

  Serial.print("Time: ");
  Serial.println(timeStr);
  if (lcd_found) {
    lcd.setCursor(0,3); 
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


void startMotor(byte duration) {
  Serial.println("===================");
  Serial.print("Starting motor from node for ");
  motor_started_time = millis();
  motor_run_time = duration * 1000;
  Serial.print(motor_run_time);
  Serial.println(" milisec");

}


//SPI interrupt routine
ISR(SPI_STC_vect) {
  char c = SPDR;
  switch (c) {
    case 'd': checksum = 0; break;
    case 'Y': SPDR = year; checksum += year; break;
    case 'M': SPDR = month; checksum += month; break;
    case 'D': SPDR = day; checksum += day; break;
    case 'H': SPDR = hour; checksum += hour; break;
    case 'm': SPDR = minute; checksum += minute; break;
    case 'S': SPDR = second; checksum += second; break;
    case 'l': SPDR = light; checksum += light; break;
    case 'h': SPDR = humidity; checksum += humidity; break;
    case 't': SPDR = temperature; checksum += temperature; break;
    case 'b': SPDR = button; checksum += button; break;
    case 'r': SPDR = motor_started; checksum += motor_started; break;
    case 'c': SPDR = checksum; Serial.print("checksum  "); Serial.println(checksum); break;
    case 's': motor_started = true; break;
    default:
      if (c >= 0 && c <= 9) {
        startMotor(c);
      }
  }
  
}