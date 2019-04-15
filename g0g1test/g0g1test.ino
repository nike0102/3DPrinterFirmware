#define Clockwise 0
#define CounterClockwise 1
#define False 0
#define True 1
#define MM_Per_Rotation 8
#define Min_Pulse_Width 1        //in microseconds, minimum of arduino seems to be ~120us on a 2 axis move with extrusion

#define X_Axis_Step_Pin 31
#define Y_Axis_Step_Pin 33
#define E_Axis_Step_Pin 35

#include <math.h>

//Structures
typedef struct Gcommand{
  
  //Values
  float x;
  float y;
  float z;
  float u;
  float v;
  float w;  
  float i;  //x-offset
  float j;  //y-offset
  float d;  //diameter
  float f;  //feed
  float e;  //extrude
  float t;  //tool
  float s;  //temp  r is treated as s
  char letter;
  float num;
  char msg[25];
  
  //Flags (0 - not used, 1 - used)
  unsigned char xf;
  unsigned char yf;
  unsigned char zf;
  unsigned char uf;
  unsigned char vf;
  unsigned char wf;
  unsigned char ifl;
  unsigned char jf;
  unsigned char df;
  unsigned char ff;
  unsigned char ef;
  unsigned char tf;
  unsigned char sf;
  unsigned char mf;
}GCommand;

GCommand gc;
uint8_t X_Direction = Clockwise, Y_Direction = Clockwise, Z_Direction = Clockwise, E_Direction = Clockwise;
int Steps_Per_Rotation = 800;
float Head_X = 0.0, Head_Y = 0.0, Head_Z = 0.0, Head_E1 = 0.0;
float MM_Per_Step = 8.0 / (float)Steps_Per_Rotation;
  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(X_Axis_Step_Pin, OUTPUT);
  pinMode(Y_Axis_Step_Pin, OUTPUT);
  pinMode(E_Axis_Step_Pin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  gc.num = 1;
  gc.f = 15;
  gc.e = 0.04;
  gc.x = 0.1;
  gc.y = 0.2;
  gc.z = 0.0;
  Serial.print("G1 X");
  Serial.print(gc.x);
  Serial.print(" Y");
  Serial.print(gc.y);
  Serial.print(" Z");
  Serial.print(gc.z);
  Serial.print(" E");
  Serial.println(gc.e);
  linearMove(&gc);
  Serial.println("\nDone with GCommand 1");
  Serial.print("Head_X = ");
  Serial.println(Head_X);
  Serial.print("Head_Y = ");
  Serial.println(Head_Y);
  Serial.print("Head_Z = ");
  Serial.println(Head_Z);
  Serial.print("Head_E = ");
  Serial.println(Head_E1);
  Serial.println();

  /*
  gc.e = 0.1;
  gc.x = 0.2;
  gc.y = 0.3;
  gc.z = 0.0;
  Serial.print("G0 X");
  Serial.print(gc.x);
  Serial.print(" Y");
  Serial.print(gc.y);
  Serial.print(" Z");
  Serial.print(gc.z);
  Serial.print(" E");
  Serial.println(gc.e);
  linearMove(&gc);
  Serial.println("\nDone with GCommand 2");
  Serial.print("Head_X = ");
  Serial.println(Head_X);
  Serial.print("Head_Y = ");
  Serial.println(Head_Y);
  Serial.print("Head_Z = ");
  Serial.println(Head_Z);
  Serial.print("Head_E = ");
  Serial.println(Head_E1);
  Serial.println();
  */
  while (1){
    
  }
  
}

//G0 and G1 - Linear Move   G0 has no extrusion
void linearMove(struct Gcommand *currentGCommand) {

  float X_Speed, Y_Speed, Z_Speed, X_Change, Y_Change, Z_Change, E_Change, E_Speed, Old_E_Head = Head_E1;
  uint8_t X_done = False, Y_done = False, Z_done = False, X_Enabled = False, Y_Enabled = False, Z_Enabled = False;
  
  long X_Timer = 0, X_Pulse_Timer = 0, Y_Timer = 0, Y_Pulse_Timer = 0, Z_Timer = 0, Z_Pulse_Timer = 0, Timer_Difference;
  long X_Time_Watch = 0, Y_Time_Watch = 0, Z_Time_Watch = 0;

  long E_Timer = 0, E_Pulse_Timer = 0, E_Time_Watch = 0;
  uint8_t E_Enabled = False, E_done = False;

  unsigned long Global_Timer = 0, Old_Global_Timer = 0;


  //Check Directions
  if (Head_X > currentGCommand->x) {
    //digitalWrite(X_Axis_Dir_Pin, LOW);
    X_Direction = CounterClockwise;
    X_Change = Head_X - currentGCommand->x;
  } else {
    //digitalWrite(X_Axis_Dir_Pin, HIGH);
    X_Direction = Clockwise;
    X_Change = currentGCommand->x - Head_X;
  }

  if (Head_Y > currentGCommand->y) {
    //digitalWrite(Y_Axis_Dir_Pin, LOW);
    Y_Direction = CounterClockwise;
    Y_Change = Head_Y - currentGCommand->y;
  } else {
    Y_Direction = Clockwise;
    //digitalWrite(Y_Axis_Dir_Pin, HIGH);
    Y_Change = currentGCommand->y - Head_Y;
  }

  if (Head_Z > currentGCommand->z) {
    //digitalWrite(Z1_Axis_Dir_Pin, LOW);
    //digitalWrite(Z2_Axis_Dir_Pin, LOW);
    Z_Direction = CounterClockwise;
    Z_Change = Head_Z - currentGCommand->z;
  } else {
    //digitalWrite(Z1_Axis_Dir_Pin, HIGH);
    //digitalWrite(Z2_Axis_Dir_Pin, HIGH);
    Z_Direction = Clockwise;
    Z_Change = currentGCommand->z - Head_Z;
  }

  if (currentGCommand->e < 0) {
    //digitalWrite(E1_Axis_Dir_Pin, LOW);
    E_Direction = CounterClockwise;
    E_Change = currentGCommand->e;
  } else {
    //digitalWrite(E1_Axis_Dir_Pin, HIGH);
    E_Direction = Clockwise;
    E_Change = currentGCommand->e;
  }
  
  Serial.print("X_Change = ");
  Serial.print(X_Change);
  Serial.print(" Y_Change = ");
  Serial.print(Y_Change);
  Serial.print(" Z_Change = ");
  Serial.print(Z_Change);
  Serial.print(" E_Change = ");
  Serial.println(E_Change);

  //Three axis move
  if (Head_X != currentGCommand->x && Head_Y != currentGCommand->y && Head_Z != currentGCommand->z) {
    X_done = 0;
    Y_done = 0;
    Z_done = 0;
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
            Serial.println("X-Axis movement!");
            X_Speed = currentGCommand->f;
            X_done = 0;
            Y_done = 1;
            Z_done = 1;
          } else

            //One axis move - Y
            if (Head_Y != currentGCommand->y) {
              Serial.println("Y-Axis movement!");
              X_done = 1;
              Y_done = 0;
              Y_Speed = currentGCommand->f;
              Z_done = 1;
            } else

              //One axis move - Z
              if (Head_Z != currentGCommand->z) {
                Serial.println("Z-Axis movement!");
                X_done = 1;
                Y_done = 1;
                Z_done = 0;
                Z_Speed = currentGCommand->f;
              } else {
                //E only
              }

  

  //Determine the timer values in microseconds
  //Makes sure there's no divide by zero error
  //Feedrate = mm/s
  //Timer = us/step
  //1mm = 200, 400, 800 steps
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
    Z_Timer *= MM_Per_Step;     //Microstepping factor
  } else {
    Z_Timer = -1;
  }
  if ((int)currentGCommand->num == 0){
    E_done = 1;
    E_Timer = -1;
  } else {
    E_done = 0;

    //Find biggest timer
    if (X_Timer >= Y_Timer && X_Timer >= Z_Timer){  //X is biggest
      E_Timer = (X_Timer * (X_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (Y_Timer >= X_Timer && Y_Timer >= Z_Timer){  //Y is biggest
      E_Timer = (Y_Timer * (Y_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (Z_Timer >= X_Timer && Z_Timer >= Y_Timer){  //Z is biggest
      E_Timer = (Z_Timer * (Z_Change / MM_Per_Step)) / (E_Change / MM_Per_Step);  //E_Timer = Total Time / E steps
    }

    if (X_Timer == -1 && Y_Timer == -1 && Z_Timer == -1){ //Only an extrude
      E_Timer = ((double)1.0 / (double)currentGCommand->f) * 1000000.0;
      E_Timer *= MM_Per_Step;     //Microstepping factor
    }
  }

  Serial.print("X_Timer = ");
  Serial.print(X_Timer);
  Serial.print(" Y_Timer = ");
  Serial.print(Y_Timer);
  Serial.print(" Z_Timer = ");
  Serial.print(Z_Timer);
  Serial.print(" E_Timer = ");
  Serial.println(E_Timer);
  
  Global_Timer = micros();

  //Do the moving
  while (X_done == 0 || Y_done == 0 || Z_done == 0 || E_done == 0) {

    //Do time math
    Old_Global_Timer = Global_Timer;
    Global_Timer = micros();
    Timer_Difference = Global_Timer - Old_Global_Timer;
    //Serial.print("Timer_Difference = ");
    //Serial.println(Timer_Difference);

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
      if (X_Direction == Clockwise){
        Head_X += MM_Per_Step;
      } else {
        Head_X -= MM_Per_Step;
      }
      X_Enabled = False;
      //Serial.print("Head_X = ");
      //Serial.println(Head_X);
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
      if (Y_Direction == Clockwise){
        Head_Y += MM_Per_Step;
      } else {
        Head_Y -= MM_Per_Step;
      }
      Y_Enabled = False;
      //Serial.print("Head_Y = ");
      //Serial.println(Head_Y);
    }

    //Lower Z on-time timer
    if (Z_Enabled == True) {
      Z_Pulse_Timer -= Timer_Difference;
    }

    //Turn Z on
    if (Z_Timer != -1 && Z_Enabled == False && Z_Time_Watch >= Z_Timer) {
      //digitalWrite(Z_Axis_Step_Pin, HIGH);
      Z_Pulse_Timer = Min_Pulse_Width;
      Z_Time_Watch = 0;
      Z_Enabled = True;
    }

    //Turn Z off
    if (Z_Enabled == True && Z_Pulse_Timer <= 0) {
      //digitalWrite(Z_Axis_Step_Pin, LOW);
      if (Z_Direction == Clockwise){
        Head_Z += MM_Per_Step;
      } else {
        Head_Z -= MM_Per_Step;
      }
      Z_Enabled = False;
      //Serial.print("Head_Z = ");
      //Serial.println(Head_Z);
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
      if (E_Direction == Clockwise){
        Head_E1 += MM_Per_Step;
      } else {
        Head_E1 -= MM_Per_Step;
      }
      E_Enabled = False;
      //Serial.print("Head_E = ");
      //Serial.println(Head_E1);
    }

    //Check if each axis is complete
    if (X_done == 0) {
      X_done = checkIfDone(currentGCommand->x - Head_X);
      if (X_done == 1){
        //Serial.println("X is done");
        X_Timer = -1;
      }
    }

    if (Y_done == 0) {
      Y_done = checkIfDone(currentGCommand->y - Head_Y);
      if (Y_done == 1){
        //Serial.println("Y is done");
        Y_Timer = -1;
      }
    }

    if (Z_done == 0) {
      Z_done = checkIfDone(currentGCommand->z - Head_Z);
      if (Z_done == 1){
        //Serial.println("Z is done");
        Z_Timer = -1;
      }
    }

    if (E_done == 0) {
      E_done = checkIfDone(currentGCommand->e - Head_E1 - Old_E_Head);
      if (E_done == 1){
        //Serial.println("E is done");
        E_Timer = -1;
      }
    }
    
  }

}

//Checks if theres less than a step left
//If so, figure that it's done
uint8_t checkIfDone(float MM_Left){
  //Serial.println("Checked if done");
  if (MM_Left < 0.0){
    MM_Left *= -1;
  }
  
  if ((MM_Left < MM_Per_Step - 0.005) || (MM_Left == 0.0)){
    return 1;
  } else {
    return 0;
  }
}
