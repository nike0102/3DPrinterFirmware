#include "wifi.h"

//Use Serial2
//ESP TX->Arduino RX
//ESP RX->Voltage divider->Arduino TX
//ESP All other pins 3.3V
//Baudrate = 9600
//Serial3 for WiFi

char ssid[] = "3DPrinter";         // your network SSID (name)
char pass[] = "12345678";        // your network password
char c;
static SdFile SDWriteFile;

int status = WL_IDLE_STATUS;     // the Wifi radio's status
RingBuffer incoming(25);
RingBuffer mimebuffer(12);

void readRequest(HTMLRequest *htreq){

  byte i;

  while (htreq->type == 0){ //Determine if it's a GET or POST request
    //Read bit
    c = client.read();

    //Put in buffer
    incoming.push(c);

      //Check for type
      if (incoming.endsWith("GET ")){   //If it's a GET, send back homescreen
        htreq->type = 1;

        //Code to fix chrome bug
        i = 0;
        while (i < 10){
          c = client.read();
          incoming.push(c);
          i++;
          if (incoming.endsWith("/favicon")){
            //Serial.println("Got google favicon request");
          }
        }

        while (client.available()){
          c = client.read();
        }
        
        //Exit the function
        return;
      }
      
      if (incoming.endsWith("POST ")){    //If it's a POST
        htreq->type = 2;

        //Get name of POST action="" text
        i = 0;
        c = client.read();
        incoming.push(c);
        htreq->link[i++] = c;
        while ((c = client.read()) != ' ' && i < 25){
          incoming.push(c);
          htreq->link[i++] = c;
        }
      }

      
    }
    
    if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'u' && htreq->link[2] == 'p' && htreq->link[3] == 'l'){ //Upload a file

      while (!incoming.endsWith("Content-Length: ")){
        c = client.read();
        incoming.push(c);
      }

      //Content-Length part
        i = 1;
        c = client.read();
        incoming.push(c);
        htreq->filename[i++] = c;
        while ((c = client.read()) != '\r' && i < 25){
          incoming.push(c);
          htreq->filename[i++] = c;
        }   
        //Set int value
        htreq->messagelength = getIntValue(htreq->filename);
        //Clear out filename
        for (i = 0; i < 25; i++){
          htreq->filename[i++] = 0;
        }

      //Skip to end of body
      while (!incoming.endsWith("\r\n\r\n")){
        c = client.read();
        incoming.push(c);
      }

        
        //Skip to filename
        while (!incoming.endsWith("filename=\"")){
          c = client.read();
          incoming.push(c);
          htreq->alreadyread++;
        }

        //Read in filename and stop at the end "
        i = 0;
        while ((c = client.read()) != '"' && i < 25){
          incoming.push(c);
          htreq->filename[i++] = c;
          htreq->alreadyread++;
        }

      //Skip to end of section
      while (!incoming.endsWith("\r\n\r\n")){
        c = client.read();
        incoming.push(c);
        htreq->alreadyread++;
      }

      //Now we're at where the file starts

      //Error check and create a file
      if (currentlyopen == true){
        Serial.println(F("Error, a file is already open!"));
        return;
      } else {
        currentlyopen = true;
        SDWriteFile.open(htreq->filename, O_WRONLY | O_CREAT);
        SDWriteFile.close();
        SDWriteFile.open(htreq->filename, O_WRONLY);
      }

      i = 0;
      while (client.available()){
        
        c = client.read();
        incoming.push(c);
        mimebuffer.push(c);
        htreq->alreadyread++;

        if (incoming.endsWith("----WebK")){
          while (client.available()){
            c = client.read();
            htreq->alreadyread++;
          }
          break;
        }
        
        if (i < 10){
          i++;
        } else {
          c = mimebuffer.getCharBack(11);
          writeCharToFile(&SDWriteFile, c);
        }
        
      }

      SDWriteFile.close();
      currentlyopen = false;  
      } 

      if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 's' && htreq->link[2] == 'e' && htreq->link[3] == 'n'){ //Send a command
    
        //Skip to command
        while (!incoming.endsWith("\"Command\" = \"")){
          c = client.read();
          incoming.push(c);
        }

        //Clear buffer
        for (i = 0; i < 25; i++){
          inputBuffer[i] = 0;
        }
        i = 0;

        //Read the command
        c = client.read();
        while ((c != '"') && (i < 25)){
          inputBuffer[i++] = c;
          c = client.read();
        }

        //Handle the command
        getInput(false);
      }

      if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'X'){  //Manually move X-Axis
        if (htreq->link[2] == '-' && htreq->link[2] == '3'){  //-10mm
          manualMove(0, -10.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '2'){  //-1mm
          manualMove(0, -1.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '1'){  //-0.1mm
          manualMove(0, -0.1);
        }
        if (htreq->link[2] == '0'){                           //Home
          //Can't home
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '1'){  //+0.1mm
          manualMove(0, -0.1);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '2'){  //+1mm
          manualMove(0, -1.0);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '3'){  //+10mm
          manualMove(0, 10.0);
        }
      }

      if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'Y'){  //Manually move Y-Axis
        if (htreq->link[2] == '-' && htreq->link[2] == '3'){  //-10mm
          manualMove(1, -10.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '2'){  //-1mm
          manualMove(1, -1.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '1'){  //-0.1mm
          manualMove(1, -0.1);
        }
        if (htreq->link[2] == '0'){                           //Home
          //Can't home
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '1'){  //+0.1mm
          manualMove(1, -0.1);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '2'){  //+1mm
          manualMove(1, -1.0);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '3'){  //+10mm
          manualMove(1, 10.0);
        }
      }

      if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'Z'){  //Manually move Z-Axis
        if (htreq->link[2] == '-' && htreq->link[2] == '3'){  //-10mm
          manualMove(2, -10.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '2'){  //-1mm
          manualMove(2, -1.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '1'){  //-0.1mm
          manualMove(2, -0.1);
        }
        if (htreq->link[2] == '0'){                           //Home
          //Can't home
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '1'){  //+0.1mm
          manualMove(2, -0.1);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '2'){  //+1mm
          manualMove(2, -1.0);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '3'){  //+10mm
          manualMove(2, 10.0);
        }
      }

      if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'E'){  //Manually move E-Axis
        if (htreq->link[2] == '-' && htreq->link[2] == '3'){  //-10mm
          manualMove(3, -10.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '2'){  //-1mm
          manualMove(3, -1.0);
        }
        if (htreq->link[2] == '-' && htreq->link[2] == '1'){  //-0.1mm
          manualMove(3, -0.1);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '1'){  //+0.1mm
          manualMove(3, -0.1);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '2'){  //+1mm
          manualMove(3, -1.0);
        }
        if (htreq->link[2] == '+' && htreq->link[2] == '3'){  //+10mm
          manualMove(3, 10.0);
        }
      }

      //Send user back to homepage
      sendHttpResponseMain();
}

void powerOnWiFi(){
  Serial3.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial3);    // initialize ESP module

  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
  IPAddress ip = WiFi.localIP();
  
  Serial.println("Access point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password (WPA2): ");
  Serial.println(pass);
  Serial.print("To access this 3D Printer's website, go to http://");
  Serial.println(ip);
  Serial.println();
  
  
  // start the web server on port 80
  server.begin();
}


void sendHttpResponseMain(){

  char buf[200];
  char c1, c2;
  byte i, num;
  FileTree *head = &FilesOnSDCard;

  //Make sure a file isn't already open and then open the website file
  if (currentlyopen == true){
    Serial.println(F("Error, file already open!"));
    return;
  } else {
    currentlyopen = true;
    SDReadFile.open("index.html", O_RDONLY);
  }
  
  delay(50);

  //Start parsing and sending data
  while (SDReadFile.available()){

    //Clean out the buffer
    for (i = 0; i < 200; i++){
      buf[i] = 0;
    }

    //Handle lines
    for (i = 0; i < 200 && SDReadFile.available(); i++){
      buf[i] = SDReadFile.read();
      incoming.push(buf[i]);

      //Special cases
      if (incoming.endsWith("<!--")){

        //Determine data needed
        for (i = 0; i < 4; i++){
          buf[i] = SDReadFile.read();
          incoming.push(buf[i]);
        }

        //Files on SD card section
        if (incoming.endsWith("File")){
          //Clean out the buffer
          for (i = 0; i < 200; i++){
            buf[i] = 0;
          }

          //Find and print out files
          while (head->nextfile != NULL){
            client.print(head->filename);
            client.print("\r\n<br>\r\n");
            head = head->nextfile;
          }
          break;
        }

        //Attributes section
        if (incoming.endsWith("Attributes")){
          client.print("X-Axis Position: ");
          client.print(Head_X);
          client.print("<br>\r\n");

          client.print("Y-Axis Position: ");
          client.print(Head_Y);
          client.print("<br>\r\n");

          client.print("Z-Axis Position: ");
          client.print(Head_Z);
          client.print("<br>\r\n");

          client.print("Current Extruder Temperature: ");
          client.print(getTemperature(analogRead(Extruder_Thermistor_Pin)));
          client.print("<br>\r\n");

          client.print("Current Bed Temperature: ");
          client.print(getTemperature(analogRead(Bed_Thermistor_Pin)));
          client.print("<br><br>\r\n");

          client.print("Set Extruder Temperature: ");
          client.print(ExtruderTemp);
          client.print("<br>\r\n");

          client.print("Set Bed Temperature: ");
          client.print(BedTemp);
          client.print("\r\n");
          
          break;
        }    
      }
      //End special cases

      //No special cases, push the html line out to the client
      if (buf[i] == '\n'){
        buf[++i] = '\0';
        //Send out the line
        client.print(buf);
        break;
      }
      
    }
    
  }

  SDReadFile.close();
  currentlyopen = false;
}

void sendHttpFiles(char* fname, char* fnum){
  client.print(F("<div style=\"margin-bottom: 5px;\">\r\n"
               "<div style=\"float: left;\">\r\n"));
  client.print(fname);                                      //Name of file
  client.print(F("\r\n<div>\r\n<form action=\"PF"));
  client.print(fnum);                                       //Number in list
  client.print(F("\" style=\"float: right; width: 50px; height: 21px; margin-left: 10px;\">\r\n"
               "<button type=\"submit\""));
  if (currentlyPrinting == true){                           //Prevent user from trying to start a new print while one is going
    client.print(F("disabled"));          
  }
  client.print(F(">Print</button>\r\n</form>\r\n"
               "<form action=\"DF"));
  client.print(fnum);                                       //Number in list
  client.print(F("\" style=\"float: right; width: 50px; height: 21px;\">\r\n"
               "<button type=\"submit\">Delete</button>\r\n"
               "</form>\r\n</div>\r\n<br>\r\n"));
}

