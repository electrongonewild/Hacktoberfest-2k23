/*
 * Device Tested on : TMS320F28379D Launchpad
 * Code   : LCD Functions File in 4 Bit Mode
 * Author: Shikha Singhal 
 *
 * To use in main file : Write the following lines and debug the code on launchpad
 *       Lcd_Cmd(0x01);                      // Clear LCD Display
 *       Lcd_out(1, 4, "Hello");             // Print "Hello" in 1st row starting at 4th column
 *       Lcd_out(2, 1, "Electrongonewild");  // Print "Electrongonewild" in 2nd row starting at 1st column
 * 
                                 DSP Launchpad Connections with LCD
                                            TMS320F28379D
                                ------------------------------------
                      EN   <-- |GPIO61                              |
                      RS   <-- |GPIO123                             |
                      D7   <-- |GPIO3                               |
                      D6   <-- |GPIO58                              |
                      D5   <-- |GPIO59                              |
                      D4   <-- |GPIO124                             |
                      R/W  <-- |GND                                 |

 
 */
#include "Header.h"    // Include the header of your project

// LCD Pin Definitions
#define enablePinON    GpioDataRegs.GPBSET.bit.GPIO61
#define rsPinON        GpioDataRegs.GPDSET.bit.GPIO123
#define d7ON           GpioDataRegs.GPASET.bit.GPIO3
#define d6ON           GpioDataRegs.GPBSET.bit.GPIO58
#define d5ON           GpioDataRegs.GPBSET.bit.GPIO59
#define d4ON           GpioDataRegs.GPDSET.bit.GPIO124

#define enablePinOFF   GpioDataRegs.GPBCLEAR.bit.GPIO61
#define rsPinOFF       GpioDataRegs.GPDCLEAR.bit.GPIO123
#define d7OFF          GpioDataRegs.GPACLEAR.bit.GPIO3
#define d6OFF          GpioDataRegs.GPBCLEAR.bit.GPIO58
#define d5OFF          GpioDataRegs.GPBCLEAR.bit.GPIO59
#define d4OFF          GpioDataRegs.GPDCLEAR.bit.GPIO124

void Delay_ms(Uint16 value){
    Uint16 i;
    for(i = 0; i < value; i++){
       DelayUs(1000);
    }
}

Uint16 stringLen(char* str){
    int length = 0;
    while(str[length] != '\0'){
        length += 1;
    }
    return length;
}

void LcdInit(void){
    Delay_ms(15);           // LCD Power on delay
    Lcd_Cmd(0x02);          // Initialization of LCD
    Lcd_Cmd(0x28);          // Initialize 5*8 matrix in (4-bit mode)
    Lcd_Cmd(0x01);          // Clear display
    Lcd_Cmd(0x0C);          // Display on, Cursor off
    Lcd_Cmd(0x06);          // Increment cursor (shift cursor to right)
}

void Lcd_Cmd(unsigned char value){
    // Upper Nibble
    if((value & 0x10) == 0)
      d4OFF = 1;
    else
      d4ON = 1;

    if((value & 0x20) == 0)
      d5OFF = 1;
    else
      d5ON = 1;

    if((value & 0x40) == 0)
      d6OFF = 1;
    else
      d6ON = 1;

    if((value & 0x80) == 0)
      d7OFF = 1;
    else
      d7ON = 1;

    // Command Register is selected RS = 0
    rsPinOFF = 1;     

    // High-to-low pulse on Enable pin to latch data
    enablePinON = 1;
    Delay_ms(1);
    enablePinOFF = 1;
    Delay_ms(1);

    // Lower Nibble
    if((value & 0x01) == 0)
       d4OFF = 1;
    else
       d4ON = 1;

    if((value & 0x02) == 0)
      d5OFF = 1;
    else
      d5ON = 1;

    if((value & 0x04) == 0)
       d6OFF = 1;
    else
       d6ON = 1;

    if((value & 0x08) == 0)
        d7OFF = 1;
    else
        d7ON = 1;

    // High-to-low pulse on Enable pin to latch data
    enablePinON = 1;
    Delay_ms(1);
    enablePinOFF = 1;
    Delay_ms(15);
}

void LcdData(unsigned char value){
    // Upper Nibble
    if((value & 0x10) == 0)
      d4OFF = 1;
    else
      d4ON = 1;

    if((value & 0x20) == 0)
      d5OFF = 1;
    else
      d5ON = 1;

    if((value & 0x40) == 0)
      d6OFF = 1;
    else
      d6ON = 1;

    if((value & 0x80) == 0)
      d7OFF = 1;
    else
      d7ON = 1;

    // Data Register is selected RS = 1
    rsPinON = 1;        

    // High-to-low pulse on Enable pin to latch data
    enablePinON = 1;
    Delay_ms(1);
    enablePinOFF = 1;
    Delay_ms(1);

    //Lower Nibble
    if((value & 0x01) == 0)
       d4OFF = 1;
    else
       d4ON = 1;

    if((value & 0x02) == 0)
      d5OFF = 1;
    else
      d5ON = 1;

    if((value & 0x04) == 0)
       d6OFF = 1;
    else
       d6ON = 1;

    if((value & 0x08) == 0)
        d7OFF = 1;
    else
        d7ON = 1;

    // High-to-low pulse on Enable pin to latch data
    enablePinON = 1;
    Delay_ms(1);
    enablePinOFF = 1;
    Delay_ms(3);
}

void Lcd_out(Uint16 rowNumber, Uint16 colNumber, char* dataDisplay){
    Uint16 j,len;
    unsigned char Display , rowCount;
    len = stringLen(dataDisplay);
    if(rowNumber == 1){
        rowCount = 127 + colNumber;
        Lcd_Cmd(rowCount);
    }
    else{
        rowCount = 191 + colNumber;
        Lcd_Cmd(rowCount);
    }

    for(j= 0; j < len ;j++){
       Display = dataDisplay[j];
       LcdData(Display);
    }
}
