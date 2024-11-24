#include <WiFi.h>
#include <PubSubClient.h>

// WiFi Credentials
const char* ssid = "Nokia X30 5G Szadzse";
const char* password = "nincskod";

// MQTT Broker
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

// Pin definitions
const int ledPin = 16;          // LED PWM kimenet
const int potPin = 34;         // Potméter analóg bemenet

long lastMsg = 0;              // Üzenetküldés időzítése
char msg[50];                  // MQTT üzenet buffer
int potValue = 0;              // Potméter érték

void setup() {
  Serial.begin(115200);

  // WiFi beállítása
  setup_wifi();

  // MQTT kliens konfigurálása
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Pin módok beállítása
  pinMode(ledPin, OUTPUT);

  // LED alaphelyzetbe állítása
  analogWrite(ledPin, 0);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // LED fényerő beállítása az MQTT üzenet alapján
  if (String(topic) == "led/control") {
    int brightness = messageTemp.toInt();
    if (brightness >= 0 && brightness <= 255) {
      analogWrite(ledPin, brightness);
      Serial.print("Set LED brightness to: ");
      Serial.println(brightness);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("led/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Potméter érték olvasása
  int newPotValue = analogRead(potPin);
  newPotValue = map(newPotValue, 0, 4095, 0, 255);

  // Csak akkor küldünk új értéket, ha az változott
  if (newPotValue != potValue) {
    potValue = newPotValue;

    // LED fényerő frissítése
    analogWrite(ledPin, potValue);

    // Üzenet küldése MQTT-n
    char valueString[8];
    itoa(potValue, valueString, 10);
    client.publish("led/potmeter", valueString);

    Serial.print("Potentiometer value sent: ");
    Serial.println(potValue);
  }

  delay(100); // Kis késleltetés az értékek stabilitása érdekében
}