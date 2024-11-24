#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "Orange-vS2t-2.4G";
const char* password = "n23TVMsE";

// MQTT broker details
const char* mqtt_server = "192.168.100.40";
const int mqtt_port = 1883;
const char* client_id = "ESP32_Client";
const char* will_topic = "istvan/potmeter/status";
const char* will_message = "disconnected";

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

// GPIO Pins
const int potPin = 0;  // Potentiometer pin
const int redPin = 15;  // Red LED pin
const int greenPin = 22;  // Green LED pin
const int bluePin = 23;  // Blue LED pin

// Topic names
const char* potmeter_topic = "istvan/potmeter";
const char* live_channels_topic = "channels/active";

// State variables
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id, will_topic, 1, true, will_message)) {
      Serial.println("connected");
      client.publish(will_topic, "connected", true);
      client.subscribe("#"); // Subscribe to all topics
      client.publish(live_channels_topic, potmeter_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleRoot() {
   String html = "<!DOCTYPE html><html>";
  html += "<h1>ESP32 MQTT Control</h1>";
  html += "<label for='red'>Red:</label><input type='range' id='red' name='red' min='0' max='255' value='" + String(redValue) + "'><br>";
  html += "<label for='green'>Green:</label><input type='range' id='green' name='green' min='0' max='255' value='" + String(greenValue) + "'><br>";
  html += "<label for='blue'>Blue:</label><input type='range' id='blue' name='blue' min='0' max='255' value='" + String(blueValue) + "'><br>";
  html += "<label for='wisdom'>Sajat bolcsessegeim:</label><input type='text' id='wisdom' name='wisdom'><br>";
  html += "<button onclick='submitData()'>Submit</button>";
  html += "<script>function submitData() {";
  html += "let red = document.getElementById('red').value;";
  html += "let green = document.getElementById('green').value;";
  html += "let blue = document.getElementById('blue').value;";
  html += "let wisdom = document.getElementById('wisdom').value;";
  html += "fetch('/', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({red, green, blue, wisdom})})";
  html += ".then(res => res.text()).then(data => console.log(data));}";
  html += "</script></html>";
  server.send(200, "text/html", html);
}

void handlePost() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    redValue = doc["red"].as<int>();
    greenValue = doc["green"].as<int>();
    blueValue = doc["blue"].as<int>();
    String wisdom = doc["wisdom"].as<String>();

    analogWrite(redPin, redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin, blueValue);

    client.publish("wisdom/channel", wisdom.c_str());
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = String((char*)payload);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handlePost);
  server.begin();
  Serial.println("HTTP server started");

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  server.handleClient();

  int potValue = analogRead(potPin);
  client.publish(potmeter_topic, String(potValue).c_str());
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);
}
