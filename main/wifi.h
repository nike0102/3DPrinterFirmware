#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include "PrinterConfig.h"
#include "parser.h"
#include "sdfunctions.h"
#include "WiFiEsp.h"
#include "gcodefunctions.h"

//Structures
typedef struct HTMLrequest{
  uint8_t type = 0;                  //GET -> 1, POST -> 2
  int messagelength = -1, alreadyread = 0;
  bool currentlyReadingFile = false;
  char filename[25] = {0};
  char link[25] = {0};
}HTMLRequest;

extern WiFiEspClient client;
extern WiFiEspServer server;

#ifdef __cplusplus
extern "C" {
#endif

//Functions
void powerOnWiFi();
void sendHttpResponseMain();
void readRequest(HTMLRequest *htreq);
void sendHttpFiles(char* fname, char* fnum);

//Variables
extern bool currentlyPrinting;
extern bool currentlyopen;
extern FileTree FilesOnSDCard;
extern char inputBuffer[25];
extern short ExtruderTemp;
extern short BedTemp;
extern void getInput(bool isSerial);

#ifdef __cplusplus
}
#endif


#endif
