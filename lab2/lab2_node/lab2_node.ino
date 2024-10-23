#include<SPI.h>


SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0); 
//100 kHz

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

unsigned long previousMillis = 0;
const long interval = 1000;


void setup() {  
 Serial.begin(9600);  
 SPI.begin();
}

void loop() {
  if (currentMillis - previousMillis >= interval) {
    // request data only every second
    previousMillis = currentMillis;
    updateSensorData();
  };

  delay(10);  
}


void updateSensorData() {
  byte receivedChecksum = requestData();
  byte localChecksum = calculateLocalChecksum();
  if (receivedChecksum != localChecksum) {
    // error receiving data
    Serial.println("Error receiving sensor data.");
  }
}


byte requestData() {
  SPI.beginTransaction(spi_settings);
  SPI.transfer('d'); // Start data request, reset checksum
  // first req sets the data on other side
  // each response is for the previous req
  SPI.transfer('1');
  year = SPI.transfer('2');
  month = SPI.transfer('3');
  day = SPI.transfer('4');
  hour = SPI.transfer('5');
  minute = SPI.transfer('6');
  second = SPI.transfer('l');
  light = SPI.transfer('h');
  humidity = SPI.transfer('t');
  temperature = SPI.transfer('b');
  button = SPI.transfer('c');
  byte receivedChecksum = SPI.transfer('.');

  SPI.endTransaction();

  return receivedChecksum;
}

byte calculateLocalChecksum() {
  byte calculatedChecksum = 0;
  calculatedChecksum += year;
  calculatedChecksum += month;
  calculatedChecksum += day;
  calculatedChecksum += hour;
  calculatedChecksum += minute;
  calculatedChecksum += second;
  calculatedChecksum += light;
  calculatedChecksum += humidity;
  calculatedChecksum += temperature;
  calculatedChecksum += button;
  return calculatedChecksum;
}