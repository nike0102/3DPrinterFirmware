//Includes
#include "gcodefunctions.h"

//Variables
uint8_t X_Motor_Direction = Clockwise, Y_Motor_Direction = Clockwise, Z_Motor_Direction = Clockwise, E1_Motor_Direction = Clockwise, E2_Motor_Direction = Clockwise;
float Head_X, Head_Y, Head_Z, Head_E1, Head_E2;

//1 = Full, 2 = 1/2, 4 = 1/4, 8 = 1/8, 16 = 1/16
//uint8_t stepMode = 1;
float MM_Per_Step = 8 / 200;//((float)stepMode * 200);

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

