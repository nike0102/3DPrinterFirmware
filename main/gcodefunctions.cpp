//Includes
#include "gcodefunctions.h"

//Variables
uint8_t X_Motor_Direction = Clockwise, Y_Motor_Direction = Clockwise, Z_Motor_Direction = Clockwise, E1_Motor_Direction = Clockwise;
float Head_X, Head_Y, Head_Z, Head_E1;

//1 = Full, 2 = 1/2, 4 = 1/4, 8 = 1/8, 16 = 1/16
//uint8_t stepMode = 1;
float MM_Per_Step = 8 / 200;//((float)stepMode * 200);

bool currentlyPrinting = false;
bool doneWithCommand = false;
char cmdBuffer[25] = {0};
static SdFile SDReadFile;

const short NTC3950ThermistorTable[34][2] PROGMEM = {
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

//G1 and G2 - Linear Move
void linearMove(struct Gcommand *currentGCommand) {

  float X_Speed, Y_Speed, Z_Speed, X_Change, Y_Change, Z_Change;
  uint8_t X_done = False, Y_done = False, Z_done = False, X_Enabled = False, Y_Enabled = False, Z_Enabled = False;
  
  int X_Timer = 0, X_Pulse_Timer = 0, Y_Timer = 0, Y_Pulse_Timer = 0, Z_Timer = 0, Z_Pulse_Timer = 0, Timer_Difference;
  int X_Time_Watch = 0, Y_Time_Watch = 0, Z_Time_Watch = 0;
  unsigned long Global_Timer = 0, Old_Global_Timer = 0;


  //Check Directions
  if (Head_X > currentGCommand->x) {
    reverseMotor(X_Motor_Direction);
    X_Change = Head_X - currentGCommand->x;
  } else {
    X_Change = currentGCommand->x - Head_X;
  }

  if (Head_Y > currentGCommand->y) {
    reverseMotor(Y_Motor_Direction);
    Y_Change = Head_Y - currentGCommand->y;
  } else {
    Y_Change = currentGCommand->y - Head_Y;
  }

  if (Head_Z > currentGCommand->z) {
    reverseMotor(Z_Motor_Direction);
    Z_Change = Head_Z - currentGCommand->z;
  } else {
    Z_Change = currentGCommand->z - Head_Z;
  }



  //Three axis move
  if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {

    if (X_Change > Y_Change) {
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

  } else

    //Two axis move - X and Y
    if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y) {
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
          if (Head_X != currentGCommand->x) {
            X_Speed = currentGCommand->f;
            Y_done = 1;
            Z_done = 1;
          } else

            //One axis move - Y
            if (Head_Y != currentGCommand->y) {
              X_done = 1;
              Y_Speed = currentGCommand->f;
              Z_done = 1;
            } else

              //One axis move - Z
              if (Head_Z != currentGCommand->z) {
                X_done = 1;
                Y_done = 1;
                Z_Speed = currentGCommand->f;
              } else {

                //Error
                return;
              }

  //Determine the timer values in microseconds
  //Makes sure there's no divide by zero error
  if (X_done == 0) {
    X_Timer = ((double)1 / (double)X_Speed) * 1000;
  } else {
    X_Timer = -1;
  }
  if (Y_done == 0) {
    Y_Timer = ((double)1 / (double)Y_Speed) * 1000;
  } else {
    Y_Timer = -1;
  }
  if (Z_done == 0) {
    Z_Timer = ((double)1 / (double)Z_Speed) * 1000;
  } else {
    Z_Timer = -1;
  }

  Global_Timer = micros();

  //Do the moving
  while (X_done != 1 && Y_done != 1 && Z_done != 1) {

    //Do time math
    Old_Global_Timer = Global_Timer;
    Global_Timer = micros();
    Timer_Difference = Global_Timer - Old_Global_Timer;

    //Protect against overflow
    if (Timer_Difference < 0) {
      //Timer_Difference =  4294967295 - Global_Timer;
    }

    //Increment timer watch values
    X_Time_Watch += Timer_Difference;
    Y_Time_Watch += Timer_Difference;
    Z_Time_Watch += Timer_Difference;

    //Lower X on-time timer
    if (X_Enabled == True) {
      X_Pulse_Timer -= Timer_Difference;
    }

    //Turn X on
    if (X_Timer != -1 && X_Enabled == False && X_Time_Watch >= X_Timer) {
      //digitalWrite(X_Axis_Step_Pin, HIGH);
      X_Pulse_Timer = Min_Pulse_Width;
      X_Enabled = True;
    }

    //Turn X off
    if (X_Enabled == True && X_Pulse_Timer <= 0) {
      //digitalWrite(X_Axis_Step_Pin, LOW);
      X_Enabled = False;
    }

    //Lower Y on-time timer
    if (Y_Enabled == True) {
      Y_Pulse_Timer -= Timer_Difference;
    }

    //Turn Y on
    if (Y_Timer != -1 && Y_Enabled == False && Y_Time_Watch >= Y_Timer) {
      //digitalWrite(Y_Axis_Step_Pin, HIGH);
      Y_Pulse_Timer = Min_Pulse_Width;
      Y_Enabled = True;
    }

    //Turn Y off
    if (Y_Enabled == True && Y_Pulse_Timer <= 0) {
      //digitalWrite(Y_Axis_Step_Pin, LOW);
      Y_Enabled = False;
    }

    //Lower Z on-time timer
    if (Z_Enabled == True) {
      Z_Pulse_Timer -= Timer_Difference;
    }

    //Turn Z on
    if (Z_Timer != -1 && Z_Enabled == False && Z_Time_Watch >= Z_Timer) {
      //digitalWrite(Z_Axis_Step_Pin, HIGH);
      Z_Pulse_Timer = Min_Pulse_Width;
      Z_Enabled = True;
    }

    //Turn Z off
    if (Z_Enabled == True && Z_Pulse_Timer <= 0) {
      //digitalWrite(Z_Axis_Step_Pin, LOW);
      Z_Enabled = False;
    }

    //Check if each axis is complete
    if (X_done == 0) {
      X_done = checkIfDone(currentGCommand->x - Head_X);
    }

    if (Y_done == 0) {
      Y_done = checkIfDone(currentGCommand->y - Head_Y);
    }

    if (Z_done == 0) {
      Z_done = checkIfDone(currentGCommand->z - Head_Z);
    }
    
  }

}

//Reverses the given motor's direction
void reverseMotor(uint8_t Motor) {
  if (Motor == Clockwise) {
    Motor = CounterClockwise;
  } else {
    Motor = Clockwise;
  }
}

//Checks if theres less than a step left
//If so, figure that it's done
uint8_t checkIfDone(float MM_Left){
  if (MM_Left < 0){
    MM_Left *= -1;
  }
  
  if (MM_Left < MM_Per_Step){
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
  
  readLine(&SDReadFile, cmdBuffer);

  if (cmdBuffer[0] == ';'){ //ignore comments
    continueSDPrint();
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
        X_Motor_Direction = CounterClockwise;
        amount *= -1;
      } else {
        digitalWrite(X_Axis_Dir_Pin, LOW);
        X_Motor_Direction = Clockwise;
      }
      break;
    case 1:
      if (amount < 0){
        digitalWrite(Y_Axis_Dir_Pin, HIGH);
        Y_Motor_Direction = CounterClockwise;
        amount *= -1;
      } else {
        digitalWrite(Y_Axis_Dir_Pin, LOW);
        Y_Motor_Direction = Clockwise;
      }
      break;
    case 2:
      if (amount < 0){
        digitalWrite(Z1_Axis_Dir_Pin, HIGH);
        digitalWrite(Z2_Axis_Dir_Pin, HIGH);
        Z_Motor_Direction = CounterClockwise;
        amount *= -1;
      } else {
        digitalWrite(Z1_Axis_Dir_Pin, LOW);
        digitalWrite(Z2_Axis_Dir_Pin, LOW);
        Z_Motor_Direction = Clockwise;
      }
      break;
    case 3:
      if (amount < 0){
        digitalWrite(E_Axis_Dir_Pin, HIGH);
        E1_Motor_Direction = CounterClockwise;
        amount *= -1;
      } else {
        digitalWrite(E_Axis_Dir_Pin, LOW);
        E1_Motor_Direction = Clockwise;
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
   
}

short getTemperature(int ADCVal){ //Returns temperature in C. Returns 0 if too cold, and -1 if too hot
  bool done = 0;
  byte i;
  short T, bigA, smallA, bigT, smallT;
  float Tslope, calV;
  
  //Check for exact match
    for (i = 0; (i < 34) && (done == 0); i++){
        if (ADCVal == NTC3950ThermistorTable[i][0]){
          T = NTC3950ThermistorTable[i][1];
          done = 1;
        }
    }
    
    //Check if too cold
    if (ADCVal < 30){
      done = 1;
      T = 0;
    }
      
    //Check if too hot
    if (ADCVal > 1010){
      done = 1;
      T = -1;
    }

    //If no exact match, do interpolation
    if (done == 0){
      
      //Find ranges
      for (i = 0; (i < 34) && (done == 0); i++){
          if (NTC3950ThermistorTable[i][0] > ADCVal){
            bigA = NTC3950ThermistorTable[i][0];
            bigT = NTC3950ThermistorTable[i][1];
            smallA = NTC3950ThermistorTable[i - 1][0];
            smallT = NTC3950ThermistorTable[i - 1][1];
            done = 1;
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
    
    if (heaterNum == 0){
      digitalWrite(Extruder_Heater_Pin, 1);
    } else if (heaterNum == 0){
      digitalWrite(Bed_Heater_Pin, 1);
    } else {
      return;
    }
    
  }

  if (currentTemp > Temp + 1){
    //Turn off heater

    if (heaterNum == 0){
      digitalWrite(Extruder_Heater_Pin, 0);
    } else if (heaterNum == 0){
      digitalWrite(Bed_Heater_Pin, 0);
    } else {
      return;
    }
    
  }
}

