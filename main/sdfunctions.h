#ifndef SDFUN_H
#define SDFUN_H

#include <SPI.h> 
#include "SdFat.h"
#include "sdios.h"

typedef struct filetree{
 char filename[20];
 struct filetree *nextfile = NULL; 
}FileTree;

extern bool currentlyopen;

//Functions
#ifdef __cplusplus
extern "C" {
#endif

void buildFileTree(FileTree* head);
byte getNumberOfFiles(FileTree* head);
void readLine(SdFile *thefile, char *dest);
bool doesExist(char* filename);
void SDError();

#ifdef __cplusplus
}
#endif

#endif
