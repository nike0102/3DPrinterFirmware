#include "parser.h"

int readGCodeString(char* command, GCommand *returnGCommand){

  char i = 0, i2 = 0, fflag = 0;
  char tempBuf[25] = {'\0'};
  int inum;
  float fnum;
  //Set all flags to 0
  returnGCommand->xf = 0;
  returnGCommand->yf = 0;
  returnGCommand->zf = 0;
  returnGCommand->uf = 0;
  returnGCommand->vf = 0;
  returnGCommand->wf = 0;
  returnGCommand->ifl = 0;
  returnGCommand->jf = 0;
  returnGCommand->df = 0;
  returnGCommand->ff = 0;
  returnGCommand->ef = 0;
  returnGCommand->tf = 0;
  returnGCommand->sf = 0;
  returnGCommand->letter = 0;
  returnGCommand->num = 0;
  returnGCommand->mf = 0;

  
  //Iterate through the line
  while (command[i] != '\n' && command[i] != '\r' && command[i] != '\0' && command[i] != ';'){
    
    //Iterate through each parameter
    while (command[i] != ' ' && command[i] != '\r' && command[i] != '\n' && command[i] != '\0' && command[i] != ';' && i2 < 24){
      tempBuf[i2++] = command[i];
      if (command[i++] == '.'){
        fflag = 1;
      }
    }
	
	if (tempBuf[0] == '\r' || tempBuf[0] == '\n' || tempBuf[0] == '\0' || tempBuf[0] == ';'){
		return 0;
	}

    //Set end of temporary buffer to NULL
    tempBuf[i2] = '\0';
	
	//Check for command with message
	if ((returnGCommand->letter == 'M') && (returnGCommand->num == 117)){
	  for (i = 0; i < 24 && tempBuf[i] != '\r' && tempBuf[i] != '\n' && tempBuf[i] != '\0'; i++){
		*(returnGCommand->msg + i) = tempBuf[i];  
	  }
	  *(returnGCommand->msg + i) = '\0';
	  returnGCommand->mf = 1;
	  return 0;
	}

    //Get number value
    if (fflag == 0){
      inum = getIntValue(tempBuf);
    } else {
      fnum = getFloatValue(tempBuf);
    }

    //Determine command
    switch (tempBuf[0]){
	  case 'x':
      case 'X':
        if (fflag == 0){
          returnGCommand->x = inum;
        } else {
          returnGCommand->x = fnum;
        }
		returnGCommand->xf = 1;
        break;
      case 'y':
      case 'Y':
        if (fflag == 0){
          returnGCommand->y = inum;
        } else {
          returnGCommand->y = fnum;
        }
		returnGCommand->yf = 1;
        break;
      case 'z':
      case 'Z':
        if (fflag == 0){
          returnGCommand->z = inum;
        } else {
          returnGCommand->z = fnum;
        }
		returnGCommand->zf = 1;
        break;
      case 'u':
      case 'U':
        if (fflag == 0){
          returnGCommand->u = inum;
        } else {
          returnGCommand->u = fnum;
        }
		returnGCommand->uf = 1;
        break;
      case 'v':
      case 'V':
        if (fflag == 0){
          returnGCommand->v = inum;
        } else {
          returnGCommand->v = fnum;
        }
		returnGCommand->vf = 1;
        break;
      case 'w':
      case 'W':
        if (fflag == 0){
          returnGCommand->w = inum;
        } else {
          returnGCommand->w = fnum;
        }
		returnGCommand->wf = 1;
        break;
	  case 'i':
      case 'I':
        if (fflag == 0){
          returnGCommand->i = inum;
        } else {
          returnGCommand->i = fnum;
        }
		returnGCommand->ifl = 1;
        break;
      case 'j':
      case 'J':
        if (fflag == 0){
          returnGCommand->j = inum;
        } else {
          returnGCommand->j = fnum;
        }
		returnGCommand->jf = 1;
        break;
      case 'd':
      case 'D':
        if (fflag == 0){
          returnGCommand->d = inum;
        } else {
          returnGCommand->d = fnum;
        }
		returnGCommand->df = 1;
        break;
	  case 'f':
      case 'F':
        if (fflag == 0){
          returnGCommand->f = inum;
        } else {
          returnGCommand->f = fnum;
        }
		returnGCommand->ff = 1;
        break;
	  case 'e':
      case 'E':
        if (fflag == 0){
          returnGCommand->e = inum;
        } else {
          returnGCommand->e = fnum;
        }
		returnGCommand->ef = 1;
        break;
	  case 't':	//Tool
      case 'T':
        if (fflag == 0){
          returnGCommand->t = inum;
        } else {
          returnGCommand->t = fnum;
        }
		returnGCommand->tf = 1;
        break;
	  case 's':	//Temperature
      case 'S':
	  case 'r':
      case 'R':
	    if (fflag == 0){
          returnGCommand->s = inum;
        } else {
          returnGCommand->s = fnum;
        }
		returnGCommand->sf = 1;
        break;
      case 'g':
      case 'G':
        returnGCommand->letter = 'G';
        if (fflag == 0){
          returnGCommand->num = inum;
        } else {
          returnGCommand->num = fnum;
        }
        break;
      case 'm':
      case 'M':
        returnGCommand->letter = 'M';
        if (fflag == 0){
          returnGCommand->num = inum;
        } else {
          returnGCommand->num = fnum;
        }
        break;
	  case 'n':
      case 'N':
        //Do nothing, we don't care about line numbers
        break;
	  default:
		printf("Error! Unknown parameter - %s", tempBuf);
    }

    //Clear temporary buffer
    for (i2 = 0; i2 < 25; i2++){
      tempBuf[i2] = '\0';
    }
    i2 = 0;
    fflag = 0;
	i++;
  }

	return 0;
}

float getFloatValue(char* temp){
  char i, i2, i3 = 0, lenBefore = 0, lenAfter = 0, value[25], value2[25];
  float num = 0;
  float mul;

  //Determine start position
  if (temp[1] == '-' || temp[1] == '+'){
	  i = 2;
  } else {
	  i = 1;
  }
  
  //Move values left of decimal to new array and count length
  for (i2 = i; temp[i2] != '.'; i2++){
	value[i3] = temp[i2];
	i3++;
	lenBefore++;
  }
  
  //Increment i2 so we're not at the decimal anymore, reset i3, then
  //move values right of decimal to new array and count length
  i3 = 0;
  i2++;
  for (i2; temp[i2] != '\0'; i2++){
	value2[i3] = temp[i2];
	i3++;
	lenAfter++;
  }
  
  //Calculate number to left of decimal
  for (i = 0; i < lenBefore; i++){
	mul = power(10, lenBefore - (i + 1));
    num += (float)(mul * (value[i] - 48));
  }
  
  //Calculate number to left of decimal
  for (i = 0; i < lenAfter; i++){
	mul = power(10, i + 1);
    num += (float)(value2[i] - 48) / mul;
  }
  
  //Make number negative if it is
  if (temp[1] == '-'){
	  num *= -1;
  }
  
  //Return number
  return num;
}

int getIntValue(char* temp){
  char i, i2, i3 = 0, len = 0, value[25];
  int num = 0, mul = 0;

  //Determine start position and if value is negative
  if (temp[1] == '-' || temp[1] == '+'){
	  i = 2;
  } else {
	  i = 1;
  }
  
  //Move values over to new array and determine length
  for (i2 = i; temp[i2] != '\0'; i2++){
	value[i3++] = temp[i2]; 
	len++;	
  }
  
  //If only single digit number, just return it now
  if (len == 1){
	  if (temp[1] == '-'){
		  return (-1 * (value[0] - 48));
	  } else {
		  return (value[0] - 48);
	  }
  }

  //If multi digit number, calculate it
  for (i = 0; i < len; i++){
	mul = power(10, len - (i + 1));
    num += mul * (value[i] - 48);
  }

  //Make number negative if it is
  if (temp[1] == '-'){
	  num *= -1;
  }
  
  //Return number
  return num;
  
}


int power(int base, int exponent){
  int num = base;
  char i;

  if (exponent == 0){
    return 1;
  }

  if (exponent == 1){
    return base;
  }
  
  for (i = 1; i < exponent; i++){
    num = num * base;
  }

  return num;
}
