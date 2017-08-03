#include <iostream>
#include <wiringPi.h>

using namespace std;
int main()
{
  int pin[3]={1,4,5};
  wiringPiSetup();
  for (int i=0; i<3; i++)
  {
    pinMode(pin[i],OUTPUT);
    digitalWrite(pin[i], LOW);
  }
  return 0;
}
