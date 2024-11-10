#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "TP-Link_871C";
const char* password = "almakorte";

int redPin = 16;     // Red RGB pin -> D3
int greenPin = 22;   // Green RGB pin -> D5
int bluePin = 23;    // Blue RGB pin -> D6

String redSliderValue = "0";
String greenSliderValue = "0";
String blueSliderValue = "0";

// setting PWM properties
const int freq = 5000;
const int ledChannelRed = 0;
const int ledChannelGreen = 1;
const int ledChannelBlue = 2;
const int resolution = 8;

const char* PARAM_INPUT = "value";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP Web Server</title>
      <style>
        html {font-family: Arial; display: inline-block; text-align: center;}
        h2 {font-size: 2.3rem;}
        p {font-size: 1.9rem;}
        body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
        .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
          outline: none; -webkit-transition: .2s; transition: opacity .2s;}
        .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
        .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
      </style>
    </head>
    <body>
      <h2>ESP Web Server</h2>
      <p>Red: <span id="textSliderRed">%REDSLIDERVALUE%</span></p>
      <p><input type="range" onchange="updateSliderRed(this)" id="pwmSliderRed" min="0" max="255" value="%REDSLIDERVALUE%" step="1" class="slider"></p>
      <p>Green: <span id="textSliderGreen">%GREENSLIDERVALUE%</span></p>
      <p><input type="range" onchange="updateSliderGreen(this)" id="pwmSliderGreen" min="0" max="255" value="%GREENSLIDERVALUE%" step="1" class="slider"></p>
      <p>Blue: <span id="textSliderBlue">%BLUESLIDERVALUE%</span></p>
      <p><input type="range" onchange="updateSliderBlue(this)" id="pwmSliderBlue" min="0" max="255" value="%BLUESLIDERVALUE%" step="1" class="slider"></p>
    <script>
    function updateSliderRed(element) {
      var sliderValue = document.getElementById("pwmSliderRed").value;
      document.getElementById("textSliderRed").innerHTML = sliderValue;
      console.log(sliderValue);
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/slider?color=red&value="+sliderValue, true);
      xhr.send();
    }
    function updateSliderGreen(element) {
      var sliderValue = document.getElementById("pwmSliderGreen").value;
      document.getElementById("textSliderGreen").innerHTML = sliderValue;
      console.log(sliderValue);
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/slider?color=green&value="+sliderValue, true);
      xhr.send();
    }
    function updateSliderBlue(element) {
      var sliderValue = document.getElementById("pwmSliderBlue").value;
      document.getElementById("textSliderBlue").innerHTML = sliderValue;
      console.log(sliderValue);
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/slider?color=blue&value="+sliderValue, true);
      xhr.send();
    }
    </script>
    </body>
  </html>
)rawliteral";



void setup(){
  Serial.begin(9600);
  
  // configure LED PWM functionalitites
  ledcAttachChannel(redPin, freq, resolution, ledChannelRed);
  ledcWrite(ledChannelRed, redSliderValue.toInt());

  ledcAttachChannel(greenPin, freq, resolution, ledChannelGreen);
  ledcWrite(ledChannelGreen, greenSliderValue.toInt());

  ledcAttachChannel(bluePin, freq, resolution, ledChannelBlue);
  ledcWrite(ledChannelBlue, blueSliderValue.toInt());

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/slider", HTTP_GET, handleSliderChange);

  server.begin();
}
  
void loop() {
  
}


String processor(const String& var){
  if (var == "REDSLIDERVALUE"){
    return redSliderValue;
  }
  if (var == "GREENSLIDERVALUE"){
    return greenSliderValue;
  }
  if (var == "BLUESLIDERVALUE"){
    return blueSliderValue;
  }
  return String();
}

void handleSliderChange(AsyncWebServerRequest *request) {
  String color = request->getParam("color")->value();
  String value = request->getParam("value")->value();

  if (color == "red") {
    redSliderValue = value;
    ledcWrite(redPin, redSliderValue.toInt());
  } else if (color == "green") {
    greenSliderValue = value;
    ledcWrite(greenPin, greenSliderValue.toInt());
  } else if (color == "blue") {
    blueSliderValue = value;
    ledcWrite(bluePin, blueSliderValue.toInt());
  }

  Serial.println("Red: " + redSliderValue + " Green: " + greenSliderValue + " Blue: " + blueSliderValue);
  request->send(200, "text/plain", "OK");
}
