#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

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
unsigned long previousMillisFirebase = 0;
const long interval = 500;
const long intervalFirebase = 5000;

String nextLastKey = "";
String currentDate = "";

const char* ssid = "Mateinfo";
const char* password = "computer";

#define API_KEY "AIzaSyBc0-NTp3oeJFd4o9l1SBQvvDRv8XsJwCk"
#define DATABASE_URL "https://iot-hln-default-rtdb.europe-west1.firebasedatabase.app/"

bool signupOK = false;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

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

  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
    String jsonData = "{";
    jsonData += "\"date\":\"" + dateString + "\",";
    jsonData += "\"light\":\"" + lightString + "\",";
    jsonData += "\"humidity\":\"" + humidityString + "\",";
    jsonData += "\"temperature\":\"" + temperatureString + "\",";
    jsonData += "\"button\":\"" + buttonString + "\",";
    jsonData += "\"motor\":\"" + motorRunningString + "\"";
    jsonData += "}";
    request->send(200, "application/json", jsonData);
  });

  server.on("/startMotor", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("duration")) {
      String durationParam = request->getParam("duration")->value();
      int duration = durationParam.toInt();
      if (duration >= 1 && duration <= 6) {
        startMotor(duration);
        request->send(200, "text/plain", "{\"message\": \"Motor started for " + String(duration) + " seconds\"}");
      } else {
        request->send(400, "text/plain", "Invalid duration.");
      }
    } else {
      request->send(400, "text/plain", "Missing duration parameter.");
    }
  });

  server.on("/getPaginatedData", HTTP_GET, [](AsyncWebServerRequest *request) {
    String date = request->getParam("currentDate")->value() ? request->getParam("currentDate")->value() : currentDate;
    String lastKey = request->hasParam("lastKey") ? request->getParam("lastKey")->value() : "";
    int limit = request->hasParam("limit") ? request->getParam("limit")->value().toInt() : 15;

    String response = getPaginatedFirebaseData(date, lastKey, limit);
    request->send(200, "application/json", response);
  });


  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase signup succesfull. ");
    signupOK = true;
  } else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  server.begin();
}

void loop() {
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    updateSensorData();
  }
  if (millis() - previousMillisFirebase >= intervalFirebase) {
    previousMillisFirebase = millis();
    saveDataToFirebase();
  }
  delay(500);
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

    if (currentDate == "") {
      currentDate = "20" + String(year, DEC) + "-";
      currentDate += (month < 10 ? "0" : "") + String(month, DEC) + "-";
      currentDate += (day < 10 ? "0" : "") + String(day, DEC);
    }
  }
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
  html += "canvas { max-width: 100%; height: auto; margin-top: 50px }";
  html += "</style>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"; // Include Chart.js
  html += "</head><body>";
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
  html += "<button onclick='startMotor()'>Start Motor</button></div>";

  html += "<canvas id='sensorGraph'></canvas>";

  html += "<script>";
  html += "function startMotor() {";
  html += "  var duration = document.getElementById('duration').value;";
  html += "  fetch('/startMotor?duration=' + duration, { method: 'GET' })";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      console.log(data);";
  html += "    }).catch(error => console.error('Error starting motor:', error));";
  html += "}";

  html += "function updateSensorData() {";
  html += "  fetch('/getData').then(response => response.json()).then(data => {";
  html += "    document.getElementById('date').innerText = data.date;";
  html += "    document.getElementById('light').innerText = data.light;";
  html += "    document.getElementById('humidity').innerText = data.humidity;";
  html += "    document.getElementById('temperature').innerText = data.temperature;";
  html += "    document.getElementById('button').innerText = data.button;";
  html += "    document.getElementById('motor').innerText = data.motor;";
    // Add the new data to the graph";
  html += "    const time = data.time;";
  html += "    if (!sensorGraph.data.labels.includes(time)) {";
  html += "      sensorGraph.data.labels.push(time);";
  html += "      sensorGraph.data.datasets[0].data.push(data.temperature);";
  html += "      sensorGraph.data.datasets[1].data.push(data.humidity);";
  html += "      sensorGraph.data.datasets[2].data.push(data.light);";
  html += "      sensorGraph.update();";
  html += "    }";
  html += "  }).catch(error => console.error('Error fetching data:', error));";
  html += "}";

    // Initialize the Chart.js graph
  html += "const ctx = document.getElementById('sensorGraph').getContext('2d');";
  html += "const sensorGraph = new Chart(ctx, {";
  html += "  type: 'line',";
  html += "  data: {";
  html += "    labels: [],"; // Timestamps
  html += "    datasets: [";
  html += "      { label: 'Temperature (°C)', data: [], borderColor: 'red', fill: false },";
  html += "      { label: 'Humidity (%)', data: [], borderColor: 'blue', fill: false },";
  html += "      { label: 'Light (Lux)', data: [], borderColor: 'yellow', fill: false }";
  html += "    ]";
  html += "  },";
  html += "  options: {";
  html += "    responsive: true,";
  html += "    plugins: { legend: { position: 'top' } },";
  html += "    scales: { x: { title: { display: true, text: 'Time' } }, y: { title: { display: true, text: 'Values' } } }";
  html += "  }";
  html += "});";

  // Fetch paginated data and update the graph
  html += "let nextLastKey = ''; let isLoading = false; let limit = 15;";
  html += "function loadGraphData() {";
  html += "  if (isLoading) return;";
  html += "  isLoading = true;";
  html += "  fetch(`/getPaginatedData?lastKey=${nextLastKey}&limit=${limit}`)";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      data.data.forEach(item => {";
  html += "        const time = item.time;";
  html += "        if (!sensorGraph.data.labels.includes(time)) {";
  html += "          sensorGraph.data.labels.push(time);"; // Add time to labels
  html += "          sensorGraph.data.datasets[0].data.push(item.temperature);";
  html += "          sensorGraph.data.datasets[1].data.push(item.humidity);";
  html += "          sensorGraph.data.datasets[2].data.push(item.light);";
  html += "        }";
  html += "      });";
  html += "      nextLastKey = data.nextLastKey;";
  html += "      sensorGraph.update();";
  html += "      if (data.data.length < limit) {"; // Stop
  html += "        console.log('All data loaded');";
  html += "        isLoading = false;";
  html += "        return;";
  html += "      }";
  html += "      isLoading = false;";
  html += "      loadGraphData();"; // Recursive call to fetch the next page";
  html += "    }).catch(error => {";
  html += "      console.error('Error fetching graph data:', error);";
  html += "      isLoading = false;";
  html += "    });";
  html += "}";



  html += "setInterval(updateSensorData, 900);";
  html += "updateSensorData();";
  html += "loadGraphData();";
  html += "</script></body></html>";
  return html;
}

void saveDataToFirebase() {
  FirebaseJson json;
  json.set("time", dateString);
  json.set("light", light);
  json.set("humidity", humidity);
  json.set("temperature", temperature);
  json.set("button", button);
  json.set("motorRunning", motorRunning);

  String path = "/sensorData/" + currentDate;
  if (Firebase.RTDB.pushJSON(&fbdo, path, &json)) {
  } else {
    Serial.println("Error saving data: " + fbdo.errorReason());
  }
}

String getPaginatedFirebaseData(String date, String lastKey, int limit) {
  if (date.isEmpty()) {
    Serial.println("Error: No date provided for fetching data.");
    return;
  }
  String path = "/sensorData/" + date;
  QueryFilter query;
  query.orderBy("$key");
  if (!lastKey.isEmpty()) {
    query.startAt(lastKey);
  }
  query.limitToFirst(limit);

  if (Firebase.RTDB.getJSON(&fbdo, path, &query)) {
    if (fbdo.dataType() == "json") {
      FirebaseJson &json = fbdo.jsonObject();
      size_t dataCount = json.iteratorBegin();
      String jsonResponse = "{";
      jsonResponse += "\"data\": [";
      for (size_t i = 0; i < dataCount; i++) {
        String key, value;
        int type;
        json.iteratorGet(i, type, key, value);

        if (value.startsWith("{")) {
          if (i > 0) {
            jsonResponse += ",";
          }
          jsonResponse += value;
          nextLastKey = key;
        }
      }
      json.iteratorEnd();
      jsonResponse += "],";
      jsonResponse += "\"nextLastKey\": \"" + nextLastKey + "\"";
      jsonResponse += "}";
      return jsonResponse;
    } else {
      Serial.println("No sensor data found for the specified date.");
      return "{\"error\": \"No sensor data found\"}";
    }
    } else {
      Serial.println("Error fetching data: " + fbdo.errorReason());
      return "{\"error\": \"" + fbdo.errorReason() + "\"}";
    }
}

