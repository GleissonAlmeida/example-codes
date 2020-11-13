#include "DHT.h"

#define DHTPIN PB5 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
#include <LiquidCrystal_I2C.h>
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3f, 16, 4); //Configuraçao do LCD
void setup()
{
  Serial.begin(9600);
  Serial.println("DHT STM32 TEST!!!");
  lcd.begin();
  lcd.backlight();
  dht.begin();
  lcd.setCursor(0, 0);
  lcd.print("DHT STM32 TEST");
  delay(5000);
}

void loop()
{
  // A leitura da temperatura e umidade pode levar 250ms!
  // O atraso do sensor pode chegar a 2 segundos.
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // testa se retorno é valido, caso contrário algo está errado.
  if (isnan(t) || isnan(h))
  {
    Serial.println("Failed to read from DHT");
  }
  else
  {
    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.print(" %t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp.:       ");
    lcd.setCursor(7, 0);
    lcd.print(t);
    
    lcd.setCursor(0, 1);
    lcd.print("Umid.:       ");
    lcd.setCursor(7, 1);
    lcd.print(h, 1);
    delay(500);














  }
}
