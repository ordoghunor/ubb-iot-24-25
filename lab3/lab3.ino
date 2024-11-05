
int redPin = 16;     // Red RGB pin -> D3
int greenPin = 22;   // Green RGB pin -> D5
int bluePin = 23;    // Blue RGB pin -> D6

void setup() {
  pinMode(redPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  pinMode(greenPin, OUTPUT);
  
}

void loop() {
for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    analogWrite(redPin, dutyCycle);
    analogWrite(greenPin, dutyCycle);
    analogWrite(bluePin, dutyCycle);
    delay(15);
  }

  // decrease the LED brightness
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    analogWrite(redPin, dutyCycle);
    analogWrite(greenPin, dutyCycle);
    analogWrite(bluePin, dutyCycle);
    delay(15);
  }
  
  delay(10);
}