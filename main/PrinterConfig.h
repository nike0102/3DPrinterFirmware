#ifndef PRINTERCONFIG_H
#define PRINTERCONFIG_H

/* Pin Definitions - Arduino Mega
 * 
 *  
 *  SD Card Module
 *  CS - 53
 *  SCK - 52
 *  MISO - 50
 *  MOSI - 51
 *  
 *  WiFi chip is on Serial3
 */

//Definitions
#define Clockwise 0
#define CounterClockwise 1
#define False 0
#define True 1
//#define JOSH

//Pins

//*******************************************************Motors
#ifndef JOSH
//A4988 driver pins
#define X_Axis_Step_Pin 37      //22 Broke - Use Z1
#define X_Axis_Dir_Pin  36      //23
#define Y_Axis_Step_Pin 25
#define Y_Axis_Dir_Pin  24
#define Z1_Axis_Step_Pin 22     //37 Tie to Z2 step pin
#define Z1_Axis_Dir_Pin  23     //36
#define Z2_Axis_Step_Pin 35
#define Z2_Axis_Dir_Pin  34
#define E_Axis_Step_Pin 49
#define E_Axis_Dir_Pin  48
#define MS1_Pin 12
#define MS2_Pin 11
#define MS3_Pin 10
#else
//Josh's pins
#define X_Axis_2B //PD2
#define X_Axis_2A //PD3
#define X_Axis_1A 22
#define X_Axis_1B 23

#define Y_Axis_2B //PD4
#define Y_Axis_2A //PD5
#define Y_Axis_1A 25
#define Y_Axis_1B 24

#define Z1_Axis_2B //PD6
#define Z1_Axis_2A //PD7
#define Z1_Axis_1A 37
#define Z1_Axis_1B 36

#define Z2_Axis_2B //No connection to ATMega2560
#define Z2_Axis_2A //No connection to ATMega2560
#define Z2_Axis_1A 35
#define Z2_Axis_1B 34

#define E_Axis_2B //PE2 Pin not supported in Arduino
#define E_Axis_2A //PE3
#define E_Axis_1A 49 
#define E_Axis_1B 48
#endif
//*******************************************************Motors


//*******************************************************Heaters
#define Extruder_Heater_Pin 6
#define Bed_Heater_Pin 7
//*******************************************************Heaters


//*******************************************************Thermistors
#define Extruder_Thermistor_Pin A0
#define Bed_Thermistor_Pin A1
//*******************************************************Thermistors


//*******************************************************Endstops
#define X-Axis_Endstop //Don't use -> Shorted to ground
#define Y-Axis_Endstop //Don't use -> Shorted to ground
#define Z-Axis_Endstop //Don't use -> Shorted to ground
//*******************************************************Endstops

//Constants
#define MM_Per_Rotation 8
#define Min_Pulse_Width 150        //in microseconds, minimum of arduino seems to be ~120us on a 2 axis move with extrusion

//Which way is positive when the motor spins
#define X_Motor_Direction_Setting Clockwise
#define Y_Motor_Direction_Setting Clockwise
#define Z_Motor_Direction_Setting CounterClockwise   
#define E1_Motor_Direction_Setting Clockwise

#endif
