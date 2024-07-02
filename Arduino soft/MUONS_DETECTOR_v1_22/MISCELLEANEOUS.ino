//------------------------------------------------------------------
void Blink_Alarm() {
  while(1) {                                  // infinite loop 
    aledstat = !aledstat;                     // state reversal  
    digitalWrite(LEDAL, aledstat);            // Blink the alarm LED
    vTaskDelay(pdMS_TO_TICKS(300));           // delay 300 ms   
  }       
}

//------------------------------------------------------------------
void Flash_Alarm() {
  digitalWrite(LEDAL, HIGH);                  // Switch on the alarm LED
  vTaskDelay(pdMS_TO_TICKS(500));             // delay 500 ms  
  digitalWrite(LEDAL, LOW);                   // Switch off the alarm LED
}
//------------------------------------------------------------------
void Top_oscillo() {                          // used to track oscilloscope detections during tests
  digitalWrite(OSCILLO,HIGH);       
  digitalWrite(OSCILLO,LOW);                  // send a brief signal to the oscilloscope output
  }

//------------------------------------------------------------------
void DAC_calculation() {                      // compute DAC from : Threshold + noise average
  int realvaldac = 0;                         // Value really send by the DAC to the comparator in magnitude on 10 bits (0 to 2047))                            
  int vthresh = 0;                            // Detection threshold in millivolts 
  int wthresh = 0;                            // Detection threshold in magnitude, exprimed with 8 bits (0 to 255)
  int wnoise = 0;                             // average noise in magnitude, exprimed with 8 bits (0 to 255)

  // computing permanent noise smoothed over 2 seconds :
  if ((nbnoisa > 0) || (nbnoisb > 0)) {                   // if noise was measured...
    noise1 = (noisea + noiseb) / (nbnoisa + nbnoisb);     // weighted average noise during 2 seconds (in magnitude, not in volts)
    noiseb = noisea;                                      // sliding noise detection from last second to previous second 
    nbnoisb = nbnoisa;                                    // sliding count detection from last second to previous second 
    noisea = 0;                                           // reset sum of noise for last second beginning now
    nbnoisa = 0;                                          // reset sum of counts for last second beginning now
}
  // Detection threshold sent to the comparator by the DAC, which is calculated by average noise over 2 seconds
  // to which we add a margin (provided by a potentiometer)
  wnoise = map(noise1,0,2047,0,255);                      // convert a 10 bits value (from ADS1015) to equivalent 8 bits (DAC ESP32) 
  det_threshold = ADS.readADC(3);                         // reading potentiometer for detection threshold in magnitude  
  wthresh = map(det_threshold,0,1650,0,255);              // convert a 10 bits value (from ADS1015) to equivalent 8 bits (DAC ESP32) (2047/4.096*3,3)
  valdac = wnoise + wthresh;                              // detection threshold in magnitude on 8 bits for external comparator
  dacWrite(DAC, valdac);                                  // send this threshold to the external comparator by DAC 
  // voltage really send by DAC to comparator is also send to ADS1015 and may be read on ADS1015 pin 2 
  realvaldac = ADS.readADC(2);                            // reading value send by the DAC in magnitude (max val = 1351 = 2047/5*3,3) 
  noise = int(ADS.toVoltage(noise1) * 1000);              // convert average noise magnitude from ASD1015 in mv for display 
  valdav = int(ADS.toVoltage(realvaldac) * 1000);         // convert reading value send by DAC in mv for display 
  detfilter = valdav / 5;                                 // compute low threshold detection to eliminate parasitics detections 
} 

//------------------------------------------------------------------
void BCDSwitchRead() {                                    // reading 4 pins of the BCD switch
  bitr[0] = digitalRead(BCDSW1);                          // read most left digit (heavy weight)
  bitr[1] = digitalRead(BCDSW2);
  bitr[2] = digitalRead(BCDSW3);
  bitr[3] = digitalRead(BCDSW4);                          // read most right digit (low weight)
  bcdvalue = (bitr[0] * 8) + (bitr[1] * 4) + (bitr[2] * 2) + bitr[3];  // conversion to decimal value
}

//---------------------------------------------------