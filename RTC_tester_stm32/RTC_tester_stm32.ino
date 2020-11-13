
#include <LiquidCrystal_I2C.h>
#include <RTClock.h>



RTClock rtclock (RTCSEL_LSE); // initialise
int timezone = 8;      // change to your timezone
time_t tt, tt1;
tm_t mtt;
LiquidCrystal_I2C lcd(0x3f, 16, 4); //Configuraçao do LCD
const char * delim = " :";
char s[128]; // for sprintf

void setup()
{
bkp_init();
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RTC STM32 TEST");
}

void loop()
{
 bb_peri_set_bit(&BKP_BASE->RTCCR, BKP_RTCCR_CCO_BIT, 1);      // Calibration clock output
  rtclock.breakTime(rtclock.now(), mtt);
  // sprintf(s, "RTC timestamp: %s %u %u, %s, %02u:%02u:%02u\n",
  //         months[mtt.month], mtt.day, mtt.year + 1970, weekdays[mtt.weekday], mtt.hour, mtt.minute, mtt.second);
  // Serial.print(s);
  sprintf(s, "TIME: %02u:%02u:%02u", mtt.hour, mtt.minute, mtt.second);
  lcd.setCursor(0, 1);
  lcd.print(s);
  Serial.println(s);
}
