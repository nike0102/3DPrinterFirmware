/* Pin Definitions - Arduino Mega
 * 
 *  
 *  SD Card Module
 *  CS - 53
 *  SCK - 52
 *  MISO - 50
 *  MOSI - 51
 */

//Definitions
#define Clockwise 0
#define CounterClockwise 1
#define False 0
#define True 1

//Pins
#define X_Axis_Step_Pin 3
#define X_Axis_Dir_Pin  4
#define X_Axis_MS1_Pin  5
#define X_Axis_MS2_Pin  6
#define X_Axis_MS3_Pin  7

#define Y_Axis_Step_Pin 8
#define Y_Axis_Dir_Pin  9

#define Z_Axis_Step_Pin 10
#define Z_Axis_Dir_Pin  11

//Variables
#define X_Axis_Dir_Config CounterClockwise  //On Y facing front of Bed
#define MM_Per_Rotation 8
#define Min_Pulse_Width 500        //in microseconds
