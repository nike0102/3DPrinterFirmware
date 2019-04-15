//Includes
#include "gcodefunctions.h"

//Variables
float Head_X, Head_Y, Head_Z, Head_E1;
uint8_t X_Motor_Direction = X_Motor_Direction_Setting, Y_Motor_Direction = Y_Motor_Direction_Setting, Z_Motor_Direction = Z_Motor_Direction_Setting, E1_Motor_Direction = E1_Motor_Direction_Setting;
int Steps_Per_Rotation = 800;
float MM_Per_Step = 32.0 / (float)Steps_Per_Rotation;
float ZMM_Per_Step = 8.0 / (float)Steps_Per_Rotation;

bool currentlyPrinting = false;
bool doneWithCommand = false;
char cmdBuffer[45] = {0};
static SdFile SDReadFile;

const short NTC3950ThermistorTable[34][2] = {
  {30,0},
  {76,20},
  {93,25},
  {113,30},
  {223,50},
  {414,75},
  {613,100},
  {827,140},
  {853,145},
  {870,150},
  {884,155},
  {897,160},
  {909,165},
  {921,170},
  {931,175},
  {940,180},
  {948,185},
  {955,190},
  {962,195},
  {968,200},
  {972,205},
  {977,210},
  {981,215},
  {985,220},
  {988,225},
  {991,230},
  {994,235},
  {997,240},
  {1001,250},
  {1003,255},
  {1004,260},
  {1006,265},
  {1007,270},
  {1010,280}
};

//Functions

//G0 and G1 - Linear Move   G0 has no extrusion
void linearMove(struct Gcommand *currentGCommand) {

  float X_Speed, Y_Speed, Z_Speed, X_Change, Y_Change, Z_Change, E_Change, E_Speed, Old_E_Head = Head_E1;
  uint8_t X_done = False, Y_done = False, Z_done = False, X_Enabled = False, Y_Enabled = False, Z_Enabled = False;
  
  long X_Timer = 0, X_Pulse_Timer = 0, Y_Timer = 0, Y_Pulse_Timer = 0, Z_Timer = 0, Z_Pulse_Timer = 0, Timer_Difference;
  long X_Time_Watch = 0, Y_Time_Watch = 0, Z_Time_Watch = 0;

  long E_Timer = 0, E_Pulse_Timer = 0, E_Time_Watch = 0;
  uint8_t E_Enabled = False, E_done = False;

  unsigned long Global_Timer = 0, Old_Global_Timer = 0;
  
  
  if (currentGCommand->xf == 0 && currentGCommand->yf == 0 && currentGCommand->zf == 0 && currentGCommand->ef == 0){
    return;
  }

  //Need to implement relative positioning. This line allows for negative relative commands to be ignored
  //So the microcontroller does not get stuck in an infinite loop
  if (currentGCommand->x < 0 || currentGCommand->y < 0 || currentGCommand->z < 0 || currentGCommand->e < 0){
    return;
  }


  //Check Directions
  if (Head_X > currentGCommand->x) {
    digitalWrite(X_Axis_Dir_Pin, LOW);
    X_Motor_Direction = CounterClockwise;
    X_Change = Head_X - currentGCommand->x;
  } else {
    digitalWrite(X_Axis_Dir_Pin, HIGH);
    X_Motor_Direction = Clockwise;
    X_Change = currentGCommand->x - Head_X;
  }

  if (Head_Y > currentGCommand->y) {
    digitalWrite(Y_Axis_Dir_Pin, LOW);
    Y_Motor_Direction = CounterClockwise;
    Y_Change = Head_Y - currentGCommand->y;
  } else {
    Y_Motor_Direction = Clockwise;
    digitalWrite(Y_Axis_Dir_Pin, HIGH);
    Y_Change = currentGCommand->y - Head_Y;
  }

  if (Head_Z > currentGCommand->z) {
    digitalWrite(Z1_Axis_Dir_Pin, HIGH);
    digitalWrite(Z2_Axis_Dir_Pin, HIGH);
    Z_Motor_Direction = Clockwise;
    Z_Change = Head_Z - currentGCommand->z;
  } else {
    digitalWrite(Z1_Axis_Dir_Pin, LOW);
    digitalWrite(Z2_Axis_Dir_Pin, LOW);
    Z_Motor_Direction = CounterClockwise;
    Z_Change = currentGCommand->z - Head_Z;
  }

  if (Head_E1 > currentGCommand->e) {
    digitalWrite(E_Axis_Dir_Pin, HIGH);
    E1_Motor_Direction = CounterClockwise;
    E_Change = Head_E1 - currentGCommand->e;
  } else {
    digitalWrite(E_Axis_Dir_Pin, LOW);
    E1_Motor_Direction = Clockwise;
    E_Change = currentGCommand->e - Head_E1;
  }
  
  //Three axis move -> Doesn't work
  if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {
    X_done = 0;
    Y_done = 0;
    Z_done = 0;
    
    /*if (X_Change > Y_Change) {
      if (Y_Change > Z_Change) {
        //X >
        X_Speed = currentGCommand->f;
        Y_Speed = currentGCommand->f / (X_Change / Y_Change);
        Z_Speed = currentGCommand->f / (X_Change / Z_Change);
      } else {
        //Z >
        X_Speed = currentGCommand->f / (Z_Change / X_Change);
        Y_Speed = currentGCommand->f / (Z_Change / Y_Change);
        Z_Speed = currentGCommand->f;
      }
    } else {
      if (Y_Change > Z_Change) {
        //Y >
        X_Speed = currentGCommand->f / (Y_Change / X_Change);
        Y_Speed = currentGCommand->f;
        Z_Speed = currentGCommand->f / (Y_Change / Z_Change);
      } else {
        //Z >
        X_Speed = currentGCommand->f / (Z_Change / X_Change);
        Y_Speed = currentGCommand->f / (Z_Change / Y_Change);
        Z_Speed = currentGCommand->f;
      }
    }
  */
  
  if (X_Change >= Y_Change && X_Change >= Z_Change){  //X is biggest
      X_Speed = currentGCommand->f;
        Y_Speed = currentGCommand->f / (X_Change / Y_Change);
        Z_Speed = currentGCommand->f / (X_Change / Z_Change);
    }

    if (Y_Change >= X_Change && Y_Change >= Z_Change){  //Y is biggest
      X_Speed = currentGCommand->f / (Y_Change / X_Change);
        Y_Speed = currentGCommand->f;
        Z_Speed = currentGCommand->f / (Y_Change / Z_Change);
    }

    if (Z_Change >= X_Change && Z_Change >= Y_Change){  //Z is biggest
      X_Speed = currentGCommand->f / (Z_Change / X_Change);
        Y_Speed = currentGCommand->f / (Z_Change / Y_Change);
        Z_Speed = currentGCommand->f;
    }
    
  } else

    //Two axis move - X and Y
    if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y) {
      X_done = 0;
      Y_done = 0;
      if (Y_Change > X_Change) {
        Y_Speed = currentGCommand->f;
        X_Speed = currentGCommand->f / (Y_Change / X_Change);
        Z_done = 1;
      } else if (X_Change > Y_Change) {
        Y_Speed = currentGCommand->f / (X_Change / Y_Change);
        X_Speed = currentGCommand->f;
        Z_done = 1;
      } else {
        Y_Speed = currentGCommand->f;
        X_Speed = currentGCommand->f;
        Z_done = 1;
      }
    } else

      //Two axis move - X and Z
      if (Head_X != currentGCommand->x && Head_Z != currentGCommand->z) {
        X_done = 0;
        Z_done = 0;
        if (X_Change > Z_Change) {
          X_Speed = currentGCommand->f;
          Y_done = 1;
          Z_Speed = currentGCommand->f / (X_Change / Z_Change);
        } else if (Z_Change > X_Change) {
          X_Speed = currentGCommand->f / (Z_Change / X_Change);
          Y_done = 1;
          Z_Speed = currentGCommand->f;
        } else {
          X_Speed = currentGCommand->f;
          Y_done = 1;
          Z_Speed = currentGCommand->f;
        }
      } else

        //Two axis move - Y and Z
        if (Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {
          Z_done = 0;
          Y_done = 0;
          if (Y_Change > Z_Change) {
            X_done = 1;
            Y_Speed = currentGCommand->f;
            Z_Speed = currentGCommand->f / (Y_Change / Z_Change);
          } else if (Z_Change > Y_Change) {
            X_done = 1;
            Y_Speed = currentGCommand->f / (Z_Change / Y_Change);
            Z_Speed = currentGCommand->f;
          } else {
            X_done = 1;
            Y_Speed = currentGCommand->f;
            Z_Speed = currentGCommand->f;
          }
        } else

          //One axis move - X
          if (Head_X != currentGCommand->x){
            X_Speed = currentGCommand->f;
            X_done = 0;
            Y_done = 1;
            Z_done = 1;
          } else

            //One axis move - Y
            if (Head_Y != currentGCommand->y) {
              X_done = 1;
              Y_done = 0;
              Y_Speed = currentGCommand->f;
              Z_done = 1;
            } else

              //One axis move - Z
              if (Head_Z != currentGCommand->z) {
                X_done = 1;
                Y_done = 1;
                Z_done = 0;
                Z_Speed = currentGCommand->f;
              } else {
                //E only
              }

  
  if (X_done == 0) {
    X_Timer = ((double)1.0 / (double)X_Speed) * 1000000.0;
    X_Timer *= MM_Per_Step;     //Microstepping factor
  } else {
    X_Timer = -1;
  }
  if (Y_done == 0) {
    Y_Timer = ((double)1.0 / (double)Y_Speed) * 1000000.0;
    Y_Timer *= MM_Per_Step;     //Microstepping factor
  } else {
    Y_Timer = -1;
  }
  if (Z_done == 0) {
    Z_Timer = ((double)1.0 / (double)Z_Speed) * 1000000.0;
    Z_Timer *= ZMM_Per_Step;     //Microstepping factor
  } else {
    Z_Timer = -1;
  }
  if ((int)currentGCommand->num == 0){
    E_done = 1;
    E_Timer = -1;
  } else {
    if (currentGCommand->ef == 0){
      E_done = 1;
      E_Timer = -1;
    } else {
      E_done = 0;

    //Find biggest timer
    if (X_Timer >= Y_Timer && X_Timer >= Z_Timer && X_Timer != -1){  //X is biggest
      E_Timer = (X_Timer * (X_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (Y_Timer >= X_Timer && Y_Timer >= Z_Timer && Y_Timer != -1){  //Y is biggest
      E_Timer = (Y_Timer * (Y_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (Z_Timer >= X_Timer && Z_Timer >= Y_Timer && Z_Timer != -1){  //Z is biggest
      E_Timer = (Z_Timer * (Z_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (X_Timer == -1 && Y_Timer == -1 && Z_Timer == -1){ //Only an extrude
      E_Timer = ((double)1.0 / (double)E_Change) * 1000000.0;
      E_Timer *= MM_Per_Step;     //Microstepping factor
    }
    }

  }
  
  Global_Timer = micros();

  //Do the moving
  while (X_done == 0 || Y_done == 0 || Z_done == 0 || E_done == 0) {

    //Do time math
    Old_Global_Timer = Global_Timer;
    Global_Timer = micros();
    Timer_Difference = Global_Timer - Old_Global_Timer;

    //Protect against overflow
    if (Timer_Difference < 0) {
      Timer_Difference =  4294967295 - Global_Timer;
    }

    //Increment timer watch values
    X_Time_Watch += Timer_Difference;
    Y_Time_Watch += Timer_Difference;
    Z_Time_Watch += Timer_Difference;
    E_Time_Watch += Timer_Difference;

    //Lower X on-time timer
    if (X_Enabled == True) {
      X_Pulse_Timer -= Timer_Difference;
    }

    //Turn X on
    if (X_Timer != -1 && X_Enabled == False && X_Time_Watch >= X_Timer) {
      digitalWrite(X_Axis_Step_Pin, HIGH);
      X_Pulse_Timer = Min_Pulse_Width;
      X_Time_Watch = 0;
      X_Enabled = True;
    }

    //Turn X off
    if (X_Enabled == True && X_Pulse_Timer <= 0) {
      digitalWrite(X_Axis_Step_Pin, LOW);
      if (X_Motor_Direction == X_Motor_Direction_Setting){
        Head_X += MM_Per_Step;
      } else {
        Head_X -= MM_Per_Step;
      }
      X_Enabled = False;
    }

    //Lower Y on-time timer
    if (Y_Enabled == True) {
      Y_Pulse_Timer -= Timer_Difference;
    }

    //Turn Y on
    if (Y_Timer != -1 && Y_Enabled == False && Y_Time_Watch >= Y_Timer) {
      digitalWrite(Y_Axis_Step_Pin, HIGH);
      Y_Pulse_Timer = Min_Pulse_Width;
      Y_Time_Watch = 0;
      Y_Enabled = True;
    }

    //Turn Y off
    if (Y_Enabled == True && Y_Pulse_Timer <= 0) {
      digitalWrite(Y_Axis_Step_Pin, LOW);
      if (Y_Motor_Direction == Y_Motor_Direction_Setting){
        Head_Y += MM_Per_Step;
      } else {
        Head_Y -= MM_Per_Step;
      }
      Y_Enabled = False;
    }

    //Lower Z on-time timer
    if (Z_Enabled == True) {
      Z_Pulse_Timer -= Timer_Difference;
    }

    //Turn Z on
    if (Z_Timer != -1 && Z_Enabled == False && Z_Time_Watch >= Z_Timer) {
      digitalWrite(Z1_Axis_Step_Pin, HIGH);
      digitalWrite(Z2_Axis_Step_Pin, HIGH);
      Z_Pulse_Timer = Min_Pulse_Width;
      Z_Time_Watch = 0;
      Z_Enabled = True;
    }

    //Turn Z off
    if (Z_Enabled == True && Z_Pulse_Timer <= 0) {
      digitalWrite(Z1_Axis_Step_Pin, LOW);
      digitalWrite(Z2_Axis_Step_Pin, LOW);
      if (Z_Motor_Direction == Z_Motor_Direction_Setting){
        Head_Z += ZMM_Per_Step;
      } else {
        Head_Z -= ZMM_Per_Step;
      }
      Z_Enabled = False;
    }

    //Lower E on-time timer
    if (E_Enabled == True) {
      E_Pulse_Timer -= Timer_Difference;
    }

    //Turn E on
    if (E_Timer != -1 && E_Enabled == False && E_Time_Watch >= E_Timer) {
      digitalWrite(E_Axis_Step_Pin, HIGH);
      E_Pulse_Timer = Min_Pulse_Width;
      E_Enabled = True;
      E_Time_Watch = 0;
    }

    //Turn E off
    if (E_Enabled == True && E_Pulse_Timer <= 0) {
      digitalWrite(E_Axis_Step_Pin, LOW);
      if (E1_Motor_Direction == E1_Motor_Direction_Setting){
        Head_E1 += MM_Per_Step;
      } else {
        Head_E1 -= MM_Per_Step;
      }
      E_Enabled = False;
    }

    //Check if each axis is complete
    if (X_done == 0) {
      X_done = checkIfDone(currentGCommand->x - Head_X);
      if (X_done == 1){
        X_Timer = -1;
      }
    }

    if (Y_done == 0) {
      Y_done = checkIfDone(currentGCommand->y - Head_Y);
      if (Y_done == 1){
        Y_Timer = -1;
      }
    }

    if (Z_done == 0) {
      Z_done = checkIfZDone(currentGCommand->z - Head_Z);
      if (Z_done == 1){
        Z_Timer = -1;
      }
    }

    if (E_done == 0) {
      E_done = checkIfDone(currentGCommand->e - Head_E1);
      if (E_done == 1){
        E_Timer = -1;
      }
    }
    
  }

}

//Checks if theres less than a step left
//If so, figure that it's done
uint8_t checkIfDone(float MM_Left){
  if (MM_Left < 0.0){
    MM_Left *= -1;
  }
  
  if ((MM_Left < MM_Per_Step - 0.00125) || (MM_Left == 0.0)){
    return 1;
  } else {
    return 0;
  }
}

//Checks if theres less than a step left
//If so, figure that it's done
uint8_t checkIfZDone(float MM_Left){
  if (MM_Left < 0.0){
    MM_Left *= -1;
  }
  
  if ((MM_Left < ZMM_Per_Step - 0.00125) || (MM_Left == 0.0)){
    return 1;
  } else {
    return 0;
  }
}

void selectSDFile(GCommand *command){               //M23

  uint8_t i;
  
  //Copy serial input into buffer
  for (i = 0; i < 25; i++){
    selectedSDFile[i] = *(command->msg + i);
    }

  //Verify that the file actually exists
  if (doesExist(selectedSDFile)){             
  //Success
    Serial.print(F("File "));
    Serial.print(selectedSDFile);
    Serial.println(F(" selected!"));
  } else {
    Serial.print(F("Sorry, but the file "));
    Serial.print(selectedSDFile);
    Serial.println(F(" does not exist."));
    for (i = 0; i < 25; i++){
      selectedSDFile[i] = 0;
    }
  }                                 
}

void startSDPrint(char* filename){                  //M24

  //Error Check
  if (filename[0] == 0){
    Serial.println(F("Error, no SD file selected!"));
    return;
  }

  //Check and set flags
  if (currentlyPrinting == true){
    Serial.println(F("Error, already printing!"));
    return;
  } else {
    currentlyPrinting = true;
  }

  if (currentlyopen == true){
    Serial.println(F("Error, file already open!"));
    return;
  } else {
    currentlyopen = true;
    SDReadFile.open(filename, O_RDONLY);
  }

  continueSDPrint();
}

void continueSDPrint(){

  int temp;
  NoCommand = false;
  
  readLine(&SDReadFile, cmdBuffer);
  
  if (cmdBuffer[0] == ';'){ //ignore comments
    NoCommand = true;
    return;
  }

  temp = readGCodeString(cmdBuffer, &currentGCodeCommand);

  if (currentlyopen == false){
    SDReadFile.close();
    Serial.println("Last line read!");
  }  
}

void manualMove(uint8_t axis, float amount){

  int i, steps;
  
  /*  Axis Values
   *  0 -> X
   *  1 -> Y
   *  2 -> Z
   *  3 -> E
   */

   //Check directions
   switch (axis){
    case 0:
      if (amount < 0){
        digitalWrite(X_Axis_Dir_Pin, HIGH);
        amount *= -1;
      } else {
        digitalWrite(X_Axis_Dir_Pin, LOW);
      }
      break;
    case 1:
      if (amount < 0){
        digitalWrite(Y_Axis_Dir_Pin, HIGH);
        amount *= -1;
      } else {
        digitalWrite(Y_Axis_Dir_Pin, LOW);
      }
      break;
    case 2:
      if (amount < 0){
        digitalWrite(Z1_Axis_Dir_Pin, HIGH);
        digitalWrite(Z2_Axis_Dir_Pin, HIGH);
        amount *= -1;
      } else {
        digitalWrite(Z1_Axis_Dir_Pin, LOW);
        digitalWrite(Z2_Axis_Dir_Pin, LOW);
      }
      break;
    case 3:
      if (amount < 0){
        digitalWrite(E_Axis_Dir_Pin, HIGH);
        amount *= -1;
      } else {
        digitalWrite(E_Axis_Dir_Pin, LOW);
      }
      break;
   }


   //Full step -> 0.04mm      All microstepping pins low
   //Half step -> 0.02mm      MS1 high, others low
   //Quarter step -> 0.01mm   MS2 high, others low

   //Determine microstepping and amount of steps needed
   if (fmod(amount, 0.04) == 0){         //Can full step
    digitalWrite(MS1_Pin, LOW);
    digitalWrite(MS2_Pin, LOW);
    digitalWrite(MS3_Pin, LOW);
    steps = amount / 0.04;
   } else if (fmod(amount, 0.02) == 0){   //Can half step
    digitalWrite(MS1_Pin, HIGH);
    digitalWrite(MS2_Pin, LOW);
    digitalWrite(MS3_Pin, LOW);
    steps = amount / 0.02;
   } else {                         //Have to quarter step
    digitalWrite(MS1_Pin, LOW);
    digitalWrite(MS2_Pin, HIGH);
    digitalWrite(MS3_Pin, LOW);
    steps = amount / 0.01;
   }

   //Do the movement
   switch (axis){
    case 0:
      for (i = 0; i < steps; i++){
        digitalWrite(X_Axis_Step_Pin, HIGH);
        delay(1);
        digitalWrite(X_Axis_Step_Pin, LOW);
        delay(1);
      }
      break;
    case 1:
      for (i = 0; i < steps; i++){
        digitalWrite(Y_Axis_Step_Pin, HIGH);
        delay(1);
        digitalWrite(Y_Axis_Step_Pin, LOW);
        delay(1);
      }
      break;
    case 2:
      for (i = 0; i < steps; i++){
        digitalWrite(Z1_Axis_Step_Pin, HIGH);
        digitalWrite(Z2_Axis_Step_Pin, HIGH);
        delay(1);
        digitalWrite(Z2_Axis_Step_Pin, LOW);
        digitalWrite(Z1_Axis_Step_Pin, LOW);
        delay(1);
      }
      break;
    case 3:
      for (i = 0; i < steps; i++){
        digitalWrite(E_Axis_Step_Pin, HIGH);
        delay(1);
        digitalWrite(E_Axis_Step_Pin, LOW);
        delay(1);
      }
      break;
   }

   Serial.println("Done moving");
   
}

short getTemperature(int ADCVal){ //Returns temperature in C. Returns 0 if too cold, and -1 if too hot
  bool done = false;
  byte i;
  short T, bigA, smallA, bigT, smallT;
  float Tslope, calV;
  
  //Check for exact match
    for (i = 0; (i < 34) && (done == false); i++){
        if (ADCVal == NTC3950ThermistorTable[i][0]){
          T = NTC3950ThermistorTable[i][1];
          done = true;
        }
    }
    
    //Check if too cold
    if (ADCVal < 30){
      done = true;
      T = 0;
    }
      
    //Check if too hot
    if (ADCVal > 1010){
      done = true;
      T = -1;    
    }

    //If no exact match, do interpolation
    if (done == false){
      //Find ranges
      for (i = 0; (i < 34) && (done == false); i++){
          if (NTC3950ThermistorTable[i][0] > ADCVal){
            bigA = NTC3950ThermistorTable[i][0];
            bigT = NTC3950ThermistorTable[i][1];
            smallA = NTC3950ThermistorTable[i - 1][0];
            smallT = NTC3950ThermistorTable[i - 1][1];
            done = true;
          }
      }
      
      //Calculate T
      Tslope = (float)((float)(bigT-smallT)/(float)(bigA-smallA)); //M = (Y2-Y1) / (X2-X1)
      T = Tslope * ADCVal + (smallT - Tslope * smallA); //Y = MX + B
    }

    return T;
}

void setTemperature(byte heaterNum, short Temp){  //0 = Bed, 1 = Extruder

  short currentTemp;

  if (heaterNum == 0){                     
    currentTemp = getTemperature(analogRead(Bed_Thermistor_Pin));
  } else if (heaterNum == 1){
    currentTemp = getTemperature(analogRead(Extruder_Thermistor_Pin));
  } else {
    Serial.println(F("Error! Bed or Extruder needs to be selected"));
    return;
  }

  if (currentTemp < Temp - 1){
    //Turn on heater
    
    if (heaterNum == 1){
      digitalWrite(Extruder_Heater_Pin, HIGH);
    } else if (heaterNum == 0){
      digitalWrite(Bed_Heater_Pin, HIGH);
    } else {
      return;
    }
    
  }

  if (currentTemp > Temp + 1){
    //Turn off heater

    if (heaterNum == 1){
      digitalWrite(Extruder_Heater_Pin, LOW);
    } else if (heaterNum == 0){
      digitalWrite(Bed_Heater_Pin, LOW);
    } else {
      return;
    }
    
  }
}

