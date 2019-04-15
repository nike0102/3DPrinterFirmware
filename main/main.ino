//Includes
#include "main.h"

bool NoCommand;
byte i;
char inputBuffer[45] = {0};
int readok;
static short BedTemp = 25, ExtruderTemp = 25;
long processed;
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

  //Force Microstepping to quarter stepping
  digitalWrite(MS1_Pin, LOW);
  digitalWrite(MS2_Pin, HIGH);
  digitalWrite(MS3_Pin, LOW);

  //Heater pins
  pinMode(Extruder_Heater_Pin, OUTPUT);
  pinMode(Bed_Heater_Pin, OUTPUT);

  //Thermistor pins
  pinMode(Extruder_Thermistor_Pin, INPUT);
  pinMode(Bed_Thermistor_Pin, INPUT);

  //Endstops  -> Error in motherboard, don't use
  //pinMode(X-Axis_Endstop, INPUT);
  //pinMode(Y-Axis_Endstop, INPUT);
  //pinMode(Z-Axis_Endstop, INPUT);
  
  //Start the Serial port for USB connections
  Serial.begin(9600);

  //Initalize the SD Card
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    Serial.println("SD card initalization failed");
    SDError();
  }
  
  //Start WiFi chip
  powerOnWiFi();  

  //Add a one second delay to make sure everything is ready before starting up
  delay(1000);

  Serial.println("Ready");

}

//Main loop

void loop(){

  //Check for USB commands
  if (Serial.available() > 0){
    getInput(true);
  }

  //Continue a print if one is started
  if (currentlyPrinting == true && doneWithCommand == true){
    processed++;
    doneWithCommand = false;
    continueSDPrint();
    handleCommand(&currentGCodeCommand);
  }

  //Handle WiFi connections
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
  
  //Make sure bed and extruder temperatures are kept within a degree
  if ((getTemperature(analogRead(Extruder_Thermistor_Pin)) < ExtruderTemp - 1) || (getTemperature(analogRead(Extruder_Thermistor_Pin)) > ExtruderTemp + 1)){
    setTemperature(1, ExtruderTemp);
  }
  if ((getTemperature(analogRead(Bed_Thermistor_Pin)) < BedTemp - 1) || (getTemperature(analogRead(Bed_Thermistor_Pin)) > BedTemp + 1)){
    setTemperature(0, BedTemp);
  }
  
  
}



/////////////////////////////////////////////////////////////////////////////////
//Function reads in serial input and decides what to do with it
/////////////////////////////////////////////////////////////////////////////////
void getInput(bool isSerial){

    if (isSerial == true){
      for (i = 0; i < 45; i++){
        inputBuffer[i] = 0;
      }
      i = 0;
      delay(100);
      while (Serial.available() > 0 && i < 45){
        inputBuffer[i++] = Serial.read();
      }
    }
    
    Serial.print("\n>> ");
    Serial.print(inputBuffer);
    
    if (inputBuffer[0] == '#'){  //Debugging
      if (inputBuffer[1] == 'n'){
        doneWithCommand = true;
      }
      if (inputBuffer[1] == 'x' && inputBuffer[2] == '+'){
        manualMove(0,10.0);
      }
      if (inputBuffer[1] == 'x' && inputBuffer[2] == '-'){
        manualMove(0,-10.0);
      }
      if (inputBuffer[1] == 'y' && inputBuffer[2] == '+'){
        manualMove(1,10.0);
      }
      if (inputBuffer[1] == 'y' && inputBuffer[2] == '-'){
        manualMove(1,-10.0);
      }
      if (inputBuffer[1] == 'z' && inputBuffer[2] == '+'){
        manualMove(2,10.0);
      }
      if (inputBuffer[1] == 'z' && inputBuffer[2] == '-'){
        manualMove(2,-10.0);
      }
      if (inputBuffer[1] == 'e' && inputBuffer[2] == '+'){
        manualMove(3,10.0);
      }
      if (inputBuffer[1] == 'e' && inputBuffer[2] == '-'){
        manualMove(3,-10.0);
      }
      if (inputBuffer[1] == 't'){
        short ad = analogRead(Extruder_Thermistor_Pin);
        Serial.print("\nExtruder Set Temperature = ");
        Serial.println(ExtruderTemp);
        Serial.print("Actual Temperature = ");
        Serial.println(getTemperature(ad));
        ad = analogRead(Bed_Thermistor_Pin);
        Serial.print("\nBed Set Temperature = ");
        Serial.println(BedTemp);
        Serial.print("Actual Temperature = ");
        Serial.println(getTemperature(ad));
      }
      if (inputBuffer[1] == 'h'){
        Head_X = 0;
        Head_Y = 0;
        Head_Z = 0;
        Head_E1 = 0;
        Serial.println("\nCurrent position set to home");
      }
      if (inputBuffer[1] == 'p'){
        Serial.print("\nHead_X = ");
        Serial.print(Head_X);
        Serial.print(" Head_Y = ");
        Serial.print(Head_Y);
        Serial.print(" Head_Z = ");
        Serial.print(Head_Z);
        Serial.print(" Head_E = ");
        Serial.println(Head_E1);
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

  if (NoCommand == true){
    doneWithCommand = true;
    return;
  }
  
  switch(command->letter){
    
    case 'G':
      switch((int)command->num){
        case 0:                                                //Linear Move without extrustion
            linearMove(command);
            doneWithCommand = true;
          break;
        case 1:                                                //Linear move with extrusion
            linearMove(command);
            doneWithCommand = true;
          break;
        case 28:                                               //Auto Home
          Serial.println(F("Error - Can not home using endstops due to motherboard errors"));
          Serial.println(F("Going to set home position"));

          if (command->xf == 0 && command->yf == 0 && command->zf == 0){  //Home all
            //First X and Y
            command->zf = 0;
            command->ef = 0;
            command->xf = 1;
            command->yf = 1;
            command->x = 0;
            command->y = 0;
            linearMove(command);

            //Then Z
            command->zf = 1;
            command->ef = 0;
            command->xf = 0;
            command->yf = 0;
            command->z = 0;
            linearMove(command);
          } else 
          if (command->xf == 0 && command->yf == 0 && command->zf == 1){  //Z only
            command->zf = 1;
            command->ef = 0;
            command->xf = 0;
            command->yf = 0;
            command->z = 0;
            linearMove(command);
          } else
          if (command->xf == 0 && command->yf == 1 && command->zf == 0){  //Y only
            command->zf = 0;
            command->ef = 0;
            command->xf = 0;
            command->yf = 1;
            command->y = 0;
            linearMove(command);
          } else 
          if (command->xf == 0 && command->yf == 1 && command->zf == 1){  //Z and Y
            command->zf = 1;
            command->ef = 0;
            command->xf = 0;
            command->yf = 1;
            command->z = 0;
            command->y = 0;
            linearMove(command);
          } else
          if (command->xf == 1 && command->yf == 0 && command->zf == 0){  //X only
            command->zf = 0;
            command->ef = 0;
            command->xf = 1;
            command->yf = 0;
            command->x = 0;
            linearMove(command);
          } else 
          if (command->xf == 1 && command->yf == 0 && command->zf == 1){  //X and Z
            command->zf = 1;
            command->ef = 0;
            command->xf = 1;
            command->yf = 0;
            command->z = 0;
            command->x = 0;
            linearMove(command);
          } else
          if (command->xf == 1 && command->yf == 1 && command->zf == 0){  //X and Y
            command->zf = 0;
            command->ef = 0;
            command->xf = 1;
            command->yf = 1;
            command->x = 0;
            command->y = 0;
            linearMove(command);
          } else {                                                        //Home all
            //First X and Y
            command->zf = 0;
            command->ef = 0;
            command->xf = 1;
            command->yf = 1;
            command->x = 0;
            command->y = 0;
            linearMove(command);

            //Then Z
            command->zf = 1;
            command->ef = 0;
            command->xf = 0;
            command->yf = 0;
            command->z = 0;
            linearMove(command);
          }
          
          doneWithCommand = true;
          break;
        case 92:                                               //Set Position of axis
            if (command->xf == 1){
              Head_X = command->x;
            }
            if (command->yf == 1){
              Head_Y = command->y;
            }
            if (command->zf == 1){
              Head_Z = command->z;
            }
            if (command->ef == 1){
              Head_E1 = command->e;
            }
            doneWithCommand = true;
          break;
        default:
          Serial.println(F("Error - Unsupported G command!"));
          doneWithCommand = true;
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
          doneWithCommand = true;
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
          doneWithCommand = true;
          break;
        case 22:                                                //Release SD Card
          //SD card is required, so no need to release it
          doneWithCommand = true;
          break;
        case 23:                                                //Select a file on SD Card
          Serial.print(F("Selecting file..."));
          selectSDFile(command);
          break;
        case 24:                                                //Start a print based on selected SD file
          //Error check
          if (selectedSDFile[0] == 0){
            Serial.println("Error - No file selected. Use \"M23 FILENAME.gcode\" before this command.");
            doneWithCommand;
            return;
          }
          processed = 1;
          startSDPrint(selectedSDFile);
          handleCommand(&currentGCodeCommand);
          doneWithCommand = true;
          break;
        case 104:                                               //Set Extruder Temperature
          setTemperature(1, (short)command->s);
          ExtruderTemp = (short)command->s;
          doneWithCommand = true;
          break;
        case 109:                                               //Wait for Extruder Temperature
          setTemperature(1, (short)command->s);
          ExtruderTemp = (short)command->s;
          while(doneWithCommand == false){
            if ((getTemperature(analogRead(Extruder_Thermistor_Pin)) >= ExtruderTemp - 1)){
              doneWithCommand = true;
           }
          }
          setTemperature(0, ExtruderTemp);
          Serial.println("Extruder has reached set temperature");
          break;
        case 140:                                               //Set Bed Temperature
          setTemperature(0, (short)command->s);
          BedTemp = (short)command->s;
          doneWithCommand = true;
          break;
        case 190:                                               //Wait for Bed Temperature
          setTemperature(0, (short)command->s);
          BedTemp = (short)command->s;
          while(doneWithCommand == false){
            if ((getTemperature(analogRead(Bed_Thermistor_Pin)) >= BedTemp - 1)){
              doneWithCommand = true;
            }
          }
          setTemperature(0, BedTemp);
          Serial.println("Bed has reached set temperature");
          break;
        case 524:                                               //Abort SD Print
          currentlyPrinting = false;
          SDReadFile.close();
          currentlyopen = false;
          break;
        default:
          Serial.println(F("Error - Unsupported M command!"));
          doneWithCommand = true;
      }
      break;
      
    default:
      Serial.println(F("Error handling command!"));
      doneWithCommand = true;
  }
  
}

