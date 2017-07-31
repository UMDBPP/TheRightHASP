int i;
int led = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  i = 0;
  pinMode(led, OUTPUT); 

}

void loop() {
  Serial.print("Data Line ");
  Serial.println(i);
  i++;
  digitalWrite(led, HIGH);
  delay(50);
  digitalWrite(led, LOW);
}
