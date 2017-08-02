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
  //gpio pin setup for indicator LED
  wiringPiSetup();
  pinMode(1,OUTPUT); pinMode(4,OUTPUT); pinMode(5,OUTPUT);
  //open file
  ofstream file;
  file.open("flight_data.txt", ios::app);
  if(!file.is_open()){
    cerr << "Error opening file" << endl;
    return -2;
  }
  //open usb line
  BPP::RS232Serial usb;
  int portOpen = usb.portOpen("/dev/ttyACM0", B57600, 8, 'N', 1);
  while(portOpen != 0){
    cerr << "Error opening usb line" << endl;
    digitalWrite(1,HIGH);
    usb.portOpen("/dev/ttyACM0", B57600, 8, 'N', 1);
    return -1;
  }
  //open camera
  raspicam::RaspiCam Camera;
  if(!Camera.open()){
    cerr << "Error Opening camera" << endl;
    return -3;
  }
  //sleep for 3 sec to let camera init
  sleep(3);
  //init variables
  string input = "";
  int x = 0;
  while(1){
    //get data from usb line (every sec)
    int rcvdData = usb.rxData();
    if(rcvdData > 0) {
      input = usb.getData();
      cout << input << endl;
      file << input << endl;
      usb.portFlush();
    }
    file.flush();
    //take a picture (every minute)
    if(x%60 == 0){
      Camera.open();
      unsigned char *data = new unsigned char[Camera.getImageTypeSize(raspicam::
						     RASPICAM_FORMAT_RGB)];
      Camera.grab();
      int i = x/60;
      stringstream ss;
      string str = "Pictures/imag_";
      ss << i;
      str += ss.str() + ".ppm";
      Camera.grab();
      Camera.retrieve(data, raspicam::RASPICAM_FORMAT_RGB);
      cout << "taking picture " << i << ", iteration " << x << endl;
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
