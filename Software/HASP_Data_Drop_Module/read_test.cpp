#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include "RS232.h"
using namespace std;
int main(){
  //open file
  ofstream file;
  file.open("flight_data.txt", ios::app);
  //setup USB
  BPP::RS232Serial usb;
  int portOpen = usb.portOpen("/dev/ttyACM0", B57600, 8, 'N', 1);
  if(portOpen !=0){
    cerr << "Error opening USB" << endl;
    return 01;
  }
  string input = "";
  int x = 0;
  while(x < 7200){
    if(file.is_open()){
      int rcvdData = usb.rxData();
      //get data
      if(rcvdData > 0){
	input = usb.getData();
	printf(input.c_str());
	file << input << endl;
	usb.portFlush();
	x++;
	  }
      file.flush();
      usleep(1000000);
    }
  }
  usb.portClose();
  file.close();
  return 0;
}