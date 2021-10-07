
/**********************************************************************
* Project: Interfacing Arduino with LCD(16x2)
* Devices: Arduino Uno 
* Authors: Ramanpreet Kaur
* This is the program written to print Hello World in LCD
***********************************************************************/
#include <LiquidCrystal.h>
#include <Wire.h>                                                 
//**********************************************************************
//--- Declarations---
//**********************************************************************
const int rs = 13, en = 12, d4 = 11, d5 = 8, d6 = 9, d7 = 10;     // initialize the library with the numbers of the interface pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                        // REGISTER SELECT PIN,ENABLE PIN,D4 PIN,D5 PIN, D6 PIN, D7 PIN
//**********************************************************************
//--- Void Setup---
//**********************************************************************
void setup() {
Serial.begin(9600);                                               // to set speed(baud rate) for serial communication i.e. 9600 
lcd.begin(16, 2);
Wire.begin();
}
//**********************************************************************
//--- Void Loop---
//**********************************************************************
void loop()
{
lcd.setCursor(0, 0);                                              //set the cursor to column 0, line 1
lcd.print("Hello World");                                         //print Hello World
}
