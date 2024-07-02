//-----------------------------------------------------------------
void create_new_file(fs::FS &fs) {   
  now = myrtc.now(); 
  if (now.year() > 2000) {                      // to avoid wrong data from rtc
    dayrec = now.day();                           // memorize new day
    sprintf(datew, "%02d%02d%02d", now.year() - 2000, now.month(), now.day()); 
    for (int i = 2; i < 8; i++)  {
      filename[i] = datew[i - 2];                 // file name including today's date as YYMMDD
    }  
    myFile = fs.open(filename, FILE_READ);      // test if file already exist  
    if (!myFile) {                              // if this file doesn't exist on the SD card...
      myFile = fs.open(filename, FILE_WRITE);   // ...create a new file 
      if (!myFile) {                            // test the correct operation
        messerr = "File open error";
        ErrorSD(); 
      } 
      else {     
        count = 0;                              // reinitialize counter for the new file 
        nbelim = 0;                             // reinitialize number of detections eliminated
        crapfile = 'C';                         // indicator file in creation mode
        write_file_header();                    // for a new file write file header
        nbfiles++;                              // increment the number of files    
      }   
    } 
    else { 
      myFile.close();
      myFile = fs.open(filename, FILE_APPEND);  // open the existing file for append
      if (!myFile) {                            // test the correct operation
        messerr = "File open error";
        ErrorSD(); 
      }     
      else {crapfile = 'A'; }                   // indicator file in append mode    
    }   
  } 
  else {
    Flash_Alarm();                             // Flash alarm led 
    RTClowBat();                               // display RTC low battery message
    RTCbatempty = true;                        // Set indicator low bat on RTC for display
  }
  sdcapacity = SD.totalBytes();
  sdused = SD.usedBytes();
  shsdused = sdused; 
  sdrealcap = sdcapacity - sdused;           // calculates the actual remaining capacity                    
}

//------------------------------------------------------------------
void reopen_file(fs::FS &fs) {              // Test the proper functioning of the SD card
  myFile.close();
  vTaskDelay(pdMS_TO_TICKS(50));          // delay 50 ms
  crapfile = 'A';                           // set indicator file in append mode
  myFile = fs.open(filename, FILE_APPEND);  // close an reopen file... 
  if (!myFile) {                            // ...then test the correct operation
    messerr = "no longer works";
    ErrorSD(); 
  } 
}
 
//------------------------------------------------------------------
void get_detector_name() {                  // read the name of the detector in non-volatile memory
  detectorname.begin("cosmic watch", true); // true set mode "read only" prefs
  bool exist = detectorname.isKey("detectid");  // test for the presence of the memory key
  if (!exist)  {                            // if not found
    detname = "no name";                    // "no name" assigned for this device
  }    
  else {    
    detname = detectorname.getString("detectid");  // read detector name in key "detectid"
  } 
  detectorname.end();  
  read_param(SD);                          // name may be changed by parameters in PARAM file 
}

 //------------------------------------------------------------------
void count_nbfiles() {                    // count of the number of files present
  nbfiles = 0;
  File root;
  root = SD.open("/");                    // open sd card as root
  File nfile = root.openNextFile();
  while (nfile)  {                        // Test if file present
    nfile = root.openNextFile();
    if (nfile)  nbfiles++;                // count the number of files 
  }
  nbfiles--;                              // minus 1 to not count the parameter file
}

//------------------------------------------------------------------
void read_param(fs::FS &fs) {  
  myFile = fs.open("/PARAM.txt",FILE_READ);    // this file is optional but takes priority over the name already assigned
  if (myFile) {                                // presence test parameter file 
    char c;                              
    String devinam = ""; 
    String regcode = "";
    String readstring = "";                                                        
    while (myFile.available()) {
      while (regcode != "P4") {                // while last record has not been reached
        regcode = "";                              
        readstring = "";    
        for (int i = 0; i < 2; i++) {          // record code = 2 char.
          c = myFile.read();
          regcode += c ;                       // concatenation of the record code
        }
        c = myFile.read();                     // jump over the ";"" separator
       while (c != '<')  {
          c = myFile.read(); 
          if (c != '<') readstring +=c;        // reading the data up to the character "<"
        }
        while (c != '\n') {                    // up to the end of the record,....
          c = myFile.read();                   // .. reading the rest of unused data 
        }          
           
        if (regcode == "P1") devinam = readstring.substring(0,14);   // get the device name
        if (regcode == "P2") impldat = readstring.substring(0,10);   // get the implementation date
        if (regcode == "P3") sponsor = readstring.substring(0,21);   // get the sponsor or donator name
        if (regcode == "P4") shipname = readstring.substring(0,21);  // get the ship name                                     
      }
    }
    myFile.close(); 
    if ((devinam != (F("              ")))          // if a device name is provided in parameters
    && (devinam != detname)) {                      // test if already saved in non-volatile memory...
      detname = devinam;                            // ...or has been modified.
      detectorname.begin("cosmic watch", false);    // false set mode "read/write" prefs   
      detectorname.putString("detectid", detname);  // save new detector name under key "detectid"
      detectorname.end();    
    }         
  }      
} 

//------------------------------------------------------------------
void write_file_header() {
  now = myrtc.now();     
  sprintf(timestamp, "%02d-%02d-%04d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());   
  myFile.print(F("#  Polar pod muons watch detector     Ship = "));
  myFile.print(String(shipname).substring(0,20));  
  myFile.print(F("     filename = "));
  myFile.println(String(filename).substring(1,12));
  myFile.print(F("#  YLC / MHK => Astrolabe Expeditions               Created on : "));
  myFile.print(timestamp);
  myFile.println(F(" UTC"));
  myFile.print(F("#  Device ID : "));
  myFile.print(detname.substring(0,14)); 
  if (coincid) myFile.print(F("     Coincidence detection")); 
  else myFile.print(F("       Single detection"));
  myFile.println(F("                  FORMAT :"));
  myFile.println(F("Timestamp[UTC];SiPM[mV];Thr.DAC[mv];Deadtime[ms];Latitude[deg];Longitude[deg];Press(hPa);Temp(°C)"));  
}

//------------------------------------------------------------------