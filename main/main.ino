//Includes
#include "main.h"

byte i;
int readok;
HTMLRequest htreq;
WiFiEspServer server(80);
WiFiEspClient client;


void setup(){
  
	//Motor pins
  #ifndef JOSH  //Using A4988 motor driver
	pinMode(X_Axis_Step_Pin, OUTPUT);
	pinMode(Y_Axis_Step_Pin, OUTPUT);
	pinMode(Z1_Axis_Step_Pin, OUTPUT);
  pinMode(Z2_Axis_Step_Pin, OUTPUT);
  pinMode(E_Axis_Step_Pin, OUTPUT);
  pinMode(X_Axis_Dir_Pin, OUTPUT);
  pinMode(Y_Axis_Dir_Pin, OUTPUT);
  pinMode(Z1_Axis_Dir_Pin, OUTPUT);
  pinMode(Z2_Axis_Dir_Pin, OUTPUT);
  pinMode(E_Axis_Dir_Pin, OUTPUT);
  pinMode(MS1_Pin, OUTPUT);
  pinMode(MS2_Pin, OUTPUT);
  pinMode(MS3_Pin, OUTPUT);
  #else //Using Josh's custom motor drivers
  #endif

  Serial.begin(9600);

  //Attempt to initalize SD card
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    Serial.println("SD card initalization failed");
    SDError();
  }
  
  powerOnWiFi();  //Start WiFi chip
  delay(1000);

}

//Main loop

void loop(){


  if (Serial.available() > 0){
    getSerialInput();
  }

  if (currentlyPrinting == true && doneWithCommand == true){
    doneWithCommand = false;
    continueSDPrint();
    handleCommand(&currentGCodeCommand);
  }

  client = server.available();  // listen for incoming clients
  if (client.available()) {                               //If there's a connection
    readRequest(&htreq);

    if (htreq.type == 1){
      sendHttpResponseMain();
    }

    //Clear out HTTP Response struct
    htreq.type = 0;
    htreq.messagelength = -1;
    htreq.alreadyread = 0;
    htreq.currentlyReadingFile = false;
    for (i = 0; i < 25; i++){
      htreq.filename[i] = 0;
      htreq.link[i] = 0;
    }
    
    delay(50);
    client.stop();
  }

  
}



/////////////////////////////////////////////////////////////////////////////////
//Function reads in serial input and decides what to do with it
/////////////////////////////////////////////////////////////////////////////////
void getSerialInput(){
  for (i = 0; i < 25; i++){
      inputBuffer[i] = 0;
    }
    i = 0;
    delay(20);
    while (Serial.available() > 0 && i < 25){
      inputBuffer[i++] = Serial.read();
    }
    Serial.print("\n>> ");
    Serial.print(inputBuffer);
    
    if (inputBuffer[0] == '#'){  //Debugging
      if (inputBuffer[1] == 'n'){
        doneWithCommand = true;
      }
    } else {
      readok = readGCodeString(inputBuffer, &currentGCodeCommand);
      if (readok == 0){
        handleCommand(&currentGCodeCommand);
      }
    }
}


/////////////////////////////////////////////////////////////////////////////////
//Function handles a G or M code command
/////////////////////////////////////////////////////////////////////////////////
void handleCommand(GCommand *command){
  //Local variables
  byte i;
  FileTree *head = &FilesOnSDCard;
  
  switch(command->letter){
    
    case 'G':
      switch((int)command->num){
        case 0:                                                //Linear Move
        case 1:
        
          break;
        case 28:                                               //Auto Home
          Serial.println(F("Error - Can not home due to motherboard errors"));
          break;
        default:
          Serial.println(F("Error - Unsupported G command!"));
      }
      break;
      
    case 'M':
      switch((int)command->num){
        case 0:                                                 //Stop  (Treated the same as M524 - Abort SD Print
        case 1:
        case 112:
          currentlyPrinting = false;
          SDReadFile.close();
          currentlyopen = false;
          break;
        case 20:                                                //List files on SD Card
          Serial.println(F("Getting files on SD card..."));
          buildFileTree(&FilesOnSDCard);

          //Print out files
          while (head->nextfile != NULL){
            Serial.println(head->filename);
            head = head->nextfile;
          }
          break;
        case 21:                                                //Init SD Card
          //Initalizes on startup
          break;
        case 22:                                                //Release SD Card
          //SD card is required, so no need to release it
          break;
        case 23:                                                //Select a file on SD Card
          Serial.print(F("Selecting file..."));
          selectSDFile(command);
          break;
        case 24:                                                //Start a print based on selected SD file
          startSDPrint(selectedSDFile);
          handleCommand(&currentGCodeCommand);
          break;
        case 104:                                               //Set Extruder Temperature
          
          break;
        case 109:                                               //Wait for Extruder Temperature
          
          break;
        case 140:                                               //Set Bed Temperature
          
          break;
        case 190:                                               //Wait for Bed Temperature
          
          break;
        case 524:                                               //Abort SD Print
          currentlyPrinting = false;
          SDReadFile.close();
          currentlyopen = false;
          break;
        default:
          Serial.println(F("Error - Unsupported M command!"));
      }
      break;
      
    default:
      Serial.println(F("Error handling command!"));
  }
  
}

