/*
  //******************************************************************************************
  Basedo no codigo :https://github.com/gordonthree/multiswitch
  Copyright (c) <18/07/2022> <>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  //*****************************************************************************************
*/

const char* ssid = "VIVOFIBRA-3490";
const char* password = "zqmS2S2Cg8";

//const char*     link =      "https://raw.githubusercontent.com/Spark-br/UPDT_IOT/main/500ms.bin";
//const char*     link =      "https://docs.arduino.cc/static/6ec5e4c2a6c0e9e46389d4f6dc924073/A000066-pinout.png";
const char*     link =      "https://content.arduino.cc/assets/Pinout-Mega2560rev3_latest.pdf";


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include <FS.h>
//#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>



#define LED 2
#define BOOT 12
int ledState = HIGH;
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;




void wifiConnect() {
  Serial.println("");
  WiFi.disconnect( true );       //Força desconecçao do dispositivo.
  WiFi.mode(WIFI_STA);
  delay(5000);                   //Aguarda a estabilizaçao do modulo.
  WiFi.begin(ssid, password);

  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {  //Tempo de tentativa de conecxao - 60 segundos
    b++;
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP:"); Serial.println(WiFi.localIP());
}


void printfiles() {
  Serial.println("Mostrando arquivos na SPIFSS:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if (dir.fileSize()) {
      File f = dir.openFile("r");
      Serial.print(" Size of file:"); Serial.println(f.size());
    }
  }
}





int downloadFile(String fileurl) { // upload new file to fs by downloading from a remote server, rather than reflash the entire spiffs
  int ret = false;
  int StepProgress = 0;
  int Step = 0;
  int beforeStep = 0;
  String aux;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String filename = fileurl.substring(fileurl.lastIndexOf("/"), fileurl.length());
  //  if (!filename.startsWith("/"))
  //  {
  //    filename = String("/") + filename;
  //  }

  Serial.print("url:"); Serial.println(fileurl);
  Serial.print("file:"); Serial.println(filename);
  http.begin(client, fileurl); // init http client
  // start connection and send HTTP header
  int httpCode = http.GET();
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      Serial.printf("HTTP client http status %d\n", httpCode);
      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      int paysize = len;
      Serial.printf("HTTP content size %d bytes\n", paysize);
      // create buffer for read
      uint8_t buff[128] = { 0 };
      // get tcp stream
      WiFiClient * stream = http.getStreamPtr();
      // create or recreate file on spiffs
      File configFile = SPIFFS.open(filename, "w");
      if (!configFile) {
        Serial.printf("Failed to open %s for write.\n", filename);
        return ret;
      }
      Serial.println("File open, write start.");
      // read all data from server

      Step = paysize / 10;
      while (http.connected() && (len > 0 || len == -1)) {
        StepProgress = (100 - ((len / Step)*10));
        if (beforeStep != StepProgress) {
          beforeStep = StepProgress;
          Serial.println("Progres:" + String(StepProgress) + "%");
        }
        size_t size = stream->available();
        if (size) {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          // write it to Serial
          configFile.write(buff, c);
          if (len > 0) {
            len -= c;
          }
        }
      } // EoF or http connection closed
      configFile.close();
      http.end();
      Serial.println("File closed, write complete.");
      return paysize;
    } else {
      Serial.printf("HTTP client http error %d\n", httpCode);
      return httpCode;
    }
  } else {
    Serial.printf("HTTP client http error %d\n", httpCode);
    return httpCode;
  }
  return 0;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BOOT, INPUT );
  SPIFFS.begin();
  Serial.begin(115200, SERIAL_8E1);
  Serial.println("");
  wifiConnect();
  printfiles();

}


void loop() {

  int reading = digitalRead(BOOT);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 250) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        ledState = !ledState;
        digitalWrite(LED, ledState);
        downloadFile(link);
        printfiles();
        delay(5000);
        ledState = false;
      }
    }
  }
  digitalWrite(LED, ledState);
  lastButtonState = reading;
}

