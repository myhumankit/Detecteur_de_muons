#include <Arduino.h>
/*  CosmicSail - Muon detector   Release 1.221  (CF : relnum below)                           
  Last modification by YLC on May 253, 2023 (Reading the voltage sent by the DAC by measuring it with the ADS1015 and no more use of the second potentiometer)
  RTOS version : 2 tasks in // in the 2 cores with maximum priority for reading SiPM measurements in core 0 
                 with detection triggered by interrupt on pin 0 of the ESP32 and shared data protected by mutex.
               
  Muon detector Arduino Code  Using ESP32, 128x64 OLed screen, SD card, 3231 RTC, M8M GPS, ADS1015, BME280, DS18B20, BCD switch
  Program derived from "CosmicWatch Muons detector" project by Spencer Axani and adapted then improved for "CosmicSail 
  Astrolabe Expedition" by Yves Le Chevalier for coding and Christian Fromentin for the design of the electronic part, at My Human Kit.

  => This code is used to record data to the microSD card and display measures and data on Oled screen.
     It only uses one or two scintillators for its detections. (two scintillators for coincidence feature).
  NB : The size of the program exceed Arduino nano capacity Flash RAM or/and SRAM => and we use an ESP32 board 
       (Espressif ESP32 WROOM 32  also named "ESP32 Dev Module" for compilation under Arduino IDE 2)

  A new file is created each day with a name consisting of an "F" prefix followed by the current date.
  During all the day, the file is closed then reopened each hour to ensure its proper functioning.
  When reopening, records are appended to the file until the end of the hour, and so on...

  The RTC DS3132 may drift in time a very little bit. This is automatically checked and corrected if the GPS is picked up.
  Otherwise, to correct this drift, the push button set seconds on rtc to "30".  
  Just press it to synchronize the the displayed time with an external reference when this one reach 30 for seconds.
  This button is operational only when the Oled switch display the UTC date and time and when the GPS is or has been
  picked up for less than 7 days.

  All displays except alert or alarm messages are automatically turned off after 15 minutes of display. 
  You must then change the position switch to restore a display.

  All the SD card used must contain a file named "PARAM" which contains the following parameters: :
  detector name, date of implementation, name of the expedition or mission, name of the boarding ship.
  Each parameter is identified by a record code P1, P2, P3, P4.
  
+++> See the detailed documentation for more information.

  ESP32 Pinout :
          External comparator output pin 0       (digital input)   Trigger an interrupt
          Power SiMP (PWSIMP cmd)    pin 2       (analog output)   Powers up the SiPM
          Power led  (PWM cmd)       pin 4       (analog output)
          Sd card  (CS)              pin 5       (SPI) 
          I/O to drop input signal   pin 12      (digital output)  
          Signal for oscilloscope    pin 13      (digital output)   To view the timing of the measurements
          Commutateur BCD  (pin 1)   pin 14      (digital input)  activates bit1 (value 0001) = 1
          Commutateur BCD  (pin 2)   pin 15      (digital input)  activates bit2 (value 0010) = 2
          M8M GPS   (RXD)            pin 16      (UART2 serial port) For communication with GPS
          M8M GPS   (TXD)            pin 17      (UART2 serial port) For communication with GPS
          Sd card  (CLK)             pin 18      (SPI)
          Sd card  (MISO)            pin 19      (SPI) 
          ADS1015 (SDA)              pin 21      (I2C  addr=0x48)
          Oled Screen (SDA)          pin 21      (I2C  addr=0x3C)
          RTC DS3231  (SDA)          pin 21      (I2C addr=0x68)
          BME280      (SDA)          pin 21      (I2C addr=0x76)
          ADS1015 (SCL)              pin 22      (I2C  addr=0x48)
          Oled Screen (SCL)          pin 22      (I2C  addr=0x3C)
          RTC DS3231  (SCL)          pin 22      (I2C addr=0x68)
          BME280   (SCL)             pin 22      (I2C addr=0x76)
          Sd card  (MOSI)            pin 23      (SPI) 
          DAC output                 pin 25      (digital to analog output) Send a voltage corresponding to the background noise        
          Alarm red led              pin 26      (analog output)
          DS18B20 scintillator temp  pin 27      One wire input (whith external pullup through a 4.7kΩ resistor)
          Button (multipurpose)      pin 32      (digital input) PULLUP  Adjust RTC timer, close file, or ... 
          Commutateur BCD  (pin 3)   pin 33      (digital input)  activates bits 1 & 2 (value 0011) = 3
          Commutateur BCD  (pin 4)   pin 34      (digital input)  activates bit 4 (value 0100) = 4
          Coincicence switch         pin 35      coincidence detection flag (with external pullup)

  ADS1015 pinout :
        A0   Noise (electronically smoothed) for DAC calculation
        A1   Voltage given by SiPM (upon detection)
        A2   Voltage really send by the DAC to the comparator (because theoretical calculation of the voltage of the DAC according to the value sent is inaccurate)
        A3   Detection threshold adjusted by potentiometer
        
  BCD Switch :
        Pos0  no screen displayed  
        Pos1  Screen 1 = Display count and saved variables on last muon detection  
        Pos2  Screen 2 = Display UTC Date & Time
        Pos3  Screen 3 = Display SD and GPS data 
        Pos4  Screen 4 = Display the parameters of the parameter file
        Pos6  Screen 6 = List GPS frames on serial monitor
        pos9  Screen 9 = Display Instant detection variables and others data (test mode)    
       
*/

#include <U8g2lib.h>           // lib from oliver Kraus  v 2.33.15
U8G2_SH1106_128X64_NONAME_F_HW_I2C  u8g2(U8G2_R2/*, reset= U8X8_PIN_NONE */);
// screens list : https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// fonts list : https://github.com/olikraus/u8g2/wiki/fntlistall
#include "RTClib.h"            // lib from Adafruit  RTClib v 2.1.1
RTC_DS3231 myrtc;              // instance for the RTC clock
#include <SPI.h>               // lib from Adafruit  SPI v 2.0.0
#include "FS.h"                // lib from Adafruit  FS v 2.0.0
#include <SD.h>                // lib from Adafruit  SD v 2.0.0
File myFile;                   // instance for the file 
#include <Preferences.h>       // lib from Volodymyr Shymanskyy  Preferences v 2.0.0
Preferences detectorname;      // instance for the Preferences
#include <TinyGPS++.h>         // lib from Mikal Hart  TinyGPS++ v 1.0.3
TinyGPSPlus gps;               // instance for the GPS
#include <HardwareSerial.h>    // standard library
HardwareSerial SerialGPS(1);   // Hardware serial port #2
#include <Adafruit_BME280.h>   // lib from Adafruit  BME280 v 2.2.2 
Adafruit_BME280 BME280;        // instance for the BME280 
#include <ADS1X15.h>           // lib from Rob Tillaart  ADS1X15 v 0.3.9 
ADS1015 ADS(0x48);             // instance for the ADS1015 
#include "OneWire.h"           // lib from Jim Studt +...  OneWire v 2.3.7 
#include "DallasTemperature.h" //  lib from Mile Burton  DallasTemperature v 3.9.0 

// pins assignement :
#define COMPAR  0        // External Comparator input (interruption)
#define POWSIMP 2        // Powers up the SiPM using an optocoupler
#define LEDPOW  4        // Power led
#define SDSSPIN 5        // SD card CS/SS pin
#define EMPTCAP 12       // relay command to make the SiPM signal drop faster after measurements
#define OSCILLO 13       // ouput for oscilloscope
#define BCDSW4  14       // BCD switch pin 1 (with external pull-down)
#define BCDSW3  15       // BCD switch pin 1 (with external pull-down)
#define RXD2    16       // RXD serial port 2 for communication with GPS
#define TXD2    17       // TXD serial port 2 for communication with GPS
#define DAC     25       // DAC output sending a voltage corresponding to the background noise
#define LEDAL   26       // Alarm led (red)
#define SCITMP  27       // Scintillator temperature
#define BUTTON  32       // Button to adjust RTC timer or close file or ...
#define BCDSW2  33       // BCD switch pin 3 (with external pull-down)
#define BCDSW1  34       // BCD switch pin 4 (with external pull-down)
#define COINCI  35       // coincidence detection flag

OneWire oneWire(SCITMP); // setup oneWire instance
DallasTemperature DS18B20(&oneWire);

// initialize global variables
const String relnum = "Release 1.22";       // release number of the actual code
int powbrgt = 1;                            // Brightness of power led
float det_threshold;                        // default minimum detection threshold (in magnitude)
String detname;                             // Name of the detector read in Preferences
unsigned long timoldaff = 0L;               // timestamp of last Oled display
unsigned long timlastop = 0L;               // timestamp of last open file
char timestamp[20] = "";                    // String for record the formatted timestamp
unsigned long deadtime = 0L;                // total time between signals (millisec)
unsigned long deadaff = 0L;                 // total time displayed on screen (millis or minutes)
unsigned long deadtime_t0 = 0L;             // start time for dead time
long count = 0L;                            // A tally of the number of muon observed
long errcnt = 0L;                           // data access conflict count
unsigned int vadc0 = 0;                     // Voltage read on SiPM (mv)
unsigned int vadc = 0;                      // Memorized voltage read on SiPM (mv)
char filename[] = "/Fyymmdd.txt";           // filename created with today's date
String messerr = "";                        // error message for oled display
boolean aledstat = false;                   // error alarm led blinking indicator
int sdremain = 100;                         // % remaining space on SD card
double sdused = 0;                          // space used on SD card
boolean capalert = false;                   // alert indicator for remaining space on SD card
double sdcapacity = 0;                      // total capacity on SD card
double sdrealcap = 0;                       // available capacity at opening SD card
String recdata = "";                        // record on SD card
byte second = 30;                           // Value to adjust rtc time on top"30"
boolean dispok = false;                     // indicator for displaying on Oled screen
boolean oldisp = true;                      // old indicator for displaying on Oled screen
unsigned long timbou = 0;                   // timer to prevent button bounce
DateTime now;                               // actual date & time from RTC
char datew[12];                             // date formatting area
char timew[10];                             // time formatting area
int dayrec = 0;                             // day beeing recorded on SD
String longitude = "";                      // last measured longitude
String latitude = "";                       // last measured latitude
String memolongi = "";                      // last memorized longitude
String memolati = "";                       // last memorized latitude
int yeargps = -1;                           // gps data when functionnal
int monthgps = -1;                          // gps data when functionnal
int daygps = -1;                            // gps data when functionnal
int hourgps = -1;                           // gps data when functionnal
int minugps = -1;                           // gps data when functionnal
int secogps = -1;                           // gps data when functionnal
int atmopres = 0;                           // atmospheric pressure measured by BME280 expressed in hPa
float temperBME = 0;                        // temperature of the BME280 detector in degrees celsius
float temperSiPM = 0;                       // temperature of the SiPM detector in degrees celsius
float temper = 0;                           // average temperature of the 2 detectors in degrees celsius
boolean detect = false;                     // detection indicator
int noisea = 0;                             // sum of noise measures during last second (in magnitude not in volts)
int noiseb = 0;                             // sum of noise measures during previous second (in magnitude not in volts)
int nbnoisa = 0;                            // number of noise detections during last second
int nbnoisb = 0;                            // number of noise detections during previous second
int noise = 0;                              // Average noise during 2 seconds (in mv)
int noise1 = 0;                             // Average noise during 2 seconds (in magnitude)
boolean setdtrtc = false;                   // first date & time RTC setting indicator
float valdac = 0;                           // pseudo noise threshold in volts
int valdav = 0;                             // DAC threshold converted to mv.
int detfilter = 0;                          // low threshold detection to eliminate parasitics detections 
int nbelim = 0;                             // number of detections eliminated
boolean testmode = false;                   // tests indicator
char crapfile;                              // file creation or append indicator
unsigned long gpstime = 0L;                 // memorize last GPS update
unsigned int nbsat =0;                      // number of satellites
int bitr[4];                                // digits read on each 4 pins from BCD switch
int bcdvalue;                               // equivalent in decimal read on BCD switch
float coefx = 0;                            // noise correction multiplier (in magnitude 0 to 1351 = 2047/5*3,3)
float xcoefx = 0;                           // multiplication factor converted to coefficient 
boolean coincid = false;                    // coincidence detection flag 
String shipname = "";                       // Ship name from SD parameters (21 car max)
String impldat = "";                        // Implementation date (JJ/MM/AAAA)
String sponsor = "";                        // Sponsor/Donor name (21 car max)
int nbfiles = 0 ;                           // number of files on SD card
unsigned long timdisp = 0;                  // display start time (for automatic switch-off)
unsigned long timsync = 0;                  // last time check by gps
unsigned long timnois1 = 0L;                // timestamp 1 to wait for noise stabilization at startup
unsigned long timnois2 = 0L;                // timestamp 2 to wait for noise stabilization at startup
boolean dispend = false ;                   // indicator end of display over 15 minutes  
int swstat = 0 ;                            // switch state (position of the BCD switch)
char updatim[20] = "";                      // RTC updated time
boolean RTCbatempty = false ;               // indicator RTC battery is empty 
boolean affbat = false ;                    // indicator for blinking low RTC battery 

// initialize shared variables
int shvaldav = 0;                           // pseudo noise threshold converted to mv.
String shlatitude = "";                     // last measured latitude
String shlongitude = "";                    // last measured longitude
int shatmopres = 0;                         // atmospheric pressure measured by BME280 expressed in hPa
float shtemper = 0;                         // temperature of the detector in degrees celsius
double shsdused = 0;                        // space used on SD card

// function prototypes
void Blink_Alarm();
void Top_oscillo();
void DAC_calculation();
void BCDSwitchRead();
int ConvBin2Dec(int bit1, int bit2, int bit3, int bit4);
void Thresh_Detect();
void create_new_file(fs::FS &fs);
void reopen_file(fs::FS &fs);
void get_detector_name();
void count_nbfiles();
void read_param(fs::FS &fs);
void write_file_header();
void Switch_Oled();
void ErrorSD();
void Mess_Remove_SD();
void SD_capa_alert();
void DispRecData();
void OpeningScreen();
void DispDateTime();
void DispInstantCount();
void DispParam();
void DispSdGps();
void FeedGPS();
void GetGPSframe();
void Set_date_time();
void DispVarProg();
void ListGPSframe();
void RTClowBat();
void SharedData();
void Flash_Alarm();

SemaphoreHandle_t xBinarySemaphore = NULL;   // to handle interrupt on COMPAR pin
SemaphoreHandle_t xSema2 = NULL;             // to handle exclusive access of shared data

//------------------------------------------------------------------
void IRAM_ATTR detectinterrupt() {          // activate Task_Detect
  if (!detect) {
    detect = true;                          // detection indicator
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
      if(xHigherPriorityTaskWoken != pdFALSE) portYIELD_FROM_ISR();
    //}
  }
}

//------------------------------------------------------------------
void Task_Detect(void *pvParameters) {      // detection task
  const char *pcTaskName = "Tack_Detect";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet(NULL);
  while(1)  {
    xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);
    if (detect) {                             // verify detection flag set by the interrupt routine
      vadc0 = ADS.readADC(1);                 // SiPM measure in magnitude not in volts
      vadc = int(ADS.toVoltage(vadc0) * 1000); // Convert SiPM reading in mv   
      if (vadc > detfilter) {                 // check that it is not a parasitic detection
        Top_oscillo();
        digitalWrite(EMPTCAP,HIGH);           // Relay on to accelerate end of detection (empty the capacitor) 
        deadtime = (millis() - deadtime_t0);  // duration of dead time without activity above the detection threshold 
        deadtime_t0 = millis();               // starting time for deadtime 
        count++;       
        now = myrtc.now();   
        sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());      
        if( xSemaphoreTake(xSema2, 0) == pdPASS) {  // mutex semaphore used to keep exclusive acces to data     
          recdata = (String(timestamp) + ";" + vadc + ";" + shvaldav + ";" + deadtime);
          myFile.print(recdata);
          recdata = (";" + shlatitude + ";" + shlongitude + ";" + shatmopres + ";" + shtemper + "\n");        
          myFile.print(recdata); 
          shsdused = shsdused + recdata.length(); // Continuous calculation of occupied space
          xSemaphoreGive(xSema2);            // release exclusivity on shared data
        }
        else errcnt++;                       // shared data access conflict count
        ledcWrite(0, 200);                   // flash brievely led power as indicator detection
        vTaskDelay(pdMS_TO_TICKS(1));        // delay 1 ms
        ledcWrite(0, powbrgt);               // restore the power LED to its normal value  
        digitalWrite(EMPTCAP,LOW);           // restore relay off to allow next detection (fill the capacitor)
      }
      else nbelim++;                         // count of detections eliminated
      detect = false;                        // detection indicator off
      vTaskDelay(pdMS_TO_TICKS(1));          // delay 1 ms
    }  
  }
}

//------------------------------------------------------------------
void Task_Processing( void *pvParameters ) {   // detection task

  while(1)  {   
    if (testmode) {                            // instructions for testing continuous SiPm measures
      vadc0 = ADS.readADC(1);                  // SiPM measure in magnitude not in volts
      vadc = int(ADS.toVoltage(vadc0) * 1000); // Convert SiPM reading in mv                                                    //
    }    
     
    FeedGPS();                                 // To make sure the gps is well fed
    noisea = noisea + ADS.readADC(0);          // measure in magnitude (not in volts)
    nbnoisa++;                                 // count number of noise detections

    if (digitalRead(BUTTON) == LOW) {            // If the button is pressed...
      if (millis() - timbou > 1000) {            // test to avoid rebound
        timbou = millis();   
        if (((millis() - timsync > 604800000)    // if last time check by gps more than 7 days old
        || (timsync == 0))                       // or never checked
        && (bcdvalue == 2))   {                  // and if screen is displaying date & time...
          now = myrtc.now();                 
          DateTime sdt = DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), 30);
          myrtc.adjust(sdt);                     // ...synchronize with external timer when sec = 30 (set time to HH,MM,30) 
        } 
        else {                                   // in all other cases
          myFile.close();                        // close the actual file 
          Mess_Remove_SD();                      // display message to remove SD card
        }
      }
    }

    if (millis() - timoldaff > 999) {          // On each second......... 
      timoldaff = millis();                    // memorize the moment of the display
      if (capalert) {                          // test if remaining capacity on sd card is low
        SD_capa_alert();                       // display alert message on Oled 
        if (powbrgt == 200)   powbrgt = 0; 
        else powbrgt = 200;   
        ledcWrite(0, powbrgt);                 // Blink the power led (with 1 sec frequency)   
      }  

      DAC_calculation();                       // compute DAC from Threshold + (noise average x multiplication factor)      
    
      GetGPSframe();                           // read GPS location and UTC 
      atmopres = round(BME280.readPressure() / 100.0);   // get atmospheric pressure (hPa)
      temperBME = BME280.readTemperature();    // get the temperature of the BME280 pressure sensor (°C) 
      DS18B20.requestTemperatures();           // call reading temperature on DS18B20
      temperSiPM = DS18B20.getTempCByIndex(0); // get the temperature of the scintillator (°C)
      temper = (temperBME + temperSiPM) / 2;

      now = myrtc.now();                       // get actual date & time
      if (now.day() != dayrec) {               // Test if day change
        myFile.close();                        // close the actual file
       vTaskDelay(pdMS_TO_TICKS(10));          // delay 10 ms            
        create_new_file(SD);                   // create a new file 
      }  

      if (timlastop == 0)  {                   // during first hour prog is running...
        if (!setdtrtc) {                       // ... and if not already done...
          Set_date_time();                     // ...first setting date & time on RTC
        }
      }

      if (millis() - timlastop > 3600000)  {   // close and reopen file each hour (in ms) 
        timlastop = millis();                  // memorize the moment of this action                                                         
        reopen_file(SD);                       // Close then reopen the same file in append mode   
        sdremain = (((sdrealcap - shsdused) * 100) / (sdcapacity));    // compute % remaning space on SD        
        if (sdremain < 10) {                   // test if capacity remaining on SD is less than 10%
          capalert = true;                     // setting indicator to blink alert led         
        }
        else capalert = false;                 // no SD capacity alert
        Set_date_time();                       // if necessary, set time to UTC from GPS 
      } 

      if ((dispok = true)                      // if display in progress
      && (millis() - timdisp > 900000))  {     // and display for more than 15 minutes
        dispok = false;                        // Oled indicator off
        testmode = false;                      // test mode desactivation 
        Switch_Oled(); 
        dispend = true;                        // indicator end of diplay    
      }     
    }    

    if( xSemaphoreTake(xSema2, 0) == pdPASS) { // mutex semaphore used to keep exclusive acces to data    
      shvaldav = valdav;
      shlatitude = latitude;
      shlongitude = longitude;
      shatmopres = atmopres;
      shtemper = temper;     
      xSemaphoreGive(xSema2);                  // release exclusivity on shared data     
    }       

    if (!capalert) {                           // Display switch operational only if no alert                    
      testmode = false;                        // test mode des-activation by default   
      BCDSwitchRead();                         // read the switch position for displays 
      if ((dispend) && (swstat == bcdvalue)) continue;  // Display turned off and no switch change
      else {                                   // BCD swith has been changed
        dispok = true;                         // turn Oled on for display 
        dispend = false;                       // restore display possible  
        swstat = bcdvalue;                     // memorize BCD switch position
      }
      if (!dispend) {
        switch(bcdvalue) {
          case 1:                                // test if display "count and saved variables"   
            DispRecData();                       // display recorded data      
            break;        
          case 2:                                // test if display "Date & time"   
            DispDateTime();                      // display Date and time (+ synch message)
            break;
          case 3:                                // test if display SD and GPS data
            DispSdGps();                         // Display display SD and GPS data
            break;  
          case 4:                                // test if display "parameters data"     
            DispParam();                         // display parameters data
            break;
          case 6:                                // test if display "parameters data"  
            DispMessGPSframe();                  // display message to connect serial monitor
            ListGPSframe();                      // print gps frames on serial monitor
            break;
          case 7:                                // test if display "other variables"     
            SharedData();
            break;  
          case 8:                                // test if display "other variables"     
            DispVarProg();
            break;
          case 9:                                // test if display Instant detection variables and others data (=> test mode)
            testmode = true;                     // tests activation for test count measurements 
            DispInstantCount();                  // Display Instant detection variables and others data
            break;
          default:
            dispok = false;                      // turn Oled off for all others switch positions
            Switch_Oled();         
        }
      }  
    }  
  }      
}

//------------------------------------------------------------------
void setup() {  
  Serial.begin(115200);             
  pinMode(OSCILLO, OUTPUT);        // output for oscillo display
  pinMode(POWSIMP, OUTPUT);        // output to powers up the SiMP
  pinMode(BCDSW1, INPUT);          // set BCD switch pin1 on low (external pull-down)
  pinMode(BCDSW2, INPUT);          // set BCD switch pin2 on low (external pull-down)
  pinMode(BCDSW3, INPUT);          // set BCD switch pin3 on low (external pull-down)
  pinMode(BCDSW4, INPUT);          // set BCD switch pin4 on low (external pull-down)
  pinMode(BUTTON, INPUT_PULLUP);   // set adjust button pin on high voltage
  pinMode(EMPTCAP, OUTPUT);        // accelerate end of detection (empty the capacitor)  
  pinMode(COINCI, INPUT);          // coincidence detection flag (0 =false, 1 = true)

  myrtc.begin();                   // start DS3231 RTC

  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);  // start GPS serial port 2

  u8g2.setI2CAddress(0x3C<<1);     // Modify I2C address + 1 bit offset for this Oled screen
  u8g2.begin();                    // screen initialization    
  u8g2.enableUTF8Print();          // allow accented characters       

  if (!SD.begin(SDSSPIN))  {       // Check for the presence of an SD card
    messerr = "Card inserted ?";
    ErrorSD();                     // show error message
  }
  ADS.begin();                     // start ASD1115
  ADS.setGain(1);                  // ± 4.096 volts (range voltage used for detection)
  ADS.setDataRate(7);              // fastest conversion rate.
  ADS.readADC(0);                  // empty reading, to send the parameters
  
  get_detector_name();             // find the name in memory or on the parameter file     

  OpeningScreen();                 // Run the splash screen on start-up 

  BME280.begin(0x76);              // start BME280 pressure sensor
  DS18B20.begin();                 // start DS18B20 scintillator temperature sensor

  pinMode(LEDAL, OUTPUT);          // Led alarm
  digitalWrite(LEDAL, LOW);        // turn the alarm LED off
  
  ledcSetup(0, 5000, 8);           // DAC channel 0 at 5kHz on 8 bits
  ledcAttachPin(LEDPOW, 0);        // Channel 0 for power led
  ledcWrite(0, powbrgt);           // turn the power LED on with low brigtness

  vTaskDelay(pdMS_TO_TICKS(500));  // delay 500 ms
  digitalWrite(POWSIMP,HIGH);      // Powers up the SiPM
  
  timnois1 = millis();             
  timnois2 = millis();
  while ((millis() - timnois1) < 4001) {   // stable noise waiting loop (during 4 sec)
    if (millis() - timnois2 > 999) {       // each second...
      DAC_calculation();                   // ...call average noise calculation
      timnois2 = millis();
    }            
    noisea = noisea + ADS.readADC(0);      // measure in magnitude (not in volts)
    nbnoisa++;                             // count number of noise detections
  }  

  Set_date_time();                         // if necessary, set time to UTC from GPS 
  count_nbfiles();                         // call file count count
  create_new_file(SD);

  if (digitalRead(COINCI) == HIGH)  coincid = true; // coincidence detection is enabled 
  dispok = false;                                  // set Oled off  
  Switch_Oled();                                   // to switch Oled off (no display) 
  deadtime_t0 = millis();                          // starting time for deadtime  
  valdav = int(ADS.toVoltage(ADS.readADC(2)) * 1000);  // pre-reading DAC threshold in mv    
  
  attachInterrupt(digitalPinToInterrupt(COMPAR), detectinterrupt, FALLING);
  xBinarySemaphore = xSemaphoreCreateBinary();     
  xSema2 = xSemaphoreCreateMutex();  

  xTaskCreatePinnedToCore(
              Task_Detect,      // Task function.  
              "Task_Detect",    // name of task.  
              10000,            // Stack size of task  
              NULL,             // parameter of the task  
              10,               // priority of the task  
              NULL,             // Task handle to keep track of created task
              0);               // on core 0

  xTaskCreatePinnedToCore(
              Task_Processing,   // Task function.  
              "Task_Processing", // name of task.  
              10000,             // Stack size of task  
              NULL,              // parameter of the task  
              1,                 // priority of the task  
              NULL,              // Task handle to keep track of created task
              1);                // on core 1
}

//------------------------------------------------------------------
void loop()  {}   
//________________________________End_______________________________
