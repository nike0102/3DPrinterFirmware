#include <SPI.h> 
#include "SdFat.h"
#include "sdios.h"

// SD card chip select pin.
const uint8_t chipSelect = SS;

// File system object.
SdFat sd;

// Directory file.
SdFile root;

// Use for file creation in folders.
SdFile file;

char buf[40] = {'\0'};
bool good;
int i;
void setup() {
  Serial.begin(9600);
  
  while (!Serial) {
    // Wait for USB Serial 
  }
  delay(1000);
  
  readfiles();
  
  file.open("test.gcode", O_RDONLY);
  Serial.println("\nLine 1");
  readline(&file);
  Serial.println("Line 2");
  readline(&file);
  Serial.println("Line 3");
  readline(&file);
  Serial.println("Line 4");
  readline(&file);
  Serial.println("Line 5");
  readline(&file);
  file.close();

}

void readline(SdFile *thefile){
  char tempbuf[25] = {0};
  byte i;

  tempbuf[0] = thefile->read();
  
  if (tempbuf[0] != ';' && tempbuf[0] != 'M' && tempbuf[0] != 'G'){
    while (tempbuf[0] != ';' && tempbuf[0] != 'M' && tempbuf[0] != 'G'){
      tempbuf[0] = thefile->read();
    }
  }
  
  for (i = 1; i < 25 && thefile->available(); i++){
    tempbuf[i] = thefile->read();
    if (tempbuf[i] == '\r'){
      tempbuf[i] = '\0';
      break;
    }
  }
  Serial.println(tempbuf);
  for (i = 0; i < 25; i++){
    tempbuf[i] = 0;
  }
}

void readfiles(){
    if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    Serial.println("SD card initalization failed");
    while(1);
  }

  //Error check
  if (!root.open("/")){
    Serial.println("Open root failed");
  }    

  Serial.println("Files on SD card ->");

  //Ignore first file - System Volume Information
  file.openNext(&root, O_RDONLY);
  file.close();
  
  //Open each file
  while (file.openNext(&root, O_RDONLY)){

    //Clear buffer
    for (i = 0; i < 40; buf[i++] = 0){}

    //Load name into buffer
    good = file.getName(buf, 40);
    if (good == true){
      Serial.println(buf);
    }
    file.close();
  }
}

void loop(){
  
}
