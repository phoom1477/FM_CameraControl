#include <Servo.h>
Servo myservo;
int servoPWM = 108;

void setup(){
  Serial.begin(115200);
  myservo.attach(9);
  myservo.write(servoPWM);
}
void loop(){
  String command = Serial.readStringUntil('\n');

  if (command == "left") {
    while (servoPWM < 162) {
      myservo.write(servoPWM);
      delay(10);
      servoPWM += 1;
    }
    Serial.println("turned");
  }

  if (command == "center") {
    while (servoPWM > 108) {
      myservo.write(servoPWM);
      delay(10);
      servoPWM -= 1;
    }
    while (servoPWM < 107) {
      myservo.write(servoPWM);
      delay(10);
      servoPWM += 1;
    }
    Serial.println("turned");
  }

  if (command == "right") {
    while (servoPWM > 51){
      myservo.write(servoPWM);
      delay(10);
      servoPWM -= 1;
    }
    Serial.println("turned");
  }
}
