#include <avr/wdt.h> // watchdog timer
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_MCP9808.h>
#include <RTClib.h> // RTC and SoftRTC
#include <Adafruit_BME280.h>
#include <SD.h>
#include <Adafruit_ADS1015.h>
#include <CCSDS.h>
#include <ccsds_xbee.h>
#include <ccsds_util.h>
#include <SSC.h>

int COMMAND = 0;
uint8_t ReadData;
int COMMAND_PIN = 0;
int teensy_ = 0;
int teensy = 0;
#define COMMAND_PIN 6
#define TEENSY_PIN A4
#define ACTUATOR_PIN_HBRIDGE_A 3 // yellow
#define ACTUATOR_PIN_HBRIDGE_B 4 // green


/* APIDs */
#define RetractActuator 'A'
#define ExtendActuator 'C'
#define case3 'E'
#define case4 'G'
#define case5 'I'
#define case6 'K'

long start_millis;
#define CYCLE_DELAY 100 // time between execution cycles [ms]
bool extended = false;
bool armed = true;

//// Enumerations
// logging flag
#define LOG_RCVD 1
#define LOG_SEND 0


/* response codes */
#define INIT_RESPONSE 0xAC
#define RETRACT_RESPONSE 0xE1

// CAMERA FcnCodes
#define COMMAND_NOOP 0
#define REQUEST_ENVIRONMENTAL_DATA 10
#define REQUEST_POWER_DATA 20
#define REQUEST_IMU_DATA 30


//// Declare objects
Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x29);
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
RTC_DS1307 rtc;
RTC_Millis SoftRTC;    // This is the millis()-based software RTC
Adafruit_BME280 bme;
Adafruit_ADS1015 ads(0x4A);
SSC ssc(0x28, 255);

//// Serial object aliases
// so that the user doesn't have to keep track of which is which
#define debug_serial Serial

//// Data Structures
// imu data
struct IMUData_s
{
        uint8_t system_calibration;
        uint8_t accel_calibration;
        uint8_t gyro_calibration;
        uint8_t mag_calibration;
        float accel_x;
        float accel_y;
        float accel_z;
        float gyro_x;
        float gyro_y;
        float gyro_z;
        float mag_x;
        float mag_y;
        float mag_z;
};
// environmental data
struct ENVData_s
{
        float bme_pres;
        float bme_temp;
        float bme_humid;
        float ssc_pres;
        float ssc_temp;
        float bno_temp;
        float mcp_temp;
        uint8_t teensy;
};
// power data
struct PWRData_s
{
        float batt_volt;
        float i_consump;
};

//// Timing
// timing counters
uint16_t imu_read_ctr = 0;
uint16_t pwr_read_ctr = 0;
uint16_t env_read_ctr = 0;

// rate setting
// sensors will be read every X cycles
uint16_t imu_read_lim = 10;
uint16_t pwr_read_lim = 100;
uint16_t env_read_lim = 100;

// interface counters
uint16_t CmdExeCtr = 0;
uint16_t CmdRejCtr = 0;
uint32_t XbeeRcvdByteCtr = 0;
uint32_t XbeeSentByteCtr = 0;



uint32_t destaddr = 2;
// logging files
File LogFile;
File IMULogFile;
File ENVLogFile;
File PWRLogFile;
File initLogFile;

//// Function prototypes
void command_response(uint8_t data[], uint8_t data_len, struct IMUData_s IMUData, struct ENVData_s ENVData, struct PWRData_s PWRData);

// interface
void send_and_log(uint8_t data[], uint8_t data_len);
void logPkt(File file, uint8_t data[], uint8_t len, uint8_t received_flg);

// pkt creation
uint16_t create_HK_pkt(uint8_t HK_Pkt_Buff[]);
uint16_t create_IMU_pkt(uint8_t HK_Pkt_Buff[], struct IMUData_s IMUData);
uint16_t create_PWR_pkt(uint8_t HK_Pkt_Buff[], struct PWRData_s PWRData);
uint16_t create_ENV_pkt(uint8_t HK_Pkt_Buff[], struct ENVData_s ENVData);
uint16_t create_Status_pkt(uint8_t HK_Pkt_Buff[], uint8_t message);
uint16_t _APID2;

// sensor reading
void read_imu(struct IMUData_s *IMUData);
void read_env(struct ENVData_s *ENVData);
void read_pwr(struct PWRData_s *PWRData);

// log data
void log_imu(struct IMUData_s IMUData, File IMULogFile);
void log_env(struct ENVData_s ENVData, File ENVLogFile);
void log_pwr(struct PWRData_s PWRData, File PWRLogFile);

// utility
void print_time(File file);

//// CODE:
void setup(void)
{
    /* setup()
     *
     * Disables watchdog timer (in case its on)
     * Initalizes all the link hardware/software including:
     *   Serial
     *   RTC
     *   SoftRTC
     *   BNO
     *   MCP
     *   BME
     *   SSC
     *   ADS
     *   SD card
     *   Log files
     */
    // disable the watchdog timer immediately in case it was on because of a
    // commanded reboot
    wdt_disable();

    //// Init serial ports:
    /*  aliases defined above are used to reduce confusion about which serial
     *    is connected to what interface
  
     */
    debug_serial.begin(1200);

    debug_serial.println("GoGoGadget HASP!");

    //// BNO
    if (!bno.begin())
    {
        debug_serial.println("BNO055 NOT detected.");
    }
    else
    {
        debug_serial.println("BNO055 detected!");
    }
    delay(1000);
    bno.setExtCrystalUse(true);

    //// MCP9808
    if (!tempsensor.begin(0x18))
    {
        debug_serial.println("MCP9808 NOT detected.");
    }
    else
    {
        debug_serial.println("MCP9808 detected!");
    }

    //// RTC
    /* The RTC is used so that the log files contain timestamps. If the RTC
     *  is not running (because no battery is inserted) the RTC will be initalized
     *  to the time that this sketch was compiled at.
     */
    if (!rtc.begin())
    {
        debug_serial.println("DS1308 NOT detected.");
    }
    else
    {
        debug_serial.println("DS1308 detected!");
    }

    if (!rtc.isrunning())
    {
        debug_serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    //// SoftRTC (for subsecond precision)
    SoftRTC.begin(rtc.now());    // Initialize SoftRTC to the current time
    start_millis = millis();    // get the current millisecond count

    //// Init BME
    if (!bme.begin(0x76))
    {
        debug_serial.println("BME280 NOT detected.");
    }
    else
    {
        debug_serial.println("BME280 detected!");
    }

    //// Init ADS
    ads.begin();
    ads.setGain(GAIN_ONE);
    debug_serial.println("Initialized ADS1015");

    //// Init SD card
    SPI.begin();
    pinMode(53, OUTPUT);
    if (!SD.begin(53))
    {
        debug_serial.println("SD Card NOT detected.");
    }
    else
    {
        debug_serial.println("SD Card detected!");
    }

    //// Init SSC
    //  set min / max reading and pressure, see datasheet for the values for your
    //  sensor
    ssc.setMinRaw(0);
    ssc.setMaxRaw(16383);
    ssc.setMinPressure(0.0);
    ssc.setMaxPressure(30);

    //  start the sensor
    debug_serial.print("SSC start: ");
    debug_serial.println(ssc.start());

    //MicroSD
    // appends to current file
    // NOTE: Filenames must be shorter than 8 characters
    File IMULogFile = SD.open("IMU_LOG.txt", FILE_WRITE);
    File PWRLogFile = SD.open("PWR_LOG.txt", FILE_WRITE);
    File ENVLogFile = SD.open("ENV_LOG.txt", FILE_WRITE);
    
    pinMode(COMMAND_PIN, INPUT);
    pinMode(ACTUATOR_PIN_HBRIDGE_A, OUTPUT);
    pinMode(ACTUATOR_PIN_HBRIDGE_B, OUTPUT);

    digitalWrite(ACTUATOR_PIN_HBRIDGE_A, LOW);
    digitalWrite(ACTUATOR_PIN_HBRIDGE_B, LOW);

    armed = true;
    delay(1000);
}

void loop(void)
{
    /*  loop()
     *
     *  Reads sensor
     *  Log sensors
     *  Reads from xbee and processes any data
     */


// declare structures to store data
    IMUData_s IMUData;
    PWRData_s PWRData;
    ENVData_s ENVData;

// increment read counters
    imu_read_ctr++;
    pwr_read_ctr++;
    env_read_ctr++;

// read sensors if time between last read
  //  if (imu_read_ctr > imu_read_lim)
   // {
        read_imu(&IMUData);
        log_imu(IMUData, IMULogFile);
        imu_read_ctr = 0;
   // }
 //   if (env_read_ctr > env_read_lim)
  //  {
        read_env(&ENVData);
        log_env(ENVData, ENVLogFile);
        env_read_ctr = 0;
  //  }

// initalize a counter to record how many bytes were read this iteration
    int BytesRead = 0;


        // respond to it
    command_response(IMUData, ENVData, PWRData);

  
    COMMAND = digitalRead(COMMAND_PIN);
    debug_serial.println("looped");
    
    //if (COMMAND == LOW){
     // delay(50);
      //COMMAND = digitalRead(COMMAND_PIN);
        //  if (COMMAND == LOW){
          //retract(25);
          //debug_serial.println("retract");
         // }
    //}


// wait a bit
    delay(10);

}

void command_response(struct IMUData_s IMUData, struct ENVData_s ENVData, struct PWRData_s PWRData)
{
    /*  command_response()
     *
     *  given an array of data (presumably containing a CCSDS packet), check if the
     *  packet is a CAMERA command packet, and if so, process it
     */
    char Cmd;
    if (Serial.available()>0){
      Cmd = Serial.read();

      switch(Cmd){
        case RetractActuator:

           debug_serial.print("Recieved Retract Cmd:  ");
           retract(25);
           debug_serial.println("Retracted");
           break;

        case ExtendActuator:
          debug_serial.print("Recieved Extend Cmd:  ");
          extend(10);
          debug_serial.println("Extended");
          break;
      }
    }

    
    uint16_t pktLength = 0;
    uint8_t Pkt_Buff[100];
    
   read_imu(&IMUData); 
   log_imu(IMUData, IMULogFile);
   read_env(&ENVData);
   log_env(ENVData, ENVLogFile);
   

}

void log_imu(struct IMUData_s IMUData, File IMULogFile)
{
// print the time to the file
    
    print_time(IMULogFile);
// print the sensor values
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.system_calibration);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.accel_calibration);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.gyro_calibration);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.mag_calibration);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.accel_x);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.accel_y);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.accel_z);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.gyro_x);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.gyro_y);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.gyro_z);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.mag_x);
    IMULogFile.print(", ");
    IMULogFile.print(IMUData.mag_y);
    IMULogFile.print(", ");
    IMULogFile.println(IMUData.mag_z);

    IMULogFile.flush();
    debug_serial.println("IMU Data");
    debug_serial.print(", ");
    debug_serial.print(IMUData.system_calibration);
    debug_serial.print(", ");
    debug_serial.print(IMUData.accel_calibration);
    debug_serial.print(", ");
    debug_serial.print(IMUData.gyro_calibration);
    debug_serial.print(", ");
    debug_serial.print(IMUData.mag_calibration);
    debug_serial.print(", ");
    debug_serial.print(IMUData.accel_x);
    debug_serial.print(", ");
    debug_serial.print(IMUData.accel_y);
    debug_serial.print(", ");
    debug_serial.print(IMUData.accel_z);
    debug_serial.print(", ");
    debug_serial.print(IMUData.gyro_x);
    debug_serial.print(", ");
    debug_serial.print(IMUData.gyro_y);
    debug_serial.print(", ");
    debug_serial.print(IMUData.gyro_z);
    debug_serial.print(", ");
    debug_serial.print(IMUData.mag_x);
    debug_serial.print(", ");
    debug_serial.print(IMUData.mag_y);
    debug_serial.print(", ");
    debug_serial.print(IMUData.mag_z);
    debug_serial.println(";");

    debug_serial.flush();
}

void log_env(struct ENVData_s ENVData, File ENVLogFile)
{
    print_time(ENVLogFile);
    teensy_ = analogRead(TEENSY_PIN);
    if (teensy_ == HIGH){
      teensy = 1;
    }
    else if (teensy_ == LOW){
      teensy = 0;
    }
// print the sensor values
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.bme_pres);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.bme_temp);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.bme_humid);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.ssc_pres);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.ssc_temp);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.bno_temp);
    ENVLogFile.write(", ");
    ENVLogFile.write(ENVData.mcp_temp);
    ENVLogFile.write(", ");
    ENVLogFile.write(teensy);
    ENVLogFile.write("/n");

    ENVLogFile.flush();
    debug_serial.println("ENV Data");
    debug_serial.print(", ");
    debug_serial.print(ENVData.bme_pres);
    debug_serial.print(", ");
    debug_serial.print(ENVData.bme_temp);
    debug_serial.print(", ");
    debug_serial.print(ENVData.bme_humid);
    debug_serial.print(", ");
    debug_serial.print(ENVData.ssc_pres);
    debug_serial.print(", ");
    debug_serial.print(ENVData.ssc_temp);
    debug_serial.print(", ");
    debug_serial.print(ENVData.bno_temp);
    debug_serial.print(", ");
    debug_serial.print(ENVData.mcp_temp);
    debug_serial.print(", ");
    debug_serial.print(teensy);
    debug_serial.println(";");

    debug_serial.flush();
}


void read_env(struct ENVData_s *ENVData)
{
//BME280
    ENVData->bme_pres = bme.readPressure() / 100.0F;    // hPa
    ENVData->bme_temp = bme.readTemperature();    // degC
    ENVData->bme_humid = bme.readHumidity();    // %

//  SSC
    /*
     ssc.update();
     ENVData->ssc_pres = ssc.pressure(); // PSI
     ENVData->ssc_temp = ssc.temperature(); // degC
     */
// BNO
    ENVData->bno_temp = bno.getTemp();

//MCP9808
    ENVData->mcp_temp = tempsensor.readTempC();    // degC
}

void read_pwr(struct PWRData_s *PWRData)
{

    PWRData->batt_volt = ((float) ads.readADC_SingleEnded(2)) * 0.002 * 3.0606;    // V
    PWRData->i_consump = (((float) ads.readADC_SingleEnded(3)) * 0.002 - 2.5) * 10;
}

void read_imu(struct IMUData_s *IMUData)
{

    uint8_t system_cal, gyro_cal, accel_cal, mag_cal = 0;
    bno.getCalibration(&system_cal, &gyro_cal, &accel_cal, &mag_cal);

// get measurements
    imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);    // (values in uT, micro Teslas)
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);    // (values in rps, radians per second)
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);    // (values in m/s^2)

// assign them into structure fields
    IMUData->system_calibration = system_cal;
    IMUData->accel_calibration = accel_cal;
    IMUData->gyro_calibration = gyro_cal;
    IMUData->mag_calibration = mag_cal;
    IMUData->accel_x = accel.x();
    IMUData->accel_y = accel.y();
    IMUData->accel_z = accel.z();
    IMUData->gyro_x = gyro.x();
    IMUData->gyro_y = gyro.y();
    IMUData->gyro_z = gyro.z();
    IMUData->mag_x = mag.x();
    IMUData->mag_y = mag.y();
    IMUData->mag_z = mag.z();

}


void print_time(File file)
{
    /*  print_time()
     *
     *  Prints the current time to the given log file
     */

// get the current time from the RTC
    DateTime now = rtc.now();

// print a datestamp to the file
    char buf[50];
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
    file.print(buf);
}

void extend(int pulse_seconds)
{
    controlActuator(1, pulse_seconds);
    extended = true;
}

void retract(int pulse_seconds)
{
    controlActuator(-1, pulse_seconds);
    extended = false;
}

void controlActuator(int direction, int pulse_seconds)
{

// actuator without built-in polarity switch
    if (direction < 0)
    {
        digitalWrite(ACTUATOR_PIN_HBRIDGE_A, LOW);
        digitalWrite(ACTUATOR_PIN_HBRIDGE_B, HIGH);
    }
    else if (direction > 0)
    {
        digitalWrite(ACTUATOR_PIN_HBRIDGE_A, HIGH);
        digitalWrite(ACTUATOR_PIN_HBRIDGE_B, LOW);
    }

    delay(pulse_seconds * 1000);

    digitalWrite(ACTUATOR_PIN_HBRIDGE_A, LOW);
    digitalWrite(ACTUATOR_PIN_HBRIDGE_B, LOW);

}

