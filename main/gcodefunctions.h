//Includes
#include "Config.h"
#include <stdint.h>
#include "parser.h"

//Global variables
extern uint8_t X_Motor_Direction, Y_Motor_Direction, Z_Motor_Direction, E1_Motor_Direction, E2_Motor_Direction;
extern float Head_X, Head_Y, Head_Z, Head_E1, Head_E2;

//Functions
void linearMove(struct Gcommand *currentGCommand);
void reverseMotor(uint8_t Motor);
uint8_t checkIfDone(float MM_Left);
