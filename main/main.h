#ifndef MAIN_H
#define MAIN_H

//Includes
#include "PrinterConfig.h"
#include "sdfunctions.h"
#include "parser.h"
#include "gcodefunctions.h"
#include "wifi.h"


//Global variables
FileTree FilesOnSDCard;
GCommand currentGCodeCommand;
char inputBuffer[25] = {0};
char selectedSDFile[25] = {0};

extern HTMLRequest htreq;
extern bool currentlyPrinting;
extern bool doneWithCommand;

//Functions
void loop();
void setup();
void handleCommand(GCommand *command);
void getSerialInput();

#endif
