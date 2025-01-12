#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include <map>

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

std::map<String, String> olderDataDictionary;
String timeParamArray[10];
int timeParamCount = 0;


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
    request->send(200, "text/html; charset=UTF-8", generateHtmlPage());
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

  server.on("/requestOlderData", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("time")) {
      String timeParam = request->getParam("time")->value();
      if (timeParamCount < 10) {
        timeParamArray[timeParamCount++] = timeParam;
      }
      request->send(200, "text/plain", "Ok");
    } else {
      request->send(400, "text/plain", "Missing time parameter.");
    }
  });

  server.on("/getOlderData", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("time")) {
      String timeParam = request->getParam("time")->value();
      String data = olderDataDictionary[timeParam];
      olderDataDictionary.erase(timeParam);
      request->send(200, "application/json", data);
    } else {
        request->send(400, "text/plain", "Missing time parameter.");
    }
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
  processTimeParamArray();
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

    currentDate = "20" + String(year, DEC) + "-";
    currentDate += (month < 10 ? "0" : "") + String(month, DEC) + "-";
    currentDate += (day < 10 ? "0" : "") + String(day, DEC)+ "-";
    currentDate += (hour < 10 ? "0" : "") + String(hour, DEC) + "-";
    currentDate += (minute < 10 ? "0" : "") + String(minute, DEC);
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
  html += "canvas{max-width: 100%; height: 300px;margin-top:20px;}";
  html += "</style>";
  html += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css'>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "</head><body class='bg-light'>";
  html += "<div class='container my-5'>";
  html += "<h1 class='text-center'>Sensor Data</h1>";
  
  // Sensor Data Display Section
  html += "<div class='row text-center'>";
  html += "  <div class='col-md-6'><p><strong>Date:</strong><span id='date'> " + dateString + "</span></p></div>";
  html += "  <div class='col-md-6'><p><strong>Light:</strong><span id='light'> " + lightString + "</span> Lux</p></div>";
  html += "</div>";
  html += "<div class='row text-center'>";
  html += "  <div class='col-md-6'><p><strong>Humidity:</strong><span id='humidity'> " + humidityString + "</span> %</p></div>";
  html += "  <div class='col-md-6'><p><strong>Temperature:</strong><span id='temperature'> " + temperatureString + "</span> °C</p></div>";
  html += "</div>";
  html += "<div class='row text-center'>";
  html += "  <div class='col-md-6'><p><strong>Button State:</strong><span id='button'> " + buttonString + "</span></p></div>";
  html += "  <div class='col-md-6'><p><strong>Motor Running:</strong><span id='motor'> " + motorRunningString + "</span></p></div>";
  html += "</div>";

  // Motor Control Section
  html += "<div class='motor-control mb-4'>";
  html += "<label for='duration'>Motor Duration (1-6 seconds):</label>";
  html += "<input type='number' class='form-control w-25 d-inline-block ml-2' id='duration' name='duration' min='1' max='6' value='1'>";
  html += "<button class='btn btn-primary ml-2' onclick='startMotor()'>Start Motor</button>";
  html += "</div>";

  // Chart Section
  html += "<div class='card'>";
  html += "<div class='card-body'>";
  html += " <h5 class='card-title'>Sensor Data Over Time</h5>";
  html += " <canvas id='sensorGraph'></canvas>";
  html += "</div>";
  html += "</div>";

  // older data
  html += "<div class='mt-5 d-flex flex-column align-items-center justify-content-center'>";
  html += " <h3>Retrieve Older Data</h3>";
  html += " <label for='timePicker'>Select Time:</label>";
  html += " <input type='datetime-local' id='timePicker' class='form-control w-25'>";
  html += " <button class='btn btn-primary mt-2' onclick='fetchOlderData()'>Fetch Data</button>";
  html += "<div class='mt-3' id='output'></div>";
  html += "</div>";

  html += "</div>";

  html += "<script>";
  // JavaScript for Chart and Sensor Data Update
  html += "function startMotor() {";
  html += "fetch('/startMotor?duration=' + document.getElementById('duration').value)";
  html += ".catch(error => {});";
  html += "}";

  html += "function updateSensorData() {";
  html += " fetch('/getData').then(response => response.json()).then(data => {";
  html += "document.getElementById('date').innerText = data.date;";
  html += "document.getElementById('light').innerText = data.light;";
  html += "document.getElementById('humidity').innerText = data.humidity;";
  html += "document.getElementById('temperature').innerText = data.temperature;";
  html += "document.getElementById('button').innerText = data.button;";
  html += "document.getElementById('motor').innerText = data.motor;";
    // Add the new data to the graph";
  html += "const time = data.date;";
  html += "if (!sensorGraph.data.labels.includes(time)) {";
  html += "sensorGraph.data.labels.push(time);";
  html += "sensorGraph.data.datasets[0].data.push(data.temperature);";
  html += "sensorGraph.data.datasets[1].data.push(data.humidity);";
  html += "sensorGraph.data.datasets[2].data.push(data.light);";
  html += "sensorGraph.update();";
  html += " }";
  html += "}).catch(error => {});";
  html += "}";

    // Initialize the Chart.js graph
  html += "const ctx = document.getElementById('sensorGraph').getContext('2d');";
  html += "const sensorGraph = new Chart(ctx, {";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [],"; // Timestamps
  html += "datasets: [";
  html += "{ label: 'Temperature (°C)', data:[], borderColor: 'red', fill: false },";
  html += "{ label: 'Humidity (%)',data:[], borderColor: 'blue', fill: false },";
  html += "{ label: 'Light (Lux)',data:[], borderColor: 'yellow', fill: false }";
  html += "]";
  html += " },";
  html += "options: {";
  html += "responsive: true,";
  html += "plugins: { legend: { position: 'top' } },";
  html += "scales: { x: { title: { display: true, text: 'Time' }}, y:{ title: { display: true, text: 'Values' } } }";
  html += "}";
  html += "});";

  html += "function fetchOlderData() {";
  html += "  var time = document.getElementById('timePicker').value;";
  html += "  if (time) {";
  html += "var date = new Date(time);";
  html += "var formattedDate = date.getFullYear() + '-';";
  html += "formattedDate += (date.getMonth() + 1 < 10 ? '0' : '')+(date.getMonth() + 1)+'-'";
  html += "+(date.getDate() < 10 ? '0' : '') + date.getDate() + '-'";
  html += "+(date.getHours() < 10 ? '0' : '') + date.getHours() + '-'";
  html += "+(date.getMinutes() < 10 ? '0' : '') + date.getMinutes();";
  html += "fetch('/requestOlderData?time=' + formattedDate);";
  html += "setTimeout(() => fetch('/getOlderData?time=' + formattedDate).then(r => r.json()).then(data=>{if(data.error){output.innerHTML='<p>Error:' + data.error + '</p>';}";
  html += "else {output.innerHTML='<p><strong>Humidity: '+data.data.humidity+'%</strong></p>' + '<p><strong>Light: '+data.data.light+'</strong></p>' + '<p><strong>Temperature: ' + data.data.temperature + '°C</strong></p>';}}), 1000);";
  html += "} else {";
  html += "alert('Please select a valid time.');";
  html += "}";
  html += "}";

  html += "setInterval(updateSensorData, 900);";
  html += "updateSensorData();";
  html += "</script>";
  html += "</body></html>";
  return html;
}


void saveDataToFirebase() {
  FirebaseJson json;
  json.set("time", dateString);
  json.set("light", light);
  json.set("humidity", humidity);
  json.set("temperature", temperature);

  String path = "/sensorData/" + currentDate;
  if (Firebase.RTDB.pushJSON(&fbdo, path, &json)) {
  } else {
    Serial.println("Error saving data: " + fbdo.errorReason());
  }
}

String getFirebaseData(String date) {
  String path = "/sensorData/" + date;
  QueryFilter query;
  query.orderBy("$key");
  query.limitToLast(1);

  if (Firebase.RTDB.getJSON(&fbdo, path, &query)) {
    if (fbdo.dataType() == "json") {
      FirebaseJson &json = fbdo.jsonObject();
      size_t dataCount = json.iteratorBegin();
      String jsonResponse = "{";
      jsonResponse += "\"data\": ";
      for (size_t i = 0; i < dataCount; i++) {
        String key, value;
        int type;
        json.iteratorGet(i, type, key, value);

        if (value.startsWith("{")) {
          jsonResponse += value;
        }
      }
      json.iteratorEnd();
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

void processTimeParamArray() {
  for (int i = 0; i < timeParamCount; i++) {
    String firebaseData = getFirebaseData(timeParamArray[i]);
    olderDataDictionary[timeParamArray[i]] = firebaseData;
    timeParamArray[i] = "";
  }
  timeParamCount = 0;
}