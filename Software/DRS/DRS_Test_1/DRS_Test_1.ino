#include <SPI.h>
int busypin = 16; //busy input pin from ADAS3022 adc
int BUSY; //busy variable indicator for ADAS3022 adc
int convertpin = 15; //convert output pin to tell ADAS3022 to begin analog-to-digital conversion
int CNV;
int PD; //power down pin for ADAS3022 adc
int ss=10; //chip select pin for spi
float voltage; 
int led = 2; //pin with led on it
int droppin = A5; //Analog pin output for detecting if we dropped or not. 1 if we did not yet, 0 if we have
//int dropcheck = 0;
int receivedval16;
void setup() {
  // put your setup code here, to run once:
 Serial.begin(57600);
 pinMode(ss, OUTPUT); //set chip select pin as output for spi library
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);  //set up spi setting for most significant bit first
  pinMode(led, OUTPUT); //set up led pin to be an output
  pinMode(droppin, OUTPUT); //set up A5 to be the analog output pin going to Balloonduino for drop signal
  pinMode(busypin, INPUT); //set up digital pin 16 to be the input pin for knowing ADC is busy
  pinMode(convertpin, OUTPUT); //set up digital pin 15 to be the output pin for telling the ADC to convert

} 
void loop() {
  // put your main code here, to run repeatedly:
BUSY = digitalRead(busypin);
  if(BUSY!= 1){ //If the ADC is not busy, convert the data
    digitalWrite(convertpin, HIGH); //set the convert pin high to command the ADC to begin converting
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE2)); //Set up adc spi settings
    receivedval16=SPI.transfer16(0x854F); //put what the adc command is here (using 5.12 V pgia gain, differential input)
    voltage= (((float)receivedval16/32768.0)*20.480);  //convert from raw data obtained to a voltage ??? may be wrong
    Serial.print("Voltage = ");
    Serial.print(voltage, DEC); //print out the voltage through serial
    Serial.println("V");
    SPI.endTransaction();
  }

  //see if still connected to Raspberry pi or not
  if(Serial.available()){ //check to see if there is anything ready to be read from Raspberry pi
    
    digitalWrite(droppin, HIGH);  //send a high analog signal if still connected to Pi
    digitalWrite(led, HIGH); //Make led light turn off if we are still connected to Pi
  }
  else{
  digitalWrite(droppin, LOW); //send a low analog signal if disconnected from Pi
  digitalWrite(led, HIGH); //Make led light stay on if we are still connected
  } 
  
  delay(50); 
}
