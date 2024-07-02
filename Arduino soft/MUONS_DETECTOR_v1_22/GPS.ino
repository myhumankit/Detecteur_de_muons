//------------------------------------------------------------------
void FeedGPS() {                // To make sure the gps is well fed
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }
}

//------------------------------------------------------------------
void GetGPSframe() {                       
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());

    if (gps.satellites.isUpdated()) {
      nbsat = gps.satellites.value();            // get the number of detected satellites
    }
    
    if ((gps.location.isUpdated())
    && (gps.location.isValid()))  {
      latitude  = String(gps.location.lat(),6);  // get coordinates
      longitude = String(gps.location.lng(),6);
      gpstime = millis();                        // get time of coordinates
      memolati = latitude;
      memolongi = longitude;
      if (gps.date.day() != 0)   {                 // if valid date
        yeargps  = gps.date.year();                // update date and time from GPS
        monthgps = gps.date.month();
        daygps   = gps.date.day();
        hourgps  = gps.time.hour();
        minugps  = gps.time.minute();
        secogps  = gps.time.second();     
      }  
      else {
        yeargps  = -1;                            // otherwise, set GPS date and time not updated
        monthgps = -1;
        daygps   = -1;
        hourgps  = -1;
        minugps  = -1;
        secogps  = -1;
      } 
    }   
    else {                                       // if no coordinates updated
      if (millis() - gpstime < 300000) {         // ...we consider that the coordinates remain valid for 5 minutes
        latitude = memolati;
        longitude = memolongi;
      }
      else {
        latitude  = "";                          // ...and later we delete the coordinates
        longitude = "";
      }
    }  
  }                 
}

//------------------------------------------------------------------
void Set_date_time() {                      // update RTC date & time from GPS
  GetGPSframe();                           // read GPS UTC 
  now = myrtc.now(); 
  if ((nbsat > 0)
  && (secogps != -1) && (daygps != -1)) {  // If date and time from GPS                    
    if (now.second() != secogps) {          // ... => comparison rtc seconds with GPS UTC seconds 
      DateTime sdt = DateTime(yeargps, monthgps, daygps, hourgps, minugps, secogps);
      myrtc.adjust(sdt);                    // if different => update of the RTC time from that of the GPS
      sprintf(updatim, "%04d-%02d-%02d %02d:%02d:%02d", yeargps, monthgps, daygps, hourgps, minugps, secogps);   
      setdtrtc = true;                    // RTC updating done
    }  
    timsync = millis();                   // save the last moment of timer is checked               
  }
} 

//------------------------------------------------------------------
void ListGPSframe() {                
  while (SerialGPS.available() > 0) {    
    char gpsData = SerialGPS.read();      
    Serial.print(gpsData);                  
  }                   
}

//------------------------------------------------------------------
