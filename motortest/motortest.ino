void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(22, OUTPUT); //X step
  pinMode(23, OUTPUT); //X dir
  pinMode(12, OUTPUT); //MS0
  pinMode(11, OUTPUT); //MS1
  pinMode(10, OUTPUT); //MS2

  byte i;

  digitalWrite(23, LOW);
  delay(1);
  for (i = 0; i < 56; i++){
    digitalWrite(22, HIGH);
    delay(80);
    digitalWrite(22, LOW);
    delay(80);
  }
  delay(1000);
  digitalWrite(23, HIGH);
  delay(1);
  for (i = 0; i < 56; i++){
    digitalWrite(22, HIGH);
    delay(80);
    digitalWrite(22, LOW);
    delay(80);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
