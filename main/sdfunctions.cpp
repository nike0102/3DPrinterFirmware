#include "sdfunctions.h"


// SD card chip select pin.
const uint8_t chipSelect = SS;

SdFat sd; // File system object.
SdFile root;  // Directory file.
SdFile file;  // Use for file creation in folders.

bool currentlyopen = false;

//Functions
void buildFileTree(FileTree* head){
  
  //Variables
  byte i;
  bool goodfile;
  FileTree *nextBranch;
  FileTree *prevhead;

  if (currentlyopen == true){
    Serial.println("Error, a file is currently open");
    return;
  }

  //Clear out linked list
  if (head->nextfile != NULL){
    prevhead = head->nextfile;
    while (prevhead->nextfile != NULL){
      nextBranch = prevhead->nextfile;
      free(prevhead);
      prevhead = nextBranch;
    }
    head->nextfile = NULL;
  }

  //Attempt to open root directory
  if (!root.open("/")){
    Serial.println("Open root failed");
    SDError();
    return;
  }   
  
  //Ignore first file - System Volume Information
  file.openNext(&root, O_RDONLY);
  file.close();
  
  //Open each file
  while (file.openNext(&root, O_RDONLY)){

    if (*(head->filename) != 0){
      //Generate next item in linked list
      nextBranch = malloc(sizeof(FileTree));
      head->nextfile = nextBranch;

      //Move to next item in list
      head = nextBranch;
      head->nextfile = NULL;
    }

    //Clear buffer
    for (i = 0; i < 30; *(head->filename + i++) = 0){}

    //Load name into FileTree
    goodfile = file.getName(head->filename, 30);

    //Close the opened file
    file.close();

  }

  root.close();
  
}

void readLine(SdFile *thefile, char *dest){
  byte i;

  if (!thefile->available()){
    currentlyopen = false;
    return;
  }

  for (i = 0; i < 45; i++){
    *(dest + i) = 0;
  }
  
  *(dest) = thefile->read();
  
  if (*(dest) != ';' && *(dest) != 'M' && *(dest) != 'G'){
    while (*(dest) != ';' && *(dest) != 'M' && *(dest) != 'G' && thefile->available()){
      *(dest) = thefile->read();
    }
  }

  if (!thefile->available()){
    currentlyopen = false;
    return;
  }
  
  for (i = 1; i < 45 && thefile->available(); i++){
    *(dest + i) = thefile->read();
    if (*(dest + i) == '\r'){
      *(dest + i) = '\0';
      break;
    }
  }
  
  Serial.println(dest);

  if (!thefile->available()){
    currentlyopen = false;
    return;
  }
}

byte getNumberOfFiles(FileTree* head){
  byte i;

  for (i = 0; i < 100 && head->nextfile != NULL; i++){
    head = head->nextfile;
  }

  return i;
}

void writeCharToFile(SdFile *thefile, char writechar){
  thefile->write(writechar);
}

void SDError(){
  Serial.println("Error with SD card!");
  while(1){};   //For debugging, do proper error handling later//////////////////////////////////////////////////
}

bool doesExist(char* filename){
  if (sd.exists(filename)){
    return true;
  } else {
    return false;
  }
}

