#include "wifi.h"

//Use Serial2
//ESP TX->Arduino RX
//ESP RX->Voltage divider->Arduino TX
//ESP All other pins 3.3V
//Baudrate = 9600

char ssid[] = "3DPrinter";         // your network SSID (name)
char pass[] = "12345678";        // your network password
char c;
static SdFile SDWriteFile;

int status = WL_IDLE_STATUS;     // the Wifi radio's status
RingBuffer incoming(25);
RingBuffer mimebuffer(12);

void readRequest(HTMLRequest *htreq){

  byte i;

  while (htreq->type == 0){
    //Read bit
    c = client.read();

    //Put in buffer
    incoming.push(c);

    //Check for type
      if (incoming.endsWith("GET ")){
        htreq->type = 1;

        //Code to fix chrome bug
        i = 0;
        while (i < 10){
          c = client.read();
          incoming.push(c);
          i++;
          if (incoming.endsWith("/favicon")){
            Serial.println("Got google favicon request");
          }
        }

        while (client.available()){
          c = client.read();
        }
        
        //Exit the function
        return;
      }
      
      if (incoming.endsWith("POST ")){
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
    
    if (htreq->type == 2 && htreq->link[0] == '/' && htreq->link[1] == 'u' && htreq->link[2] == 'p' && htreq->link[3] == 'l'){

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

        if (incoming.endsWith("----WebKit")){
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

      //Send user back to home
      sendHttpResponseMain();
}

void powerOn(){
  Serial2.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial2);    // initialize ESP module

  Serial.print("Attempting to start AP ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);

  Serial.println("Access point started");

  // start the web server on port 80
  server.begin();
  Serial.println("Server started");
}


void sendHttpResponseMain(){
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    "\r\n");
  client.print("<!DOCTYPE HTML>\r\n");
  client.print("<html>\r\n");
  client.print("<body style=\"background-color: lightblue;\">\r\n");
  client.print("<h1 style=\"text-align: center;\">3D Printer Control</h1>\r\n");
  client.print("<form action=\"upload\" method=\"post\" enctype=\"multipart/form-data\">\r\n");
  client.print("<input type=\"file\" name=\"fileToUpload\" value=\"Upload\">\r\n");
  client.print("<input type=\"submit\" value=\"Upload\">\r\n</form>\r\n");
  client.print("</body>\r\n");
  client.print("</html>\r\n");
  delay(50);
}



