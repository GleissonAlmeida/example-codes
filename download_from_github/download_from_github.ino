/*
  //******************************************************************************************
  Copyright (c) <18/07/2022> <Gleisson Almeida.>

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
const char*     link =      "https://docs.arduino.cc/static/6ec5e4c2a6c0e9e46389d4f6dc924073/A000066-pinout.png";
//const char*     link =      "https://content.arduino.cc/assets/Pinout-Mega2560rev3_latest.pdf";


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
  delay(2000);                   //Aguarda a estabilizaçao do modulo.
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



int downloadFile(String File_Url, boolean debug = true, String File_Name = "") {

  int StepProgress = 0;
  int beforeStep = 0;
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  if (File_Name == "") {
    File_Name = File_Url.substring(File_Url.lastIndexOf("/"), File_Url.length());
  }
  else
  {
    File_Name = String("/") + File_Name;
  }

  if (debug) {
    Serial.print("Connecting to:"); Serial.println(File_Url);
    Serial.print("Downloading:"); Serial.println(File_Name);
  }

  http.begin(client, File_Url); // abre o link
  int httpCode = http.GET(); //verifica se o link e valido

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) { //arquivo OK
      int len = http.getSize();
      int paysize = len;
      if (debug) Serial.printf("File size %d bytes\n", paysize);
    
      uint8_t buff[128] = { 0 };   // Cria o buffer para receber os dados
     
      WiFiClient * stream = http.getStreamPtr(); // Recebe os dados
      
      File configFile = SPIFFS.open(File_Name, "w"); //cria o arquivo na spifss
      if (!configFile) {
        if (debug)Serial.printf("Failed to create %s for write.\n", File_Name);
        return false;
      }
      if (debug) Serial.println("File Create and start Download.");

      int Step = paysize / 10;
      while (http.connected() && (len > 0 || len == -1)) {
        StepProgress = (100 - ((len / Step) * 10));
        if (beforeStep != StepProgress) {
          beforeStep = StepProgress;
          if (debug) Serial.println("Progres:" + String(StepProgress) + "%");   //Mostra o progresso do Download
        }
        size_t size = stream->available();
        if (size) {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          configFile.write(buff, c);
          if (len > 0) {
            len -= c;
          }
        }
      } 
      configFile.close(); //finaliza o arquivo
      http.end();
      Serial.println("Download complet.");
      return true;
    } else {
      Serial.printf("Download error - http error %d\n", httpCode);
      return false;
    }
  } else {
    Serial.printf("Conection erro %d\n", httpCode);
    return false;
  }
  return true;
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
        downloadFile(link,true,"xxx.vtx");
        printfiles();
        delay(2000);
        ledState = false;
      }
    }
  }
  digitalWrite(LED, ledState);
  lastButtonState = reading;
}
