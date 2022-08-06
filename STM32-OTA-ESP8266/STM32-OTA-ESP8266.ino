/*
<2022> <Gleisson Almeida>

*basead  from original code CS.NOL
*2017  CS.NOL  https://github.com/csnol/STM32-OT*

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    ESP8266/8285 Pin       STM32 MCU      NodeMCU Pin(ESP8266 based)
    RXD                    PA9             RXD
    TXD                    PA10            TXD
    Pin4                   BOOT0           D2
    Pin5                   RST             D1
    Vcc                    3.3V            3.3V
    GND                    GND             GND
    En -> 10K -> 3.3V
    TX-SERIAL (PC)                         D6       
    RX-SERIAL (PC)                         D7     

*/
#include "stm32ota.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

SoftwareSerial Debug;
#define MYPORT_TX 12
#define MYPORT_RX 13




const char* ssid = "MY"; //you ssid
const char* password = "1234567890"; //you password
const char*     link =        "https://raw.githubusercontent.com/Spark-br/UPDT_IOT/main/500ms.bin";

//----------------------------------------------------------------------------------
void wifiConnect() {
  Debug.println("");
  WiFi.disconnect( true );       //Força desconecçao do dispositivo.
  WiFi.mode(WIFI_STA);
  delay(2000);                   //Aguarda a estabilizaçao do modulo.
  WiFi.begin(ssid, password);
  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {  //Tempo de tentativa de conecxao - 60 segundos
    b++;
    Debug.print(".");
    delay(500);
  }
  Debug.println("");
  Debug.print("IP:"); Debug.println(WiFi.localIP());
}
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
boolean printfiles() {
  Debug.println("Files in SPIFSS:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Debug.print(dir.fileName());
    if (dir.fileSize()) {
      File f = dir.openFile("r");
      Debug.print(" Size of file:"); Debug.println(f.size());
    }
  }
  return true;
}

stm32ota STM32(5, 4, 2);

void setup() {
  Debug.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false); // for debug
  Serial.begin(115200, SERIAL_8E1);
  Debug.println("DEBUG SOFTWARESERIAL");
  SPIFFS.begin();
  delay(5000);
  wifiConnect(); 
  Debug.println(STM32.otaUpdate(link));
  Debug.println("END OF UPDT");
}

void loop() {
  // put your main code here, to run repeatedly:

}
