#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include "parser.h"
#include "sdfunctions.h"
#include "WiFiEsp.h"

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
void powerOn();
void sendHttpResponseMain();
void readRequest(HTMLRequest *htreq);

#ifdef __cplusplus
}
#endif


#endif
