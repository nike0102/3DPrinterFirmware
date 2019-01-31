#include "Config.h"
#include "Parser.h"

float Head_X, Head_Y, Head_Z, Head_E;
bool X_Motor_Direction = X_Axis_Dir_Config, Y_Motor_Direction, Z_Motor_Direction, E_Motor_Direction;

//1 = Full, 2 = 1/2, 4 = 1/4, 8 = 1/8, 16 = 1/16
byte stepMode = 1;

float MM_Per_Step = 8 / (stepMode * 200);

void setup() {
  pinMode(X_Axis_Step_Pin, OUTPUT);
  pinMode(X_Axis_Dir_Pin, OUTPUT);
  pinMode(Y_Axis_Step_Pin, OUTPUT);
  pinMode(Y_Axis_Dir_Pin, OUTPUT);
  pinMode(Z_Axis_Step_Pin, OUTPUT);
  pinMode(Z_Axis_Dir_Pin, OUTPUT);
}

void loop() {

}

//Checks for an input on the controller
void checkInput() {

}



//Moves everything linearly -> Add support for TFT Screen, Call a function?
void linearMove(struct Gcommand *currentGCommand) {

  //temporaryFloatVar variables
  float X_Speed, Y_Speed, Z_Speed, X_Change, Y_Change, Z_Change;
  bool done_X = false, done_Y = false, done_Z = false, X_enabled = false, Y_enabled = false, Z_enabled = false;
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

  //Set the movement rate for each axis
  //Find biggest move
  //Determine time to move that much
  //Divide smaller moves so they end up at same time
  /*
    Is X > Y
      Yes	Is X > Z
        Yes	X >
        No	Z >

      No	Is Y > Z
        Yes	Y >
        No	Z >
  */

  //Three axis move
  if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {

    if (X_Change > Y_Change) {
      if (Y_Change > Z_Change) {
        //X >
        X_Speed = currentGCommand->feed;
        Y_Speed = currentGCommand->feed / (X_Change / Y_Change);
        Z_Speed = currentGCommand->feed / (X_Change / Z_Change);
      } else {
        //Z >
        X_Speed = currentGCommand->feed / (Z_Change / X_Change);
        Y_Speed = currentGCommand->feed / (Z_Change / Y_Change);
        Z_Speed = currentGCommand->feed;
      }
    } else {
      if (Y_Change > Z_Change) {
        //Y >
        X_Speed = currentGCommand->feed / (Y_Change / X_Change);
        Y_Speed = currentGCommand->feed;
        Z_Speed = currentGCommand->feed / (Y_Change / Z_Change);
      } else {
        //Z >
        X_Speed = currentGCommand->feed / (Z_Change / X_Change);
        Y_Speed = currentGCommand->feed / (Z_Change / Y_Change);
        Z_Speed = currentGCommand->feed;
      }
    }

  } else

    //Two axis move - X and Y
    if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y) {
      if (Y_Change > X_Change) {
        Y_Speed = currentGCommand->feed;
        X_Speed = currentGCommand->feed / (Y_Change / X_Change);
        done_Z = 1;
      } else if (X_Change > Y_Change) {
        Y_Speed = currentGCommand->feed / (X_Change / Y_Change);
        X_Speed = currentGCommand->feed;
        done_Z = 1;
      } else {
        Y_Speed = currentGCommand->feed;
        X_Speed = currentGCommand->feed;
        done_Z = 1;
      }
    } else

      //Two axis move - X and Z
      if (Head_X != currentGCommand->x && Head_Z != currentGCommand->z) {
        if (X_Change > Z_Change) {
          X_Speed = currentGCommand->feed;
          done_Y = 1;
          Z_Speed = currentGCommand->feed / (X_Change / Z_Change);
        } else if (Z_Change > X_Change) {
          X_Speed = currentGCommand->feed / (Z_Change / X_Change);
          done_Y = 1;
          Z_Speed = currentGCommand->feed;
        } else {
          X_Speed = currentGCommand->feed;
          done_Y = 1;
          Z_Speed = currentGCommand->feed;
        }
      } else

        //Two axis move - Y and Z
        if (Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {
          if (Y_Change > Z_Change) {
            done_X = 1;
            Y_Speed = currentGCommand->feed;
            Z_Speed = currentGCommand->feed / (Y_Change / Z_Change);
          } else if (Z_Change > Y_Change) {
            done_X = 1;
            Y_Speed = currentGCommand->feed / (Z_Change / Y_Change);
            Z_Speed = currentGCommand->feed;
          } else {
            done_X = 1;
            Y_Speed = currentGCommand->feed;
            Z_Speed = currentGCommand->feed;
          }
        } else

          //One axis move - X
          if (Head_X != currentGCommand->x) {
            X_Speed = currentGCommand->feed;
            done_Y = 1;
            done_Z = 1;
          } else

            //One axis move - Y
            if (Head_Y != currentGCommand->y) {
              done_X = 1;
              Y_Speed = currentGCommand->feed;
              done_Z = 1;
            } else

              //One axis move - Z
              if (Head_Z != currentGCommand->z) {
                done_X = 1;
                done_Y = 1;
                Z_Speed = currentGCommand->feed;
              } else {

                //Error
                return 0;
              }

  //Determine the timer values in microseconds
  //Makes sure there's no divide by zero error
  if (done_X == 0) {
    X_Timer = ((double)1 / (double)X_Speed) * 1000;
  } else {
    X_Timer = -1;
  }
  if (done_Y == 0) {
    Y_Timer = ((double)1 / (double)Y_Speed) * 1000;
  } else {
    Y_Timer = -1;
  }
  if (done_Z == 0) {
    Z_Timer = ((double)1 / (double)Z_Speed) * 1000;
  } else {
    Z_Timer = -1;
  }

  Global_Timer = micros();

  //Do the moving
  while (done_X != 1 && done_Y != 1 && done_Z != 1) {

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

    //Lower X on-time timer
    if (X_enabled == true) {
      X_Pulse_Timer -= Timer_Difference;
    }

    //Turn X on
    if (X_Timer != -1 && X_enabled == false && X_Time_Watch >= X_Timer) {
      digitalWrite(X_Axis_Step_Pin, HIGH);
      X_Pulse_Timer = Min_Pulse_Width;
      X_enabled = true;
    }

    //Turn X off
    if (X_enabled == true && X_Pulse_Timer <= 0) {
      digitalWrite(X_Axis_Step_Pin, LOW);
      X_enabled = false;
    }

    //Lower Y on-time timer
    if (Y_enabled == true) {
      Y_Pulse_Timer -= Timer_Difference;
    }

    //Turn Y on
    if (Y_Timer != -1 && Y_enabled == false && Y_Time_Watch >= Y_Timer) {
      digitalWrite(Y_Axis_Step_Pin, HIGH);
      Y_Pulse_Timer = Min_Pulse_Width;
      Y_enabled = true;
    }

    //Turn Y off
    if (Y_enabled == true && Y_Pulse_Timer <= 0) {
      digitalWrite(Y_Axis_Step_Pin, LOW);
      Y_enabled = false;
    }

    //Lower Z on-time timer
    if (Z_enabled == true) {
      Z_Pulse_Timer -= Timer_Difference;
    }

    //Turn Z on
    if (Z_Timer != -1 && Z_enabled == false && Z_Time_Watch >= Z_Timer) {
      digitalWrite(Z_Axis_Step_Pin, HIGH);
      Z_Pulse_Timer = Min_Pulse_Width;
      Z_enabled = true;
    }

    //Turn Z off
    if (Z_enabled == true && Z_Pulse_Timer <= 0) {
      digitalWrite(Z_Axis_Step_Pin, LOW);
      Z_enabled = false;
    }

    //Check if each axis is complete
    if (done_X == 0) {
      done_X = checkIfDone(currentGCommand->x - Head_X);
    }

    if (done_Y == 0) {
      done_Y = checkIfDone(currentGCommand->y - Head_Y);
    }

    if (done_Z == 0) {
      done_Z = checkIfDone(currentGCommand->z - Head_Z);
    }
    
  }

}


//Reverses the given motor's direction
void reverseMotor(bool Motor) {
  if (Motor == Clockwise) {
    Motor = CounterClockwise;
  } else {
    Motor = Clockwise;
  }
}

//Checks if theres less than a step left
//If so, figure that it's done
bool checkIfDone(float MM_Left){
  if (MM_Left < 0){
    MM_Left *= -1;
  }
  
  if (MM_Left < MM_Per_Step){
    return 1;
  } else {
    return 0;
  }
}

