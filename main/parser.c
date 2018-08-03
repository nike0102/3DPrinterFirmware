//GCode parser

struct Gcommand{
	float x;
	float y;
	float z;
	float feed;
	float extrude;
	int number;
	char letter;
};

struct Gcommand* readGCodeString(char* command, struct Gcommand *returnGCommand){
	
	return returnGCommand;
}