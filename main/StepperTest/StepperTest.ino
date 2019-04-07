//Register mapping values for Arduino UNO
//PORTD is 0 - 7
//PORTB is 8 - 13
//PORTC is A0 - A5

//Define which pins on microcontroller connect to motor coil inputs
#define A1Pin 3
#define B1Pin 5
#define A2Pin 6
#define B2Pin 9

//Initalize variables
int currentMotorState = 0, dir = 0, go = 0, ms0 = 0, ms1 = 0, ms2 = 0, msValue = 0, currentlyStepping = 0, spee = 0, steps, i;
int mamount = 1;
int doneStepping = 0;
int currentReadingValues[20] = {0};
char buff[40] = {'\0'};
String pars = "";

//Runs only once when Arduino is powered
void setup() {

  //Configure serial port
  Serial.begin(9600);
  
  //Configure Inputs and Outputs on register level
  DDRD = B01101000;
  DDRB = B00000010;
}

//Main program loop
void loop() {

  //Show option menu
  Serial.println();
  Serial.println("Commands -");
  Serial.println("Set - Show current settings");
  Serial.println("Dir - Toggle direction");
  Serial.println("Speed - Set speed in RPM    [MAX 750]");
  Serial.println("Steps - Set number of steps");
  Serial.println("MS0 - Toggle MS0");
  Serial.println("MS1 - Toggle MS1");
  Serial.println("MS2 - Toggle MS2");
  Serial.println("Go - Drive motor");
  Serial.println();

  //Do nothing until a command is sent through the Serial port to the Arduino, then wait 250 milliseconds
  //To ensure entire data transmission has been received before attempting to parse
  while (!Serial.available()) {}
  delay(250);
  
  //Load in serial information into character buffer
  for (i = 0; i < 40 && Serial.available() > 0; i++) {
    buff[i] = Serial.read();
  }

  //Output the command that has been received back to the user
  Serial.print(">>");
  Serial.println(buff);
  Serial.println();

  //Parse the various commands. Only checks the first few letters of the word.
  //IE code interprets "Direction", "Dir", "D", "direction", "dir", and "d" the same.
  if (buff[0] == 'D' || buff[0] == 'd') {
    if (dir == 0){
      dir = 1;
    } else {
      dir = 0;
    }
  } else

    if (buff[0] == 'S' || buff[0] == 's') {
      if (buff[1] == 'p') {
        for (i = 0; i < 40; i++) {
          buff[i] = '\0';
        }
        Serial.println("Enter speed in RPM: ");
        while (!Serial.available()) {}
        delay(250);
        for (i = 0; i < 40 && Serial.available() > 0; i++) {
          buff[i] = Serial.read();
        }
        for (i = 0; buff[i] != '\n'; i++) {
          pars += buff[i];
        }
        spee = pars.toInt();
        pars = "";
        Serial.print("Speed set to ");
        Serial.print(spee);
        Serial.println(" RPM");
      }
      if (buff[1] == 't') {
        for (i = 0; i < 40; i++) {
          buff[i] = '\0';
        }
        Serial.println("Enter amount of steps: ");
        while (!Serial.available()) {}
        delay(250);
        for (i = 0; i < 40 && Serial.available() > 0; i++) {
          buff[i] = Serial.read();
        }
        for (i = 0; buff[i] != '\n'; i++) {
          pars += buff[i];
        }
        steps = pars.toInt();
        pars = "";
        Serial.print("Steps set to ");
        Serial.println(steps);
      }
      if (buff[1] == 'e') {
        Serial.println("MS2\tMS1\tMS0\tDir\tSteps\tSpeed");
        Serial.print(ms2);
        Serial.print("\t");
        Serial.print(ms1);
        Serial.print("\t");
        Serial.print(ms0);
        Serial.print("\t");
        if (dir == 0) {
          Serial.print("C\t");
        } else {
          Serial.print("CC\t");
        }
        Serial.print(steps);
        Serial.print("\t");
        Serial.print(spee);
        Serial.println(" RPM");
        Serial.println();
        Serial.print("Microstepping factor is ");
        Serial.println(m());
      }
    } else
    if (buff[0] == 'M' || buff[0] == 'm' || buff[0] == '0' || buff[0] == '1' || buff[0] == '2') {
        Serial.println("MS2\tMS1\tMS0");
        Serial.print(ms2);
        Serial.print("\t");
        Serial.print(ms1);
        Serial.print("\t");
        Serial.println(ms0);
        Serial.print("Microstepping factor is now ");
        Serial.println(m());
      } else

        if (buff[0] == 'G' || buff[0] == 'g') {
          runMotor();
        } else

        {
          Serial.println("Improper command!");
        }

  //Clear the buffer after parsing
  for (i = 0; i < 40; i++) {
    buff[i] = '\0';
  }
  
}

void runMotor(){

  int i2;;
  unsigned long t, allstart, allstop, startStepTime;
  float timp = (float)spee * 10 / 3 ;
  timp = 1 / timp;
  t = timp * 1000000; //t is microseconds
  
  //Output information
  Serial.println("MS2\tMS1\tMS0\tDir\tSteps\tSpeed");
  Serial.print(ms2);
  Serial.print("\t");
  Serial.print(ms1);
  Serial.print("\t");
  Serial.print(ms0);
  Serial.print("\t");
  if (dir == 0) {
    Serial.print("C\t");
  } else {
    Serial.print("CC\t");
  }
  Serial.print(steps);
  Serial.print("\t");
  Serial.print(spee);
  Serial.println(" RPM");
  Serial.println();
  Serial.print("Microstepping factor is ");
  Serial.println(m());
  Serial.println("\nRunning motor...");
  
  //Direct port manipulation allows for faster and simultaneous writes
  //Current implementation ensures H-Bridges never short to ground
  allstart = micros();
  for (i2 = 0; i2 < steps; i2++){

    //Turn off stepper pins
    PORTD = PORTD & B10010111;
    PORTB = PORTB & B11111101;

    //Turn on next stepper pin
    switch (currentMotorState){
      case 0:
        PORTD = PORTD | B00001000;    //Turn on 1A
        break;
      case 1:
        PORTD = PORTD | B01000000;    //Turn on 2A
        break;
      case 2:
        PORTD = PORTD | B00100000;    //Turn on 1B 
        break;
      case 3:
        PORTB = PORTB | B00000010;    //Turn on 2B 
        break;
    }
    startStepTime = micros();
    
    while (micros() - startStepTime < t){}  //Wait for step time per step to finish

    //Determine next coil state
    if (dir == 0){ //Clockwise
      currentMotorState++;
    } else {       //CounterClockwise
      currentMotorState--;
    }
  }
  allstop = micros();
  Serial.print("\nDone. Should of taken ");
  Serial.print(t * steps);
  Serial.print(" microseconds and took ");
  Serial.print(allstart - allstop);
  Serial.print(" microseconds. Difference of ");
  Serial.println( (allstart - allstop) - (t * steps) );
}

//Turn microstepping binary into decimal and calculate new step factor amount
int m() {
  int msValue = 0, mamount;
  msValue = ms0 | (ms1 << 1) | (ms2 << 2);
  switch (msValue) {
    case 0:
      mamount = 1;
      break;
    case 1:
      mamount = 2;
      break;
    case 2:
      mamount = 4;
      break;
    case 3:
      mamount = 8;
      break;
    case 4:
      mamount = 16;
      break;
    case 5:
      mamount = 16;
      break;
    case 6:
      mamount = 16;
      break;
    case 7:
      mamount = 16;
      break;
  }
  return mamount;
}

