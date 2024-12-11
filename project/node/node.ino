#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0); 
// 100 kHz

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
byte motorRunning;

String dateString;
String lightString;
String humidityString;
String temperatureString;
String buttonString;
String motorRunningString;

unsigned long previousMillis = 0;
const long interval = 600;

const char* ssid = "Mateinfo";
const char* password = "computer";

AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  SPI.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Use this URL to connect: http://");
  Serial.println(WiFi.localIP());

  // Serve the main HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateHtmlPage());
  });

  // Handle motor start request
  server.on("/startMotor", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("duration")) {
      String durationParam = request->getParam("duration")->value();
      int duration = durationParam.toInt();
      if (duration >= 1 && duration <= 6) {
        startMotor(duration);
        request->send(200, "text/plain", "Motor started for " + String(duration) + " seconds");
      } else {
        request->send(400, "text/plain", "Invalid duration.");
      }
    } else {
      request->send(400, "text/plain", "Missing duration parameter.");
    }
  });

  server.begin();
}

void loop() {
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    updateSensorData();
  }
}

void startMotor(int duration) {
  SPI.beginTransaction(spi_settings);
  SPI.transfer('s');
  SPI.transfer(duration);
  SPI.endTransaction();
}

void updateSensorData() {
  byte receivedChecksum = requestData();
  byte localChecksum = calculateLocalChecksum();
  if (receivedChecksum != localChecksum) {
    Serial.println("Error receiving sensor data.");
    Serial.print("Received checksum: ");
    Serial.println(receivedChecksum, DEC);
    Serial.print("Local checksum: ");
    Serial.println(localChecksum, DEC);
  } else {
    dateString = createDateString();
    lightString = String(light, DEC);
    humidityString = String(humidity, DEC);
    temperatureString = String(temperature, DEC);
    buttonString = String(button, DEC);
    motorRunningString = String(motorRunning);
  }
  Serial.println("");
}

String createDateString() {
  String timeStr = "20";
  timeStr += String(year, DEC);
  timeStr += ".";
  timeStr += String(month, DEC);
  timeStr += ".";
  timeStr += String(day, DEC);
  timeStr += " ";
  timeStr += String(hour, DEC); 
  timeStr += ":";
  String min = String(minute, DEC);
  if (min.toInt() < 10) {
    timeStr += "0";
  }
  timeStr += min;
  timeStr += ":";
  String sec = String(second, DEC);
  if (sec.toInt() < 10) {
    timeStr += "0";
  }
  timeStr += sec;
  return timeStr;
}

byte requestData() {
  SPI.beginTransaction(spi_settings);
  SPI.transfer('d'); // Start data request, reset checksum
  // first req sets the data on other side
  // each response is for the previous req
  SPI.transfer('Y');
  year = SPI.transfer('M');
  month = SPI.transfer('D');
  day = SPI.transfer('H');
  hour = SPI.transfer('m');
  minute = SPI.transfer('S');
  second = SPI.transfer('l');
  light = SPI.transfer('h');
  humidity = SPI.transfer('t');
  temperature = SPI.transfer('b');
  button = SPI.transfer('r');
  motorRunning = SPI.transfer('c');
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
  calculatedChecksum += motorRunning;
  return calculatedChecksum;
}

String generateHtmlPage() {
  String html = "<!DOCTYPE HTML>";
  html += "<html><head><style>";
  html += "h1 { color: #333; } p { font-size: 18px; } .sensor-data { margin: 0 50px; } button { margin-top: 10px; }";
  html += "</style></head><body>";
  html += "<h1>Sensor Data</h1><div class='sensor-data'>";
  html += "<p><strong>Date:</strong> <span id='date'>" + dateString + "</span></p>";
  html += "<p><strong>Light:</strong> <span id='light'>" + lightString + "</span> Lux</p>";
  html += "<p><strong>Humidity:</strong> <span id='humidity'>" + humidityString + "</span> %</p>";
  html += "<p><strong>Temperature:</strong> <span id='temperature'>" + temperatureString + "</span> C</p>";
  html += "<p><strong>Button State:</strong> <span id='button'>" + buttonString + "</span></p>";
  html += "<p><strong>Motor Running:</strong> <span id='motor'>" + motorRunningString + "</span></p>";
  html += "<div class='motor-control' style='margin: 0 20px;'>";
  html += "<label for='duration'>Motor Duration (1-6 seconds):</label>";
  html += "<input type='number' id='duration' name='duration' min='1' max='6' value='1'>";
  html += "<button onclick='startMotor()'>Start Motor</button>";
  html += "</div><script>function startMotor() {";
  html += "var duration = document.getElementById('duration').value;";
  html += "var xhr = new XMLHttpRequest(); xhr.open('GET', '/startMotor?duration=' + duration, true); xhr.send();";
  html += "}</script></body></html>";
  return html;
}
