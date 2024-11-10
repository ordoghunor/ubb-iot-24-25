#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Mateinfo";
const char* password = "computer";

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
      <p>Choose RGB Color:</p>
      <input type="color" id="colorPicker" class="color-picker" value="#%REDSLIDERVALUE%">
      <script>
        // Set initial color picker value
        document.getElementById('colorPicker').value = "#%REDSLIDERVALUE%";

        // Listen for color changes
        document.getElementById('colorPicker').addEventListener('input', function(e) {
          var color = e.target.value;
          var red = parseInt(color.substring(1, 3), 16);   // Extract red channel
          var green = parseInt(color.substring(3, 5), 16); // Extract green channel
          var blue = parseInt(color.substring(5, 7), 16);  // Extract blue channel

          // Update the server with the new RGB values
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/slider?color=rgb&value=" + red + "," + green + "," + blue, true);
          xhr.send();
        });
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

  if (color == "rgb") {
    int red = value.substring(0, value.indexOf(',')).toInt();
    int green = value.substring(value.indexOf(',') + 1, value.lastIndexOf(',')).toInt();
    int blue = value.substring(value.lastIndexOf(',') + 1).toInt();

    ledcWrite(redPin, red);
    ledcWrite(greenPin, green);
    ledcWrite(bluePin, blue);

    redSliderValue = String(red);
    greenSliderValue = String(green);
    blueSliderValue = String(blue);

    Serial.println("Red: " + redSliderValue + " Green: " + greenSliderValue + " Blue: " + blueSliderValue);
  }

  request->send(200, "text/plain", "OK");
}
