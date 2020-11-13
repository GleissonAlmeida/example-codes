/*
Copyright <2020> <.>

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
*/

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f, 16, 4); //Configuraçao do LCD
//******************************************************************************************
// VARIABLES / CONSTANTES
//******************************************************************************************
const int PhSensor = PA6;
int n_read_adc = 20;
//******************************************************************************************
// READ PH VALUE
//******************************************************************************************
float read_adc() {
  float adc_value = 0;
  for (int i = 0; i < n_read_adc; ++i) {
    adc_value += analogRead(PhSensor);  //12BITS FOR STM32
    delay(10);
  }
  adc_value /= n_read_adc;
  return adc_value;
}

float read_ph() {
  float phValue=(read_adc ()*5.0/4096);
  phValue = 3.5 *phValue; 
  return phValue;
}

//******************************************************************************************
// SETUP ROUTINE
//******************************************************************************************
void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  pinMode(PA6, INPUT_ANALOG); // set up pin for analog input pH sensor
  delay(1000);
  Serial.println("PH STM32 TEST - Ver 1.0!");
  Serial.println("Autor - sparkbr@hotmail.com");
 lcd.setCursor(0, 0);
 lcd.print("  PH STM32 TEST ");
 lcd.setCursor(0, 1);
 lcd.print("                 ");
}
//******************************************************************************************
// MAIN ROUTINE
//******************************************************************************************
void loop() {
  lcd.setCursor(3, 1);
  lcd.print("Ph - ");
  if (read_ph() < 0.0) {
    lcd.setCursor(8, 1);
  }
  else if (read_ph() >= 10.00)
  {
    lcd.setCursor(8, 1);
  }
  else {
    lcd.setCursor(8, 1);
    lcd.print(" ");
    lcd.setCursor(9, 1);
  }
  lcd.print (read_ph(), 2);
  delay(100);
}
