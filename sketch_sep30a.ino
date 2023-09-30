/*
  This software, the ideas and concepts is Copyright (c) David Bird 2021
  All rights to this software are reserved.
  It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
  5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies or substantial portions of the software
     and where the software use is visible to an end-user.
  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
  FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR
  A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OR
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://dsbird.org.uk
*/
//################# LIBRARIES ################
#include <WiFi.h>                      // Built-in
#include <ESPmDNS.h>                   // Built-in
#include <SPIFFS.h>                    // Built-in
#include "ESPAsyncWebServer.h"         // Built-in
#include "AsyncTCP.h"                  // https://github.com/me-no-dev/AsyncTCP
//#include <AsyncElegantOTA.h>

//################  VERSION  ###########################################
String version = "1.0";      // Programme version, see change log at end
//################ VARIABLES ###########################################

const char* ServerName = "Controller"; // Connect to the server with http://controller.local/ e.g. if name = "myserver" use http://myserver.local/

#define Channels        8              // n-Channels
#define NumOfEvents     4              // Number of events per-day, 4 is a practical limit

struct settings {
  String DoW;                          // Day of Week for the programmed event
  String Start[NumOfEvents];           // Start time
  String Stop[NumOfEvents];            // End time
};

String       DataFile = "params.txt";  // Storage file name on flash
settings     Timer[Channels][7];       // Timer settings, n-Channels each 7-days of the week

//################ VARIABLES ################
const char* ssid       = "General_Yang";               // WiFi SSID     replace with details for your local network
const char* password   = "8587056243";           // WiFi Password replace with details for your local network
String sitetitle            = "8-Channel Relay Controller";
String webpage              = "";         // General purpose variables to hold HTML code for display

AsyncWebServer server(80); // Server on IP address port 80 (web-browser default, change to your requirements, e.g. 8080

// To access server from outside of a WiFi (LAN) network e.g. on port 8080 add a rule on your Router that forwards a connection request
// to http://your_WAN_address:8080/ to http://your_LAN_address:8080 and then you can view your ESP server from anywhere.
// Example http://yourhome.ip:8080 and your ESP Server is at 192.168.0.40, then the request will be directed to http://192.168.0.40:8080

//#########################################################################################
void setup() {
  SetupSystem();                          // General system setup
  StartWiFi();                            // Start WiFi services
//  SetupTime();                            // Start NTP clock services
//  StartSPIFFS();                          // Start SPIFFS filing system
  Initialise_Array();                     // Initialise the array for storage and set some values
//  RecoverSettings();                      // Recover settings from LittleFS
  SetupDeviceName(ServerName);            // Set logical device name

  // Set handler for '/'
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->redirect("/homepage");       // Go to home page
  });
  // Set handler for '/homepage'
  server.on("/homepage", HTTP_GET, [](AsyncWebServerRequest * request) {
    Homepage();
    request->send(200, "text/html", webpage);
  });
  server.begin();
}
//#########################################################################################
void loop() {
}
//#########################################################################################
void Homepage() {
  append_HTML_header();

  webpage += "<table>";

  for (byte timer = 0; timer < 48; timer++) { // 0 to 24 hours in mins, reported in 30-min increments hence 60 / (2) = 30
    webpage += "<tr>";
    webpage += "<td>" + String((timer / 2 < 10 ? "0" : "")) + String(timer / 2) + ":" + String(((timer % 2) * 30) < 10 ? "0" : "") + String((timer % 2) * 30) + "</td>"; // 4 x 15-min intervals per hour
    for (byte DoW = 0; DoW < 7; DoW++) {
      webpage += "<td style='color:red'>1</td>";
      webpage += "<td style=''>2</td>";
      webpage += "<td>3</td>";
      webpage += "<td>4</td>";
      webpage += "<td>5</td>";
      webpage += "<td>6</td>";
      webpage += "<td>7</td>";
      webpage += "<td>8</td>";
    }
    webpage += "</tr>";
  }
  webpage += "</table>";
  webpage += "<br>";
  append_HTML_footer();
}
//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################
void append_HTML_header() {
  webpage  = "<!DOCTYPE html><html lang='en'>";
  webpage += "<head>";
  webpage += "<title>" + sitetitle + "</title>";
  webpage += "<meta charset='UTF-8'>";
  webpage += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>";
  webpage += "<style>";
  webpage += "body {width:auto;margin-left:auto;margin-right:auto;font-family:Arial,Helvetica,sans-serif;font-size:14px;color:blue;background-color:#e1e1ff;text-align:center;}";

  webpage += ".coff {background-color:gainsboro;}";
  webpage += ".c1on {background-color:yellow;}";
  webpage += ".c2on {background-color:orange;}";
  webpage += ".c3on {background-color:red;}";
  webpage += ".c4on {background-color:pink;}";
  webpage += ".c5on {background-color:blue;}";
  webpage += ".c6on {background-color:green;}";
  webpage += ".c7on {background-color:purple;}";
  webpage += ".c8on {background-color:brown;}";

  webpage += "</style></head>";
  webpage += "<body>";
  webpage += "</div><br>";
}
//#########################################################################################
void append_HTML_footer() {
  webpage += "</body></html>";
}
//#########################################################################################
void SetupDeviceName(const char *DeviceName) {
  if (MDNS.begin(DeviceName)) { // The name that will identify your device on the network
    Serial.println("mDNS responder started");
    Serial.print("Device name: ");
    Serial.println(DeviceName);
    MDNS.addService("n8i-mlp", "tcp", 23); // Add service
  }
  else
    Serial.println("Error setting up MDNS responder");
}
//#########################################################################################
void StartWiFi() {
  Serial.print("\r\nConnecting to: "); Serial.println(String(ssid));
  IPAddress dns(8, 8, 8, 8); // Use Google as DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);       // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(50);
  }
  Serial.println("\nWiFi connected at: " + WiFi.localIP().toString());
}
//#########################################################################################
void SetupSystem() {
  Serial.begin(115200);                                           // Initialise serial communications
  while (!Serial);
  Serial.println(__FILE__);
  Serial.println("Starting...");
}
//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################

//#########################################################################################
void Initialise_Array() {
  for (int channel = 0; channel < Channels; channel++) {
    Timer[channel][0].DoW = "Sun";
    Timer[channel][1].DoW = "Mon";
    Timer[channel][2].DoW = "Tue";
    Timer[channel][3].DoW = "Wed";
    Timer[channel][4].DoW = "Thu";
    Timer[channel][5].DoW = "Fri";
    Timer[channel][6].DoW = "Sat";
  }
}
//#########################################################################################

//#########################################################################################

//#########################################################################################

/*
   Version 1.0 667 lines of code
   All HTML fully validated by W3C
*/
