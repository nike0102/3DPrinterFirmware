//Libraries
#include <stdio.h>

//Structures
typedef struct Gcommand{
	
	//Values
	float x;
	float y;
	float z;
	float u;
	float v;
	float w;	
	float i;	//x-offset
	float j;	//y-offset
	float d;	//diameter
	float f;	//feed
	float e;	//extrude
	float t;	//tool
	float s;	//temp	r is treated as s
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

//Functions
int readGCodeString(char* command, struct Gcommand* returnGCommand);
int getIntValue(char* temp);
float getFloatValue(char* temp);
int power(int base, int exponent);
