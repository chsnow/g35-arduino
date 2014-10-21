#ifndef _WIFI_HTTP_SERVER_H_
#define _WIFI_HTTP_SERVER_H_
// A giant hack on top of the Arduino WifiWebServer example code.
//
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

#include <SPI.h>
#include <WiFi.h>
#include "htmlwriter.h"

#include "credentials.h"  // NOTE: you need to create this file yourself and define WIFI_SSID and WIFI_PASSWORD

namespace Wifi {
  char ssid[] = WIFI_SSID;      //  your network SSID (name) 
  char pass[] = WIFI_PASSWORD;   // your network password
  int status = WL_IDLE_STATUS;
  int last_status = WL_IDLE_STATUS;
  WiFiServer server(80);
  
  bool HandleConnection(WiFiClient* client) {
    bool return_status = false;
    bool found_upload = true;
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client->connected()) {            // loop while the client's connected
      if (client->available()) {             // if there's bytes to read from the client,
        char c = client->read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) { 
            htmlwriter::Output(client, Global::program_str);
            /* 
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:    
            client->println("HTTP/1.1 200 OK");
            client->println("Content-type:text/html");
            client->println();

            // the content of the HTTP response follows the header:
            client->print("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
            client->print("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>");

            // The HTTP response ends with another blank line:
            client->println();
            */
            // break out of the while loop:
            break;         
          } 
          else {      // if you got a newline, then clear currentLine:
            // Was this an Upload request?
            if (found_upload) {
              Serial.println("Found an upload request!!");
            }
            found_upload = false;
            currentLine = "";
          }
        }     
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /restart")) {
          return_status = true;
        }
        if (currentLine.endsWith("GET /halloween")) {
          Global::selected_program = Global::PROGRAM_HALLOWEEN;
          return_status = true;
        }
        if (currentLine.endsWith("GET /christmas")) {
          Global::selected_program = Global::PROGRAM_CHRISTMAS;
          return_status = true;
        }
        if (currentLine.endsWith("Get /upload")) {
          found_upload = true;
        }
      }
    }
    // close the connection:
    client->stop();
    Serial.println("client disonnected");
    return return_status;
  }
  bool WifiCallback() {
    if (status != WL_CONNECTED) {
      Serial.print(F("Attempting to connect to Network named: "));
      Serial.println(ssid);                   // print the network name (SSID);

      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
      status = WiFi.begin(ssid, pass);
      return false;
    }
    if (status != last_status) {
      last_status = status;
      Serial.println("Connected to Wifi");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());

      // print your WiFi shield's IP address:
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);
      
      Serial.println("Starting server");
      server.begin();
    }
    WiFiClient client = server.available();
    if (client) {
      return Wifi::HandleConnection(&client);
    } else {
      return false;
    }
  }
}  // namespace Wifi  

#endif  // _WIFI_HTTP_SERVER_H_
