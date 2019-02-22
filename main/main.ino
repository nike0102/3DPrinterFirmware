//Includes
#include "main.h"

byte i;
int readok;
HTMLRequest htreq;
WiFiEspServer server(80);
WiFiEspClient client;


void setup(){
	//Motor step pins
	//pinMode(X_Axis_Step_Pin, OUTPUT);
	//pinMode(Y_Axis_Step_Pin, OUTPUT);
	//pinMode(Z_Axis_Step_Pin, OUTPUT);

  Serial.begin(9600);

  //Attempt to initalize SD card
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    Serial.println("SD card initalization failed");
    SDError();
  }
  
  powerOn();
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
  if (client.available()) {                               // if you get a client,
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
  
  switch(command->letter){
    
    case 'G':
      switch((int)command->num){
        default:
          Serial.println(F("Error - Unsupported G command!"));
      }
      break;
      
    case 'M':
      switch((int)command->num){
        case 20:                                                //List files on SD Card
          Serial.println(F("Getting files on SD card..."));
          buildFileTree(&FilesOnSDCard);
          break;
        case 23:                                                //Select a file on SD Card
          Serial.print(F("Selecting file..."));
          selectSDFile(command);
          break;
        case 24:                                                  //Start a print based on selected SD file
          startSDPrint(selectedSDFile);
          handleCommand(&currentGCodeCommand);
          break;
        default:
          Serial.println(F("Error - Unsupported M command!"));
      }
      break;
      
    default:
      Serial.println(F("Error handling command!"));
  }
  
}

