#include<SPI.h>
#include <ESP8266WiFi.h>

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
WiFiServer server(80);




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
  server.begin();
  Serial.println("Server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  if (millis() - previousMillis >= interval) {
    // request data only every second
    previousMillis = millis();
    updateSensorData();
  };

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

    // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  if (request.startsWith("GET /startMotor")) {
    handleMotorStart(client, request);
  } else {
    sendHtml(client);
  }
  client.stop();

  delay(50);  
}



void handleMotorStart(WiFiClient client, String request) {
  int duration = -1;

  if (request.indexOf("duration=") >= 0) {
    int startIndex = request.indexOf("duration=") + 9;
    int endIndex = request.indexOf(' ', startIndex);
    String durationStr = request.substring(startIndex, endIndex);
    duration = durationStr.toInt();
  }

  if (duration >= 0 && duration <= 6) {
    startMotor(duration);
  } else {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: text/plain");
    client.println("");
    client.println("Invalid duration.");
  }
}

void sendHtml(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<style>");
  client.println("h1 { color: #333; }");
  client.println("p { font-size: 18px; }");
  client.println(".sensor-data { margin: 0 50px; }");
  client.println("button { margin-top: 10px; }");
  client.println("</style>");
  client.println("</head>");

  client.println("<body>");
  client.println("<h1>Sensor Data</h1>");
  client.println("<div class='sensor-data'>");
  client.println("<p><strong>Date:</strong> <span id='date'>" + dateString + "</span></p>");
  client.println("<p><strong>Light:</strong> <span id='light'>" + lightString + "</span> Lux</p>");
  client.println("<p><strong>Humidity:</strong> <span id='humidity'>" + humidityString + "</span> %</p>");
  client.println("<p><strong>Temperature:</strong> <span id='temperature'>" + temperatureString + "</span> C</p>");
  client.println("<p><strong>Button State:</strong> <span id='button'>" + buttonString + "</span></p>");
  client.println("<p><strong>Motor Running:</strong> <span id='motor'>" + motorRunningString + "</span></p>");


  client.println("<div class='motor-control' style='margin: 0 20px;'>");
  client.println("<label for='duration'>Motor Duration (1-6 seconds):</label>");
  client.println("<input type='number' id='duration' name='duration' min='1' max='6' value='1'>");
  client.println("<button onclick='startMotor()'>Start Motor</button>");
  client.println("</div>");

  // start motor script
  client.println("<script>");
  client.println("function startMotor() {");
  client.println("  var duration = document.getElementById('duration').value;");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.open('GET', '/startMotor?duration=' + duration, true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("</script>");

  client.println("</body>");
  client.println("</html>");
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
    // error receiving data
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