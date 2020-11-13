
//******************************************************************************************
//PROGRMA TESTE PARA BALNAÇA FEITA COM ARDUINO.
//O PROGRMA POSSUI UMA ROTINA DE AUTO CALIBRAÇAO, O QUE FACILITA O PROCESSO DE IMPLEMENTAÇAO
//FINAL, DE MODO QUE NAO NECESSITA MAIS DE UMA SKETCH PARA ACHAR O VALOR DA CALIBRAÇAO.
//ISSO PERMITE QUE A CELULA DE CARGA POSSA SER TROCADA A QUALQUR MOMENTO DO PROJETO.
//******************************************************************************************
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <EEPROM.h>
#define DOUT  PB15
#define CLK  PB14

HX711 scale;
LiquidCrystal_I2C lcd(0x3f, 16, 4); //Configuraçao do LCD
//******************************************************************************************
// DECLARAÇAO DAS CONSTANTES / VARIAVEIS
//******************************************************************************************
const int CursorPos = PB3;     // botao  Enter
const int buttonExit = PB4;    // botao  exit
const int button_Up = PB8;    // botao  up
const int button_Down = PB9;  // botao  Down
int CP = 9;                  // posiçao inicial do cursor
//volatile float encoderPosition = 0;          // valor inicial da varia
int reading = 0;             //variavel auxiliar para o lcd.vel
int encoderPosition = 0;
volatile unsigned long factor = 1000;           // numero de steps
volatile unsigned long encoderMax = 30000;    // Valor maximo da variavel.
float peso_cal = 0.1; //peso em gramas para evitar o uso de float.
float valor_lido;
float peso_lido;
long valor_call = 447000;
int XX = 0;
int Aux1 = 0;
//******************************************************************************************
// FUNÇAO DE LEITURA E SALVAMENTO DE DADOS DA EEPROM PARA VARIAVAEL TIPO LONG
//******************************************************************************************
long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
//******************************************************************************************
// ROTINAS DE AUTO CALIBRAÇAO DO DISPOSITIVO
//******************************************************************************************
void auto_calibration() {   //Rotina de calibraçao automatica da celula de carga

  Serial.println("Calibration process");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press Enter for 5 seconds to save data");
  lcd.setCursor(0, 0);
  lcd.print("Calibration Menu");
  lcd.setCursor(0, 1);
  lcd.print("Weight");
  scale.tare();
  delay(2000);
  while (Aux1) {
    scale.set_scale(valor_call); // carrega a variavel de calibraçao.
    peso_lido = scale.get_units(5);
    reading = encoderPosition;
    peso_cal = (reading / 1000.00);
    CursorPosition();
    botao();
    if (peso_cal >= 10.00)
    {
      lcd.setCursor(8, 1);
    }
    else {
      lcd.setCursor(8, 1);
      lcd.print("0");
      lcd.setCursor(9, 1);
    }
    lcd.print (peso_cal, 3);
    lcd.setCursor(14, 1);
    lcd.print("Kg");
    lcd.setCursor (CP, 1);                              //Posisiona o curssor conforme a variavel
    lcd.cursor();
    while (digitalRead(CursorPos) == LOW && XX < 25) {
      delay(100);
      XX =  ++XX;
    }
    if (digitalRead(buttonExit) == LOW) { //Sai do menu de calibraçao.
      Aux1 = 0;
      lcd.setCursor(0, 0);
      lcd.print("Arduino Balance!");
      lcd.setCursor(0, 1);
      lcd.print("                 ");
      Serial.println("Calibration Process end without saving data");
      delay(100);
    }
    if (XX >= 25) {                       // Salva os dados
      valor_lido = scale.get_value(10);   // Realiza 10 amostras para um valor mais preciso.
      valor_call = valor_lido / peso_cal; // Realiza o calculo da variavel de calibraçao.
      EEPROMWritelong(0, valor_call);
      Serial.println("Calibration Process Completed");
      lcd.setCursor(0, 1);
      lcd.print("Saved Data       ");
      delay(2000);
      lcd.setCursor(0, 0);
      lcd.print("Arduino Balance!");
      lcd.setCursor(0, 1);
      lcd.print("                 ");
      //lcd.setCursor(0, 2);
      //lcd.print("                 ");
      Aux1 = 0;
    }
    else {
      XX = 0;
    }
    delay(100);
  }
}
//******************************************************************************************
// ROTINAS DE CONTROLE DOS DADOS NO LCD
//******************************************************************************************
void botao() { // funçao de controle dos botoes.
  if (digitalRead(button_Up) == LOW) {
    encoderPosition = encoderPosition + factor;
  }
  if (digitalRead(button_Down) == LOW) {
    encoderPosition = encoderPosition - factor;
  }
  encoderPosition = min(encoderMax, max(0, encoderPosition));
  if (digitalRead(button_Up) == LOW) {
  }
}

void CursorPosition(void) {
  if (digitalRead(CursorPos) == LOW) {
    delay(100);
    CP = CP + 1;
    if (CP > 13) {
      CP = 9;
    }
    if (CP == 13) {
      factor = 1;
    }
    if (CP == 12) {
      factor = 10;
    }
    if (CP == 10) {
      CP = 11;
      factor = 100;
    }
    if (CP == 9) {
      factor = 1000;
    }
  }
}
//******************************************************************************************
// ROTINA DE CONFIGURAÇAO INICIAL
//******************************************************************************************

void setup() {
  Serial.begin(9600);
  delay(5000);
  scale.begin(DOUT, CLK);
  lcd.begin();
  lcd.backlight();
  lcd.print("Arduino Balance!");
  Serial.println("Arduino Weight Balance - Ver 2.1!");
  Serial.println("Autor - sparkbr@hotmail.com");
  Serial.println("For calibration mode pres Enter and exit, while booting the Arduino");
  pinMode(CursorPos, INPUT_PULLUP);
  pinMode(buttonExit, INPUT_PULLUP);
  pinMode(button_Up, INPUT_PULLUP);
  pinMode(button_Down, INPUT_PULLUP);
  valor_call = EEPROMReadlong(0);
  while (digitalRead(CursorPos) == LOW && digitalRead(buttonExit) == LOW && XX < 10) {
    delay(200);
    XX =  ++XX;
  }
  if (XX >= 10) {
    XX = 0;
    Aux1 = 1;
    auto_calibration();
  }
  scale.set_scale(valor_call);
  scale.tare(); //Reset the scale to 0
}
//******************************************************************************************
//  PROGRAMA PRINCIPAL
//******************************************************************************************
void loop() {

if (scale.wait_ready_timeout(100)) {

  if (digitalRead(buttonExit) == LOW) {
    scale.tare(); //Reset the scale to 0
    delay(100);
  }
  peso_lido = scale.get_units(5);
 if (peso_lido < 0.001 && peso_lido >-0.001){  //esttabiliza a leitura dos dados
  peso_lido = 0.00;
 }
}
 else
  peso_lido = 25.00;
  
  Serial.print("Weight:\t");
  Serial.println(peso_lido, 3);
  lcd.setCursor(0, 1);
  lcd.print("Weight");
  if (peso_lido < 0.0) {
    lcd.setCursor(8, 1);
  }
  else if (peso_lido >= 10.00)
  {
    lcd.setCursor(8, 1);
  }
  else {
    lcd.setCursor(8, 1);
    lcd.print("0");
    lcd.setCursor(9, 1);
  }
  lcd.print (peso_lido, 3);
  lcd.setCursor(14, 1);
  lcd.print("Kg");
  delay(100);
}
