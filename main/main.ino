//Config.h
#define Clockwise 0
#define CounterClockwise 1

#define X_Axis_Step_Pin 3
#define X_Axis_Dir_Pin  4
#define X_Axis_MS1_Pin  5
#define X_Axis_MS2_Pin  6
#define X_Axis_MS3_Pin  7

#define X_Axis_Dir_Config CounterClockwise

//#include "Config.h"
//#include "parser.h"

float Head_X, Head_Y, Head_Z, Head_E;
bool X_Motor_Direction = X_Axis_Dir_Config, Y_Motor_Direction, Z_Motor_Direction, E_Motor_Direction;

void setup() {

}

void loop() {
  
}

//Checks for an input on the controller
void checkInput(){
  
}

struct Gcommand{
  float x;
  float y;
  float z;
  float feed;
  float extrude;
  int number;
  char letter;
};

//Parses GCode string and places the information in a returned struct
struct Gcommand* readGCodeString(char* command, struct Gcommand *returnGCommand){
  
  return returnGCommand;
}

//Executes the given GCode command and returns a 1 on success
bool executeGCode(struct Gcommand *currentGCommand){
  
  //Temporary variables
  float X_Speed, Y_Speed, Z_Speed, X_Change, Y_Change, Z_Change;
  
  if (currentGCommand->letter == 'M'){
    
    //Move
    if (currentGCommand->number == 0 || currentGCommand->number == 1){

      //Check Directions
      if (Head_X > currentGCommand->x){
        reverseMotor(X_Motor_Direction);    
        X_Change = Head_X - currentGCommand->x;    
      } else {
        X_Change = currentGCommand->x - Head_X;
      }
      
      if (Head_Y > currentGCommand->y){
        reverseMotor(Y_Motor_Direction);
        Y_Change = Head_Y - currentGCommand->y;
      } else {
        Y_Change = currentGCommand->y - Head_Y;
      }
      
      if (Head_Z > currentGCommand->z){
        reverseMotor(Z_Motor_Direction);
        Z_Change = Head_Z - currentGCommand->z;
      } else {
        Z_Change = currentGCommand->z - Head_Z;
      }

      //Set the movement rate for each axis
      
      //Find biggest move
      //Determine time to move that much
      //Divide smaller moves so they end up at same time

      //Three axis move
      if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z){
        if (X_Change > Y_Change){
          
        } else

        if (Y_Change > X_Change){
          
        } else 

        if (X_Change > Z_Change){
          
        } else

        if (Z_Change > X_Change){
          
        } else 

        if (Y_Change > Z_Change){
          
        } else

        if (Z_Change > Y_Change){
          
        } else {
          //Error
          return 0;
        }
      } else

      //Two axis move - X and Y
      if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y){
        if (Y_Change > X_Change){
          Y_Speed = currentGCommand->feed;
          X_Speed = currentGCommand->feed / (Y_Change / X_Change);
        } else
        if (X_Change > Y_Change){
          Y_Speed = currentGCommand->feed / (X_Change / Y_Change);
          X_Speed = currentGCommand->feed;
        } else {
          Y_Speed = currentGCommand->feed;
          X_Speed = currentGCommand->feed;
        }
      } else

      //Two axis move - X and Z
      if (Head_X != currentGCommand->x && Head_Z != currentGCommand->z){
        if (X_Change > Z_Change){
          X_Speed = currentGCommand->feed;
          Z_Speed = currentGCommand->feed / (X_Change / Z_Change);
        } else
        if (Z_Change > X_Change){
          X_Speed = currentGCommand->feed / (Z_Change / X_Change);
          Z_Speed = currentGCommand->feed;
        } else {
          X_Speed = currentGCommand->feed;
          Z_Speed = currentGCommand->feed;
        }
      } else

      //Two axis move - Y and Z
      if (Head_Y != currentGCommand->y && Head_Z != currentGCommand->z){
        if (Y_Change > Z_Change){
          Y_Speed = currentGCommand->feed;
          Z_Speed = currentGCommand->feed / (Y_Change / Z_Change);
        } else
        if (Z_Change > Y_Change){
          Y_Speed = currentGCommand->feed / (Z_Change / Y_Change);
          Z_Speed = currentGCommand->feed;
        } else {
          Y_Speed = currentGCommand->feed;
          Z_Speed = currentGCommand->feed;
        }
      } else

      //One axis move - X
      if (Head_X != currentGCommand->x){
        X_Speed = currentGCommand->feed;
        Y_Speed = 0;
        Z_Speed = 0;
      } else
      
      //One axis move - Y
      if (Head_Y != currentGCommand->y){
        X_Speed = 0;
        Y_Speed = currentGCommand->feed;
        Z_Speed = 0;
      } else

      //One axis move - Z
      if (Head_Z != currentGCommand->z){
        X_Speed = 0;
        Y_Speed = 0;
        Z_Speed = currentGCommand->feed;
      } else {

        //Error
        return 0;
      }


      //Do the moving
      while (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z && Head_E != currentGCommand->extrude){
        
      }

      //Success
      return 1;
    }
  }

  //Error
  return 0;
}

//Reverses the given motor's direction
void reverseMotor(bool Motor){
  if (Motor == Clockwise){
    Motor = CounterClockwise;
  } else {
    Motor = Clockwise;
  }
}

