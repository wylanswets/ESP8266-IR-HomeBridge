#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <EEPROM.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

int sendTimes = 10; //How many times to send the signals defined in sendIROn() or sendIROff()
int Current_State = 0; //When booting it will also send sendIROff() so we can "know for sure" that the device is off and report to homekit
int delay_ms = 50; //delay to run between sending batches of IR signals to prevent crash

const uint16_t kIrLed = 15;  // ESP8266 GPIO pin to use. Label: D8 (works with a few others too if needed, but you're on your own to test)
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

ESP8266WebServer server(80);

void setup(void){
  irsend.begin();
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
    
  server.on("/", handleRoot);

  server.on("/status", []() {

    server.send(200, "text/html", (String)Current_State);
    
  });

  server.on("/on", []() {
    server.send(200, "text/html", "on");
    sendIROn();
    Current_State = 1;
  });

  server.on("/off", []() {
    server.send(200, "text/html", "off");
    sendIROff();
    Current_State = 0;
  });

  server.onNotFound(handleNotFound);
  server.begin();

  //Initially send off signal... so our state is in sync :D
  sendIROff();
}

//main loop for handling clients
void loop(void){
  server.handleClient();
}

String getMac() {
  unsigned char macarr[6];
  WiFi.macAddress(macarr);
  
  String mac = String(macarr[0], HEX); mac += ":";
  mac += String(macarr[1], HEX); mac += ":";
  mac += String(macarr[2], HEX); mac += ":";
  mac += String(macarr[3], HEX); mac += ":";
  mac += String(macarr[4], HEX); mac += ":";
  mac += String(macarr[5], HEX);
  return mac;
}

String getIP() {
  IPAddress ip = WiFi.localIP();
  String ret = (String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3];
  return ret;
}

void handleRoot() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["mac"] = getMac();
  root["ip"] = getIP();
  root["state"] = (String)Current_State;
  
  String snd;
  root.printTo(snd);

  server.send(200, "application/json", snd);

}

void handleNotFound(){
  server.send(404, "text/html", "Method not found.");
}

void sendIROff() {
  //If you have more than one IR signal to send, define them all here
  uint16_t rawData1[23] = {1200,450, 1250,450, 350,1300, 400,1250, 400,1300, 350,1300, 400,1300, 1200,450, 1200,450, 1200,500, 1200,450, 1200};

  //Send the off signals
  for(int i = 0; i<sendTimes; i++) {
    //Send all of the signals in different sendRaw calls here (if you have more than one)
    irsend.sendRaw(rawData1, 24, 38);
    delay(delay_ms);
  }
}

void sendIROn() {
  //If you have more than one IR signal to send, define them all here
  uint16_t rawData1[23] = {1200,450, 1200,450, 400,1300, 350,1300, 350,1300, 400,1300, 350,1300, 400,1300, 350,1300, 350,1300, 1200,500, 350};

  //Send the on signals
  for(int i = 0; i<sendTimes; i++) {
    //Send all of the signals in different sendRaw calls here (if you have more than one)
    irsend.sendRaw(rawData1, 24, 38);
    delay(delay_ms);
  }
}
