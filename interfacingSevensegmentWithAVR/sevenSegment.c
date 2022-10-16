/*
    Interfacing seven segment display with AVR microcontroller

    Device tested on: ATMega 16/ ATMega 32

    Author : Nikhil Vashisht
 */

#include<avr/io.h>

int main(void){

    DDRB = 0XFF; //set the LED port direction to output
    PORTB = 0XFF; //value to be supplied to output

    char arr[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90}; //hex values for seven segment display ranging from 0 to 9

    while(1){

        for(int i = 0; i < 10; i++){
            PORTB = arr[i];  //writing the value to output
            _delay_ms(1000);  //delay for 1 s
        }

    }

    return 0;
}