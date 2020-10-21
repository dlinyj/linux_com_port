void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
     Serial.print("01234567890123456789\n\r");
     delay(2000);
}
