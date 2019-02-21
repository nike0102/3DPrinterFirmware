#ifndef GCODEFUN_H
#define GCODEFUN_H

//Includes
#include <Arduino.h>
#include "Config.h"
#include <stdint.h>
#include "parser.h"
#include "sdfunctions.h"

//Global variables
extern uint8_t X_Motor_Direction, Y_Motor_Direction, Z_Motor_Direction, E1_Motor_Direction, E2_Motor_Direction;
extern float Head_X, Head_Y, Head_Z, Head_E1, Head_E2;
extern char selectedSDFile[25];
extern GCommand currentGCodeCommand;

#ifdef __cplusplus
extern "C" {
#endif

//Functions
//GCode Functions
void linearMove(GCommand *currentGCommand);         //G0 and G1
void autoHome();                                    //G28

//MCode Functions
void stopJob();                                     //M0 and M1
void listSDFiles();                                 //M20
void selectSDFile(GCommand *command);               //M23
void startSDPrint(char* filename);                  //M24
void continueSDPrint();                             //M24 continued

//Misc Functions
void manualMove(uint8_t axis, float amount);      //NOT WRITTEN YET
void reverseMotor(uint8_t Motor);
uint8_t checkIfDone(float MM_Left);

#ifdef __cplusplus
}
#endif

#endif
