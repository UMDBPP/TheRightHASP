//This code is written for a Raspberry Pi using a pi camera and 4 pin LED
//The code creates a file to log data, tests the line sending data,
// and tests the camera by taking a picture. The images will appear
// in a test_images folder.
//The LED should flash white, red, and yellow respectively if there is an error 
//in any of the three tests.

//LED PINS: r=12;g=16;b=18; on pi

#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <raspicam/raspicam.h>
#include "RS232.h"
#include <wiringPi.h>

using namespace std;

int main(){
  //initializing LED pins (and turning them off)
  wiringPiSetup();
  pinMode(1,OUTPUT);pinMode(4,OUTPUT);pinMode(5,OUTPUT);
  digitalWrite(1,LOW);digitalWrite(4,LOW);digitalWrite(5,LOW);delay(500);

  //open file
  digitalWrite(1,HIGH);digitalWrite(4,HIGH);digitalWrite(5,HIGH);
  ofstream file;
  file.open("full_test.txt", ios::app);
  if(!file.is_open()){
    cerr << "Error: Cannot open file" << endl;

    return -2;
  }
  //open usb line
  digitalWrite(1,HIGH);digitalWrite(4,LOW);digitalWrite(5,LOW);
  BPP::RS232Serial usb;
  int portOpen = usb.portOpen("/dev/ttyACM0", B57600, 8, 'N', 1);
  if(portOpen != 0){
    cerr << "Error: Cannot open USB line" << endl;
    return -1;
  }
  //open camera
  digitalWrite(1,HIGH); digitalWrite(4,HIGH);
  raspicam::RaspiCam Camera;
  if(!Camera.open()){
    cerr << "Error: Cannot open camera" << endl;
    return -3;
  }
  //sleep for 3 sec to let camera init
  digitalWrite(1,LOW);digitalWrite(4,LOW);digitalWrite(5,LOW);
  sleep(3);
  //init variables
  string input = "";
  int x = 0;
  while(x < 25){
    //get data from usb line (every sec)
    int rcvdData = usb.rxData();
    if(rcvdData > 0) {
      input = usb.getData();
      cout << input << endl;
      file << input << endl;
      usb.portFlush();
    }
    file.flush();
    //take a picture (every 5 secs)
    if(x%5 == 0){
      Camera.open();
      unsigned char *data = new unsigned char[Camera.getImageTypeSize(raspicam::
						     RASPICAM_FORMAT_RGB)];
      Camera.grab();
      int i = x/5;
      stringstream ss;
      string str = "test_images/imag_test";
      ss << i;
      str += ss.str() + ".ppm";
      Camera.grab();
      Camera.retrieve(data, raspicam::RASPICAM_FORMAT_RGB);
      cout << "Taking picture " << i << ", iteration " << x << endl;
      std::ofstream output(str.c_str(), std::ios::binary);
      output << "P6\n" << Camera.getWidth() << " " << Camera.getHeight() <<
	" 255\n";
      output.write( (char*) data, Camera.getImageTypeSize(raspicam::
							  RASPICAM_FORMAT_RGB));
      Camera.release();
      delete data;
    }
    usleep(1000000);
    x++;
  }
  cout << "Test Complete!" << endl;
  //close and free
  usb.portClose();
  file.close();
  return 0;
}
