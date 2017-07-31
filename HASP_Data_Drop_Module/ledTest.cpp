//This code is written for a Raspberry Pi to test a 4-pin LED
//The LED should flash red, green, blue, yellow, light blue,
// purple, and white. Colors may vary.
//LED PINS: r=12;g=16;b=18;

#include <wiringPi.h>

#include <iostream>
using namespace std;

int main(void)
{
  int pin[3]={1,4,5};
  wiringPiSetup();
  for (int i=0; i<3; i++)
  {
    pinMode(pin[i],OUTPUT);
    digitalWrite(pin[i],HIGH); delay(500);
    digitalWrite(pin[i], LOW); delay(500);
  }
  digitalWrite(1,HIGH); digitalWrite(4,HIGH); delay(500);
  digitalWrite(1,LOW) ; digitalWrite(4,LOW) ; delay(500);
  digitalWrite(4,HIGH); digitalWrite(5,HIGH); delay(500);
  digitalWrite(4,LOW) ; digitalWrite(5,LOW) ; delay(500);
  digitalWrite(5,HIGH); digitalWrite(1,HIGH); delay(500);
  digitalWrite(5,LOW) ; digitalWrite(1,LOW) ; delay(500);

  digitalWrite(1,HIGH); digitalWrite(4,HIGH); digitalWrite(5,HIGH); delay(500);
  digitalWrite(1,LOW) ; digitalWrite(4,LOW) ; digitalWrite(5,LOW) ; delay(500);
  return 0;
}
