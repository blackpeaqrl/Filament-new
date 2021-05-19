/*

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>

*/

#pragma region Headres

#include <TimerOne.h>

#include <LiquidCrystal_I2C.h> //SDA = A4, SCL = A5

#pragma endregion

#pragma region Variables

LiquidCrystal_I2C lcd(0x27, 16, 2);
float result;
int d = 0;
int d_old = -1;
int i;
int sign;
long value;
long pwm;
int clockpin = 9;
int datapin = 6;
int period = 128;
int pwmpin = A3;
unsigned long tempmicros;

#pragma endregion

/**
 * @brief Setup
 * 
 */
void setup()
{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.home(); // go home (0, 0)
  lcd.print("Width:");
  lcd.setCursor(0, 1);
  lcd.print("Speed:");

  pinMode(clockpin, INPUT);
  pinMode(datapin, INPUT);
  pinMode(pwmpin, INPUT);
  Timer1.initialize(period);
  makeOutput(0);

  Serial.println("Ready");
}

/**
 * @brief Loop
 * 
 */
void loop()
{
  while (digitalRead(clockpin) == HIGH) {}
  tempmicros = micros();
  while (digitalRead(clockpin) == LOW) {}
  if ((micros() - tempmicros) > 500) {
    decode();
  }
}

#pragma region Functions

/**
 * @brief Make output of te extruder to diameter.
 * 
 * @param um Diameter.
 */
void makeOutput(int um)
{
    digitalWrite(clockpin, HIGH);
    int x = analogRead(pwmpin);
    int value = d;
    d = (x / 100) * 1;
    if (d <= 1) d = 1;
    while (analogRead(pwmpin) == HIGH) {}
    while (digitalRead(clockpin) == LOW) {}
    Timer1.pwm(pwmpin, value);
}

/**
 * @brief Decode caliper value.
 * 
 */
void decode()
{
    // Clear values.
  sign = 1;
  i = 0;
  value = 0;
  result = 0;
  digitalRead(datapin);
  for (i = 0; i < 24; i++) {
    // Wait for clock pin to get high.
   while (digitalRead(clockpin) == HIGH) {}
   // Wait for clock pin to get low.
   while (digitalRead(clockpin) == LOW) {} 
   // Data pin is low?
    if (digitalRead(datapin) == HIGH) {
      if (i < 20) {
        value |= 1 << i;
      }
      if (i == 20) {
        sign = -1;
      }

    }

  }
    
    result = (value * sign) / 100.00;
    makeOutput(value);
    
    // Return interupts.
    interrupts();
    
    lcd.setCursor(7, 0);
    lcd.print(result, 4);
    lcd.print(" in     ");
    lcd.setCursor(7, 1);
    lcd.print(d);
    lcd.print(" ");
    noInterrupts();
    delay(1000);
    
    result = (value * sign) / 100.00;
    interrupts();
    lcd.setCursor(7, 0);
    lcd.print(result, 2);
    lcd.print(" mm     ");
    lcd.setCursor(7, 1);
    lcd.print(d);
    lcd.print(" ");
    noInterrupts();
    
    Serial.println(result, 2);
    delay(1000);
}

#pragma endregion
