//Use Serial2
//ESP TX->Arduino RX
//ESP RX->Voltage divider->Arduino TX
//ESP All other pins 3.3V
//Baudrate = 9600

#define REQUESTLENGTH 1000

#include "WiFiEsp.h"


char ssid[] = "3DPrinter";         // your network SSID (name)
char pass[] = "12345678";        // your network password
char c, req[REQUESTLENGTH], endbufc[200];
int reqi = 0;

int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiEspServer server(80);

RingBuffer buf(8), endbuf(200);

void setup() {
  Serial.begin(9600);   // initialize serial for debugging
  Serial2.begin(9600);    // initialize serial for ESP module
  
  WiFi.init(&Serial2);    // initialize ESP module

  Serial.print("Attempting to start AP ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);

  Serial.println("Access point started");
  printWifiStatus();

  // start the web server on port 80
  server.begin();
  Serial.println("Server started");
}

void loop() {
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
       
        clearReq();
        Serial.println("Reading first 1000 characters");
        while (client.available() && reqi < REQUESTLENGTH){
          c = client.read();          // read byte into buffer
          req[reqi++] = c;
          buf.push(c);
        }
        Serial.println("Done");
        Serial.println("Reading more characters");
        while (client.available()){
          c = client.read();          // read byte into buffer
          buf.push(c);
          endbuf.push(c);
        }
        Serial.println("Done");
        if (reqi == REQUESTLENGTH){
          Serial.println("Request longer than 1000 characters!");
        }

        Serial.print("Recieved: ");
        Serial.println(req);
        Serial.println();

        Serial.println("\nLast however many chars: ");
        for (reqi = 0; reqi < 200; reqi++){
          endbuf.getStr(endbufc, 0);
        }
        Serial.println(endbufc);
        Serial.println();
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          Serial.println("Sending Response");
          sendHttpResponse(client);
          Serial.println("Sent");
          break;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(10);

    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }

}

void sendHttpResponse(WiFiEspClient client)
{
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
  client.print("<input type=\"submit\" name=\"upload\" value=\"Upload\">\r\n</form>\r\n");
  client.print("</body>\r\n");
  client.print("</html>\r\n");
}

void printWifiStatus()
{
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, connect to ");
  Serial.print(ssid);
  Serial.print(" and open a browser to http://");
  Serial.println(ip);
  Serial.println();
}

void clearReq(){
  for (reqi = 0; reqi < REQUESTLENGTH; reqi++){
    req[reqi] = 0;
  }
  reqi = 0;
}

