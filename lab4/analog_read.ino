
int value;

void setup() 
{

//pinMode(4,INPUT);
Serial.begin(9600);
}

void loop() 
{
value = analogRead(4);
value = value>>4;
Serial.println(value);
delay(100);
}
