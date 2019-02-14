//Includes
#include <wiringPi.h>
#include "Config.h"
#include "Parser.h"


//Main loop
int main(void){

	setup();

	return 0;
}



void setup(){
	//Motor step pins
	pinMode(X_Axis_Step_Pin, OUTPUT);
	pinMode(Y_Axis_Step_Pin, OUTPUT);
	pinMode(Z_Axis_Step_Pin, OUTPUT);


}