//------------------------------------------------------------------
void Switch_Oled() {               // To witch off or on the oled depending on display or not
  if (!dispok == oldisp) {         // so as not to repeat the operation
    oldisp = dispok;
    u8g2.clearBuffer();
    if (dispok) { 
      u8g2.setPowerSave(0);        // turn OLed on
      timdisp = millis();          // save display start time
    }
    else u8g2.setPowerSave(1);     // turn OLed off
  }
}

//------------------------------------------------------------------
void ErrorSD() {                   //  in case of error on the SD card
  u8g2.setPowerSave(0);            // turn OLed on
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_crox3hb_tf);
  u8g2.setCursor(25,15);
  u8g2.print(F("SD Card"));
  u8g2.setCursor(20,37);
  u8g2.print(F("E R R O R"));
  u8g2.setCursor(0,60);
  u8g2.print(messerr);
  u8g2.sendBuffer();  
  Blink_Alarm();                   
}

//------------------------------------------------------------------
void Mess_Remove_SD() {            //  in case of error on the SD card
  u8g2.setPowerSave(0);            // turn OLed on
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_samim_12_t_all); 
  u8g2.setCursor(15,14);
  u8g2.print(F("File  "));
  u8g2.print(String(filename).substring(1,8));
  u8g2.setCursor(15,29);
  u8g2.print(F("has been closed."));
  u8g2.setCursor(0,44);
  u8g2.print(F("Change the SD card or"));
  u8g2.setCursor(0,59);
  u8g2.print(F("restart the program"));
  u8g2.sendBuffer();
  Blink_Alarm();                     
}

//------------------------------------------------------------------
void SD_capa_alert() {             // display alert message about sd capacity being low 
  u8g2.setPowerSave(0);            // turn OLed on
  u8g2.clearBuffer(); 
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,9);
  u8g2.print(F("Count "));   
  u8g2.setCursor(35,9);
  u8g2.print(count); 
  u8g2.setCursor(75,9);
  u8g2.print(String(filename).substring(1,8)); 
  u8g2.setCursor(0, 30);
  u8g2.print(F("WARNING    SD  "));
  u8g2.print(long(sdcapacity / (1024 * 1024)));
  u8g2.print(F("  Mo."));
  u8g2.setCursor(0,44);
  u8g2.print(F("Remaining space on SD card"));
  u8g2.setCursor(15,57);
  u8g2.print(F("is only   "));
  u8g2.print(sdremain);
  u8g2.print(F("    %."));
  u8g2.sendBuffer();
}

//------------------------------------------------------------------
void DispRecData() {               // displaying recorded data and variables
  Switch_Oled();  
  if (dispok) {  
    u8g2.clearBuffer();  
    u8g2.setFont(u8g2_font_glasstown_nbp_tf);
    u8g2.setCursor(0,9);
    u8g2.print(F("Count "));   
    u8g2.setCursor(35,9);
    u8g2.print(count); 
    u8g2.setCursor(78,9);
    u8g2.print(F("P. "));
    u8g2.print(atmopres);
    u8g2.print(F(" hPa"));
    u8g2.setCursor(0,23);
    u8g2.print(F("Coincid ")); 
    u8g2.setCursor(35,23); 
    if (coincid) u8g2.print(F("OUI")); 
    else u8g2.print(F("NON"));  
    u8g2.setCursor(75,23);
    u8g2.print(F("Tmp "));
    u8g2.print(temper,1); 
    u8g2.print(F("°C"));       
    u8g2.setCursor(0,37);
    u8g2.print(F("Timstp "));
    u8g2.setCursor(35,37);
    u8g2.print(timestamp);
    u8g2.setCursor(0,51);
    u8g2.print(F("SiPM "));
    u8g2.setCursor(35,51);
    u8g2.print(vadc);   
    u8g2.print(F(" mv"));       
    u8g2.setCursor(75,50);
    u8g2.print(F("Noise "));
    u8g2.print(noise);  
    u8g2.print(F(" mv"));   
    u8g2.setCursor(0,64);
    u8g2.print(F("Dead.T. "));
    u8g2.setCursor(35,64);
    if (deadtime > 60000) {
      deadaff = deadtime / 60000;
      u8g2.print(deadaff);
      u8g2.print(F(" min"));
    }
    else {
      if (deadtime > 1000) {
        deadaff = deadtime / 1000;
        u8g2.print(deadaff);
        u8g2.print(F(" sec"));
      }
      else {
        u8g2.print(deadtime);
        u8g2.print(F(" ms"));
      }
    }
    u8g2.setCursor(75,64);
    u8g2.print(F("DAC "));
    u8g2.print(valdav);  
    u8g2.print(F(" mv"));            
    u8g2.sendBuffer();
  }
}

//------------------------------------------------------------------
void OpeningScreen() {             // Splash screen with the device name
  u8g2.clearBuffer();
  u8g2.drawFrame(0,0,128,64);
  u8g2.setFont(u8g2_font_crox3hb_tf );
  u8g2.setCursor(20,16);
  u8g2.print(F("CosmicSail"));
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(10,30);
  u8g2.print(F("By  Astrolabe  Expeditions"));
  u8g2.setCursor(27,43);
  u8g2.print(F("& My  Human  Kit"));
  u8g2.setCursor(5,60);
  u8g2.print(F("Dev: ")); 
  u8g2.setFont(u8g2_font_crox2tb_tf);  
  u8g2.print(detname);                         
  u8g2.sendBuffer(); 
}

//------------------------------------------------------------------
void DispDateTime() {              // Displaying UTC Date and time 
  Switch_Oled(); 
  u8g2.clearBuffer();
  now = myrtc.now();     
  u8g2.drawRFrame(15,0,112,40,9);
  u8g2.setFont(u8g2_font_courB12_tf );
  u8g2.setCursor(20,15); 
  sprintf(datew, "%02d-%02d-%04d", now.day(), now.month(), now.year());                  
  u8g2.print(datew);    
  u8g2.setFont(u8g2_font_profont22_tf); 
  u8g2.setCursor(22,35);  
  sprintf(timew, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());                 
  u8g2.print(timew); 
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  if ((millis() - timsync > 604800000)     // if last time check by gps more than 7 days old
  || (timsync == 0))  {                    // or never checked
    u8g2.setCursor(15,52);
    u8g2.print(F("You can correct the")); 
    u8g2.setCursor(20,63); 
    u8g2.print(F("seconds manually"));
  }
  else {
    u8g2.setCursor(5,52);
    u8g2.print(F("Time is checked and updated")); 
    u8g2.setCursor(10,63); 
    u8g2.print(F("automatically  by  GPS"));
  }  
  u8g2.setFont(u8g2_font_battery19_tn);
  u8g2.setCursor(0,30); 
  if (RTCbatempty) {
    affbat = !affbat;
    if (affbat) u8g2.print("1"); 
    else u8g2.print("0"); 
  }
  else u8g2.print("4"); 
  u8g2.sendBuffer(); 
}  

//------------------------------------------------------------------
void DispInstantCount() {              // Displaying instant count and other data for tests
  Switch_Oled(); 
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,9);
  u8g2.print(relnum);   
  u8g2.setCursor(80,9);
  u8g2.print(F("Elim "));
  u8g2.setCursor(103,9);  
  u8g2.print(nbelim);    
  u8g2.setCursor(0,23);
  u8g2.print(F("Vadc "));   
  u8g2.setCursor(35,23);
  u8g2.print(vadc); 
  u8g2.print(F(" mv"));
  u8g2.setCursor(80,23);
  u8g2.print(F("Filt "));   
  u8g2.setCursor(103,23);
  u8g2.print(detfilter);
  u8g2.setCursor(80,37);
  u8g2.print(F("Day "));   
  u8g2.setCursor(103,37);
  u8g2.print(dayrec); 
  u8g2.setCursor(0,51);
  u8g2.print(F("Noise "));   
  u8g2.setCursor(35,51);
  u8g2.print(noise); 
  u8g2.print(F(" mv")); 
  u8g2.setCursor(80,51); 
  u8g2.setCursor(0,64);
  u8g2.print(F("Dac "));   
  u8g2.setCursor(35,64);
  u8g2.print(valdav); 
  u8g2.print(F(" mv")); 
  u8g2.setCursor(80,64);
  u8g2.print(F("Err "));   
  u8g2.setCursor(103,64);
  u8g2.print(errcnt); 
  u8g2.sendBuffer();  
}

//------------------------------------------------------------------
void DispParam() {              // Display of the parameters provided by the file
  Switch_Oled(); 
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(70,9);
  u8g2.print(F("Parameters")); 
  u8g2.setCursor(0,23);
  u8g2.print(F("Dev. ")); 
  u8g2.setCursor(30,23);
  u8g2.print(detname);  
  u8g2.setCursor(0,36);
  u8g2.print(F("Imple. ")); 
  u8g2.setCursor(30,36);
  u8g2.print(impldat);  
  u8g2.setCursor(0,49);
  u8g2.print(F("Spons. ")); 
  u8g2.setCursor(30,49);
  u8g2.print(sponsor);  
  u8g2.setCursor(0,62);
  u8g2.print(F("Ship ")); 
  u8g2.setCursor(30,62);
  u8g2.print(shipname);  
  u8g2.sendBuffer();    
}

//------------------------------------------------------------------
void DispSdGps() {              // Display of SD card and GPS data
  float sdfilled = 0;
  int pcfilled = 0;
  Switch_Oled();
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,9);
  u8g2.print(F("Nb Fil. ")); 
  u8g2.print(nbfiles);  
  u8g2.setCursor(65,9);
  u8g2.print(F("File "));
  u8g2.print(String(filename).substring(1,8)); 
  u8g2.print(F(" "));
  u8g2.print(crapfile); 
  sdfilled = sdused / (1024 * 1024); 
  u8g2.setCursor(0,23); 
  u8g2.print("SD : "); 
  u8g2.print(sdfilled,3); 
  u8g2.print("  Mo");    
  u8g2.print(F("  /  "));
  u8g2.print(int(sdcapacity / (1024 * 1024)));
  u8g2.print(F("  Mo"));  
  u8g2.drawFrame(0,28,127,6);
  pcfilled = sdused * 127 / sdcapacity;            // compute SD filled part
  if (pcfilled < 2)  pcfilled = 3;                 // Graphic minimal representation (but no proportional)
  u8g2.drawBox(0,28,pcfilled,6);                   // SD filling progress bar
  u8g2.setCursor(0,50);  
  u8g2.print(F("  Lat.  "));
  u8g2.print(latitude);   
  u8g2.setCursor(0,61); 
  u8g2.print(F("Long.  ")); 
  u8g2.print(longitude); 
  u8g2.setCursor(85,61);
  u8g2.print(F("N.Sat. "));   
  u8g2.print(nbsat);  
  u8g2.sendBuffer();   
}  

//------------------------------------------------------------------
void DispVarProg() {              // Display other variables for debugging
  Switch_Oled(); 
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,9);
  u8g2.print("yeargps ");  
  u8g2.print(yeargps);  
  u8g2.setCursor(70,9);
  u8g2.print("hourgps "); 
  u8g2.print(hourgps); 
  u8g2.setCursor(0,23);
  u8g2.print("monthgps ");  
  u8g2.print(monthgps);  
  u8g2.setCursor(70,23);
  u8g2.print("minugps "); 
  u8g2.print(minugps); 
  u8g2.setCursor(0,37);
  u8g2.print("daygps ");  
  u8g2.print(daygps);  
  u8g2.setCursor(70,37);
  u8g2.print("secogps "); 
  u8g2.print(secogps); 
  u8g2.setCursor(0,51);
  u8g2.print("timsync "); 
  u8g2.print(timsync);  
  u8g2.setCursor(90,51);
  u8g2.print("setrtc "); 
  u8g2.print(setdtrtc); 
  u8g2.setCursor(0,64);
  u8g2.print("Update "); 
  u8g2.print(String(updatim));  
  u8g2.sendBuffer();    
}

//------------------------------------------------------------------
void RTClowBat() {
  u8g2.setPowerSave(0);            // turn OLed on
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_crox3hb_tf);
  u8g2.setCursor(40,15);
  u8g2.print(F("RTC"));
  u8g2.setCursor(0,37);
  u8g2.print(F("Real Time Clock"));
  u8g2.setCursor(15,59);
  u8g2.print(F("low battery"));
  u8g2.sendBuffer();
  vTaskDelay(pdMS_TO_TICKS(2000));          // delay 2 s
}

//------------------------------------------------------------------
void SharedData() {    
  Switch_Oled(); 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,9);
  u8g2.print("shvaldav ");  
  u8g2.print(shvaldav);  
  u8g2.setCursor(90,9);
  u8g2.print(valdav); 
  u8g2.setCursor(0,23);
  u8g2.print("shlatitude ");  
  u8g2.print(shlatitude);  
  u8g2.setCursor(90,23); 
  u8g2.print(latitude); 
  u8g2.setCursor(0,37);
  u8g2.print("shlongitude ");  
  u8g2.print(shlongitude);  
  u8g2.setCursor(90,37);
   u8g2.print(longitude);
  u8g2.setCursor(0,51);
  u8g2.print("shatmopres ");  
  u8g2.print(shatmopres);  
  u8g2.setCursor(90,51);
  u8g2.print(atmopres);
  u8g2.setCursor(0,64);
  u8g2.print("shtemper ");  
  u8g2.print(shtemper);  
  u8g2.setCursor(90,64);
  u8g2.print(temper);
  u8g2.sendBuffer();
}

//------------------------------------------------------------------
void DispMessGPSframe()  {    
  Switch_Oled(); 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_samim_12_t_all); 
  u8g2.setCursor(0,14);
  u8g2.print(F("Connect serial monitor"));
  u8g2.setCursor(0,30);
  u8g2.print(F("to list the currently"));
  u8g2.setCursor(0,46);
  u8g2.print(F("received GPS frames."));
  u8g2.sendBuffer();
}

//------------------------------------------------------------------