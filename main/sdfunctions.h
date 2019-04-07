#ifndef SDFUN_H
#define SDFUN_H

#include <SPI.h> 
#include "SdFat.h"
#include "sdios.h"

typedef struct filetree{
 char filename[20];
 struct filetree *nextfile = NULL; 
}FileTree;

extern SdFat sd; // File system object.
extern SdFile root;  // Directory file.
extern const uint8_t chipSelect;

//Functions
#ifdef __cplusplus
extern "C" {
#endif

void buildFileTree(FileTree* head);
byte getNumberOfFiles(FileTree* head);
void readLine(SdFile *thefile, char *dest);
void writeCharToFile(SdFile *thefile, char writechar);
bool doesExist(char* filename);
void SDError();
extern bool currentlyopen;

#ifdef __cplusplus
}
#endif

#endif
