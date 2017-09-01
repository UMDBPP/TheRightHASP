//This code is written for a Raspberry Pi using a pi camera and 4 pin LED
//The code creates a file to log data, tests the line sending data,
// and takes pictures in minute intervals. The images will appear
// in the Pictures folder.
//The LED should be a solid blue if there is a file creation issue
// The LED will blink green if the program is working as intended.
// The LED will blink red if the camera has malfunctioned but the program still runs.

//LED PINS: r=12;g=16;b=18; on pi 3

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
int cam = 4;	    // 4 is working; 1 is broken
int t = 3;	    // time for LED to blink
int camTime = 60;   // time between pictures

#define working 4
#define blueled 5
#define redled 4
#define greenled 1
#define broken 1

int main() {
  // GPIO pin setup for indicator LED
  wiringPiSetup();
  pinMode(greenled,OUTPUT); pinMode(redled,OUTPUT); pinMode(blueled,OUTPUT);
  digitalWrite(greenled,LOW);digitalWrite(redled,LOW);digitalWrite(blueled,LOW);delay(500);

  // File setup
  ofstream file;
  file.open("/home/pi/Desktop/flight_data.txt", ios::app);
  if(!file.is_open())
  {
    digitalWrite(greenled,LOW);digitalWrite(redled,LOW);digitalWrite(blueled,HIGH);
    cerr << "File Error: Try deleting the data file on Desktop." << endl;
    return -2;
  }

  // Camera setup
  //raspicam::RaspiCam Camera;
  //if(!Camera.open())
  //{
   // cerr << "Camera Error: check pi cam connection." << endl;
    //cam = broken;
  //}
  //sleep(3);
  //Camera.release();

  // USB connection setup
  BPP::RS232Serial usb;

  // Waits for the USB line to be plugged in.
  while(int portOpen = usb.portOpen("/dev/ttyACM0", B57600, 8, 'N', 1))
  {
    digitalWrite(greenled,HIGH);digitalWrite(redled,LOW);digitalWrite(blueled,LOW);
    cerr << "Connection Error: check that the USB cable is plugged in." << endl;
    delay(10000);
    usb.portClose();
  }

  // LED blinks for 10 seconds to signify that it is ready to go.
  // blinks red if there is a camera error; green if all is working
  digitalWrite(greenled,LOW);digitalWrite(redled,LOW);digitalWrite(blueled,LOW);
  for (int i=0; i<t; i++)
    {
      digitalWrite(cam,HIGH);
      delay(500);
      digitalWrite(cam,LOW);
      delay(500);
    }

  //init variables
  string input = "";
  int x = 0;

  for(;;) {

    digitalWrite(blueled, HIGH);
    //get data from usb line (every sec)

    int rcvdData = usb.rxData();

    cout << "data";
    input = usb.getData();
      cout << input << endl;
      file << input << endl;
      usb.portFlush();
      file.flush();
      cout.flush();
    
      file.flush();
      cout << "here";
/*
    //take a picture
    if (x%camTime == 0 && cam == working)
    {
      //if(!Camera.open())
     // {
      //  cam = broken;
     // }
      if(cam == working)
      {
        unsigned char *data = new unsigned char[Camera.getImageTypeSize(raspicam::
						     RASPICAM_FORMAT_RGB)];
        cout << "1";
  
        //Camera.grab();

        // Should produce an integer from truncation
        int i = x/camTime;

        cout << "2";
        stringstream ss;

        cout << "3";
        string str = "/home/pi/Desktop/Pictures/imag_";

        cout << "4";
        ss << i;

        cout << "5";
        str += ss.str() + ".ppm";

        cout << "6";
        Camera.grab();

        cout << "7";
        data = Camera.getImageBufferData();

        cout << "8";
        cout << "taking picture " << i << ", iteration " << x << endl;

        cout << "9";
        std::ofstream output(str.c_str(), std::ios::binary);

        cout << "0";
        output << "P6\n" << Camera.getWidth() << " " << Camera.getHeight() <<

        cout << "10";
	" 255\n";

        cout << "11";
        output.write( (char*) data, Camera.getImageTypeSize(raspicam::
							  RASPICAM_FORMAT_RGB));

        cout << "12";
        //Camera.release();

        delete data;
        cout << "13";
      }
    }
   usleep(1000000);
    x++;
*/
  }
  cout << "Test Complete!" << endl;
  //close and free
  usb.portClose();
  file.close();
  digitalWrite(redled, HIGH); digitalWrite(blueled, LOW);
  return 0;
}
