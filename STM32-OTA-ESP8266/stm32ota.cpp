#include"stm32ota.h"
String bin_file_name;
const String STM32_CHIPNAME[8] = {
  "Unknown Chip",
  "STM32F03xx4/6",
  "STM32F030x8/05x",
  "STM32F030xC",
  "STM32F103x4/6",
  "STM32F103x8/B",
  "STM32F103xC/D/E",
  "STM32F105/107"
};



stm32ota::stm32ota(int _NRST, int _BOOT0 , int _LED) {

  this->pin_BOOT0 = _BOOT0;
  this->pin_LED = _LED;
  this->pin_NRST = _NRST;
  pinMode(_BOOT0, OUTPUT);
  pinMode(_NRST, OUTPUT);
  pinMode(_LED, OUTPUT);
  digitalWrite(pin_BOOT0, LOW);
  digitalWrite(pin_NRST, HIGH);
  
}

String stm32ota::conect() {

  String stringtmp;
  int rdtmp;
  delay(100);
  digitalWrite(pin_BOOT0, HIGH);
  delay(100);
  digitalWrite(pin_NRST, LOW);
  digitalWrite(pin_LED, LOW);
  delay(50);
  digitalWrite(pin_NRST, HIGH);
  delay(500);
  for ( int i = 0; i < 3; i++) {
    digitalWrite(pin_LED, !digitalRead(pin_LED));
    delay(100);
  }

  Serial.write(STM32INIT);
  delay(10);
  if (Serial.available() > 0);
  rdtmp = Serial.read();
  if (rdtmp == STM32ACK)   {
    stringtmp = STM32_CHIPNAME[GetId()];
  }
  else if (rdtmp == STM32NACK) {
    Serial.write(STM32INIT);
    delay(10);
    if (Serial.available() > 0);
    rdtmp = Serial.read();
    if (rdtmp == STM32ACK)   {
      stringtmp = STM32_CHIPNAME[GetId()];
    }
  }
  else
    stringtmp = "ERROR";
return stringtmp;
}
//------------------------------------------------------------------------------
unsigned char stm32ota:: GetId() {     // Tested
  int getid = 0;
  unsigned char sbuf[5];
  SendCommand(STM32ID);
  while (!Serial.available());
  sbuf[0] = Serial.read();
  if (sbuf[0] == STM32ACK) {
    Serial.readBytesUntil(STM32ACK, sbuf, 4);
    getid = sbuf[1];
    getid = (getid << 8) + sbuf[2];
    if (getid == 0x444)
      return 1;
    if (getid == 0x440)
      return 2;
    if (getid == 0x442)
      return 3;
    if (getid == 0x412)
      return 4;
    if (getid == 0x410)
      return 5;
    if (getid == 0x414)
      return 6;
    if (getid == 0x418)
      return 7;
  }
  else
    return 0;
}
//------------------------------------------------------------------------------
void stm32ota:: SendCommand(unsigned char commd) {    // Tested
  Serial.write(commd);
  Serial.write(~commd);
}
//----------------------------------------------------------------------------------
unsigned char stm32ota::Erase() {     //
  SendCommand(STM32ERASE);
  while (!Serial.available());
  if (Serial.read() == STM32ACK)
  {
    Serial.write(0xFF);
    Serial.write(0x00);
  }
  else return STM32ERR;
  while (!Serial.available());
  return Serial.read();
}
//------------------------------------------------------------------------------
unsigned char stm32ota::Erasen() {     //
  SendCommand(STM32ERASEN);
  while (!Serial.available());
  if (Serial.read() == STM32ACK)
  {
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0x00);
  }
  else return STM32ERR;
  while (!Serial.available());
  return Serial.read();
}
//------------------------------------------------------------------------------
boolean stm32ota::EraseChip() {  // modificar para boolean
  boolean aux;
  if (Erase() == STM32ACK)
    aux = true;
  else if (Erasen() == STM32ACK)
    aux = true;
  else
    aux = false;
  return aux;
}
//------------------------------------------------------------------------------
boolean stm32ota::Flash(String bin_file_name) {
  File fsUploadFile;
  boolean flashwr;
  int lastbuf = 0;
  uint8_t cflag, fnum = 256;
  int bini = 0;
  uint8_t binread[256];

  fsUploadFile = SPIFFS.open(bin_file_name, "r");
  if (fsUploadFile) {
    bini = fsUploadFile.size() / 256;
    lastbuf = fsUploadFile.size() % 256;
    for (int i = 0; i < bini; i++) {
      fsUploadFile.read(binread, 256);
      SendCommand(STM32WR);
      while (!Serial.available()) ;
      cflag = Serial.read();
      if (cflag == STM32ACK)
        if (Address(STM32STADDR + (256 * i)) == STM32ACK) {
          if (SendData(binread, 255) == STM32ACK)
          flashwr = true;
          else flashwr = false;
        }
    }
    fsUploadFile.read(binread, lastbuf);
    SendCommand(STM32WR);
    while (!Serial.available()) ;
    cflag = Serial.read();
    if (cflag == STM32ACK)
      if (Address(STM32STADDR + (256 * bini)) == STM32ACK) {
        if (SendData(binread, lastbuf) == STM32ACK)
          flashwr = true;
        else flashwr = false;
      }
    fsUploadFile.close();
  }
return flashwr;
}

//------------------------------------------------------------------------------
boolean stm32ota:: downloadFile(String File_Url, String File_Name = "") {
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

  bin_file_name = File_Name;
  http.begin(client, File_Url); // abre o link
  int httpCode = http.GET(); //verifica se o link e valido

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) { //arquivo OK
      int len = http.getSize();
      int paysize = len;
      uint8_t buff[128] = { 0 };   // Cria o buffer para receber os dados
      WiFiClient * stream = http.getStreamPtr(); // Recebe os dados
      File configFile = SPIFFS.open(File_Name, "w"); //cria o arquivo na spifss
      if (!configFile) {
        return false;
      }
      int Step = paysize / 10;
      while (http.connected() && (len > 0 || len == -1)) {
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
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
  return true;
}
//------------------------------------------------------------------------------
unsigned char stm32ota::Address(unsigned long addr) {

  unsigned char sendaddr[4];
  unsigned char addcheck = 0;
  sendaddr[0] = addr >> 24;
  sendaddr[1] = (addr >> 16) & 0xFF;
  sendaddr[2] = (addr >> 8) & 0xFF;
  sendaddr[3] = addr & 0xFF;
  for (int i = 0; i <= 3; i++) {
    Serial.write(sendaddr[i]);
    addcheck ^= sendaddr[i];
  }
  Serial.write(addcheck);
  while (!Serial.available());
  return Serial.read();
}
//------------------------------------------------------------------------------
unsigned char stm32ota::SendData(unsigned char * data, unsigned char wrlen) {
  Serial.write(wrlen);
  for (int i = 0; i <= wrlen; i++) {
    Serial.write(data[i]);
  }
  Serial.write(getChecksum(data, wrlen));
  while (!Serial.available());
  return Serial.read();
}
//------------------------------------------------------------------------------
unsigned char stm32ota::getChecksum( unsigned char * data, unsigned char datalen) {
  unsigned char lendata = datalen;
  for (int i = 0; i <= datalen; i++)
    lendata ^= data[i];
  return lendata;
}

//----------------------------------------------------------------------------------
void stm32ota::deletfiles(String bin_file) {
  Dir dir = SPIFFS.openDir("/");
  if (SPIFFS.exists(bin_file)) {
    SPIFFS.remove(bin_file);
  }
}
//----------------------------------------------------------------------------------
String stm32ota::binfilename() {
  String aux = bin_file_name;
  return aux;
}
//----------------------------------------------------------------------------------
void stm32ota:: RunMode()  {    //Tested  Change to runmode
  digitalWrite(pin_BOOT0, LOW);
  delay(100);
  digitalWrite(pin_NRST, LOW);
  digitalWrite(pin_LED, LOW);
  delay(50);
  digitalWrite(pin_NRST, HIGH);
  delay(200);
  for ( int i = 0; i < 3; i++) {
    digitalWrite(pin_LED, !digitalRead(pin_LED));
    delay(100);
  }
}
//----------------------------------------------------------------------------------
char stm32ota::chipVersion() {     // Tested
  unsigned char vsbuf[14];
  SendCommand(STM32GET);
  while (!Serial.available());
  vsbuf[0] = Serial.read();
  if (vsbuf[0] != STM32ACK)
    return STM32ERR;
  else {
    Serial.readBytesUntil(STM32ACK, vsbuf, 14);
    return vsbuf[1];
  }
}
//----------------------------------------------------------------------------------
String stm32ota::otaUpdate(String File_Url){
String aux = "";
if (WiFi.waitForConnectResult() == WL_CONNECTED){

  if (downloadFile(File_Url,"stm32.bin")){
   //printfiles();
    String aux = conect();
    if (aux!="ERROR"){
       EraseChip();
       Flash("/stm32.bin");
       RunMode();
       deletfiles("/stm32.bin");
    }
      else {
      return "Unknown Chip";
    }
  }
     else{
   return "Download Fail";
   }
  }
 else{
   return "WiFi not conected";
  }
   return "Update OK";
}
//----------------------------------------------------------------------------------
