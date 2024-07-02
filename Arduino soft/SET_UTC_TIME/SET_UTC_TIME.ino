/* Muon detector set Date and Time in UTC time from connected computer on RTC DS3231 
   code written for Polar pod cosmic detection by YLC on 20/12/2022
   This programm must be executed only once before starting the detector.
*/

#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2/*, reset= U8X8_PIN_NONE */);  // to be modified according to screen
// screens list : https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// fonts list : https://github.com/olikraus/u8g2/wiki/fntlistall
#include "RTClib.h"
RTC_DS3231 myrtc;

int hourgap = 99;   // time difference between TU and time of the computer (number of hours in signed value)    
char datew[12]; 
char timew[10]; 
boolean rspd = false;      
int tuhour = 0;    // computed universal time hour
DateTime tutime;
DateTime now;

//--------------------------------------------------------------------------
void affdate() {
  now = myrtc.now();
  sprintf(datew, "%02d-%02d-%04d", now.day(), now.month(), now.year());  
  Serial.println("");
  Serial.println("------ Muon detector set UTC time ---------");           
  Serial.print("Date : ");  
  Serial.println(datew);   
}

//--------------------------------------------------------------------------
void afftime() {
  now = myrtc.now(); 
  sprintf(timew, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());             
  Serial.println(timew);  
}

//------------------------------------------------------------------
void DispDateTime() {              // Date and time display  
  char datw[12]; 
  char heurw[10]; 
  now = myrtc.now();    
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_crox3hb_tf );
  u8g2.setCursor(10,15);      
  sprintf(datw, "%02d-%02d-%04d", now.day(), now.month(), now.year());              
  u8g2.print(datw);    
  u8g2.setFont(u8g2_font_inb16_mf ); 
  u8g2.setCursor(0,55);       
  sprintf(heurw, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());            
  u8g2.print(heurw);  
  u8g2.sendBuffer(); 
}   

//--------------------------------------------------------------------------
void setup () {
  Serial.begin(9600);
  u8g2.setI2CAddress(0x3C<<1);     // Modify I2C address + 1 bit offset for this Oled screen
  u8g2.begin();                    // screen initialization    
  u8g2.enableUTF8Print();          // allow accented characters               
  myrtc.begin();                   // start DS3231 RTC
  myrtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   // set rtc date and time from computer 
  DispDateTime();
}
//--------------------------------------------------------------------------
void loop () {
  affdate(); 
  Serial.print("Local time is : ");  
  afftime();
  while (Serial.available()) { char inChar = Serial.read();}    // empty serial buffer
  Serial.println("Enter the signed number of hours difference between UTC and your time zone");  
  hourgap = 99;  
  while (hourgap == 99) {
    if (Serial.available())  {
      hourgap =  Serial.parseInt();      
    }        
  }   
  Serial.print("You answered ");  
  Serial.println(hourgap);
  if (!hourgap == 0) {   
    DateTime now = myrtc.now(); 
    tuhour = now.hour() + hourgap; 
    if ((tuhour > -1) && (tuhour < 24)) {
      DateTime tutime = DateTime(now.year(), now.month(), now.day(), tuhour, now.minute(), now.second());
      myrtc.adjust(tutime); 
      Serial.println("RTC changed to UTC"); 
      rspd = true;         
    }
    else {
      Serial.println("Please, make sure that we stay on the same date");  
      Serial.println("RTC time has not been changed !");  
    }
  }      
  else { 
    Serial.println("Nothing to change in the RTC."); 
  } 
  if (rspd) { 
    Serial.print("UTC (Universal Coordinated Time) is : ");      
    afftime(); 
     Serial.println("-------------------------------------");  
  }     
  while (1) {
    DispDateTime();
    delay(1000);   
  }
}
