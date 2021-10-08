//#########################################################################
// Code to interface MSP430FR5969 with RTC 
// Devices : MSP430FR5969
// Author : Shikha Singhal
// Code : Interfacing with RTC
//#########################################################################
//  In this code MSP430FR5969 transmits data to 0x68 slave address. In this 
//  case I2C slave is RTC.
//
//                                /|\  /|\
//                		  10k  10k (Pull-up resistors)    
//                MSP430FR5969     |    |         RTC
//             -----------------   |    |   -----------------
//            |     P1.6/UCB0SDA|<-|----+->|SDA	             |
//            |                 |  |       |                 |
//            |                 |  |       |                 |
//            |     P1.7/UCB0SCL|<-+------>|SCL	             |
//            |                 |          |                 |
//            |                 |          |                 |
//            |                 |          |                 |



#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define     RTCAddress       0x68				 //set RTC address 

void ConfigWDT(void);
void Delay_ms(unsigned int);
void InitI2C(void);
void setRTCTime(int,int,int);
void setRTCDate(int,int,int);
void readDateTimeRTC(void);

void RTC_WriteByte(int, int);
int RTC_ReadByte( int);
int Binary2BCD(int);
int BCD2Binary(int);
char BCD2UpperCh( char);
char BCD2LowerCh( char);

void I2CWriteInit(void);
void I2CReadInit(void);

//#########################################################################
// variables to set date and time
//#########################################################################

int yrCounter, monCounter, dtCounter;
int hourCounter = 0, minuteCounter = 0, secondsCounter = 0;
int hrCounter, minCounter, secCounter;

//#########################################################################
// variables to display date and time
//#########################################################################
int dateBCD, monthBCD, yearBCD, secondsBCD, minutesBCD, hoursBCD;
char Time[] ="00:00:00";
char Date[] ="00/00/00";

int main(void)
{
	ConfigWDT();									// Configure Watchdog Timer
        configureClocks();								// Configure Clocks
	InitI2C();									// Initialize I2C 

	Address = 9;									// Address value to write on EEPROM

	setRTCTime(12,22,0);            				                // Write time in RTC Address
	Delay_ms(1000);
	setRTCDate(07,10,21);                			                        // Write date in RTC Address 
        Delay_ms(1000);
        while(1)
       {
       	    readDateTimeRTC();                          				// Read date and time from RTC  
       	    Delay_ms(1000);
       }
}

void ConfigWDT()
{
    WDTCTL = WDTPW | WDTHOLD;                           // Configure Watchdog Timer
}

void configureClocks()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;
    // Clock System Setup
    CSCTL0_H = CSKEY >> 8;                   		      	  // Unlock CS registers
    CSCTL1 = DCORSEL | DCOFSEL_6;             			  // Set DCO to 24MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;          // Set SMCLK = MCLK = DCO
                                              			  // ACLK = VLOCLK
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     			  // Set all dividers to 1
    CSCTL0_H = 0;                            		          // Lock CS registers
}


//#########################################################################
// I2C FUNCTIONS: START
//#########################################################################

void InitI2C()
{
    // Configure GPIO
    P1SEL1 |= BIT6 | BIT7;                  // I2C pins
    P1SEL0 &= ~(BIT6 | BIT7);               // configure I2C pins
    // I2C default uses SMCLK
    UCB0CTL1 |= UCSWRST;                    // put eUSCI_B in reset state
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC; // I2C, master, sync
    UCB0BRW = 0x00F0;                       // baud rate = SMCLK / 240 = 100khz (SMCLK = 24MHz)
    UCB0CTL1 &= ~UCSWRST;                   // eUSCI_B in operational state
    UCB0IFG = 0x00;                         //Clear Interrupt flags
}

void setRTCTime(int hour, int minute, int second)
{
    secCounter = Binary2BCD(second);
    minCounter = Binary2BCD(minute);
    hrCounter  = Binary2BCD(hour);

    RTC_WriteByte(0x00,secCounter);
    RTC_WriteByte(0x01,minCounter);
    RTC_WriteByte(0x02,hrCounter);

}

void setRTCDate(int date, int month, int year)
{
    dtCounter  = Binary2BCD(date);
    monCounter = Binary2BCD(month);
    yrCounter  = Binary2BCD(year);

    RTC_WriteByte(0x04,dtCounter);
    RTC_WriteByte(0x05,monCounter);
    RTC_WriteByte(0x06,yrCounter);
}

void readDateTimeRTC()
{
    int second = 0, minute = 0, hour = 0, day = 0, month = 0, year = 0;

    second = RTC_ReadByte(0x00);
    minute = RTC_ReadByte(0x01);
    hour   = RTC_ReadByte(0x02);
    day    = RTC_ReadByte(0x04);
    month  = RTC_ReadByte(0x05);
    year   = RTC_ReadByte(0x06);


    Time[0] = BCD2UpperCh(hour);
    Time[1] = BCD2LowerCh(hour);
    Time[3] = BCD2UpperCh(minute);
    Time[4] = BCD2LowerCh(minute);
    Time[6] = BCD2UpperCh(second);
    Time[7] = BCD2LowerCh(second);


    Date[0] = BCD2UpperCh(day);
    Date[1] = BCD2LowerCh(day);
    Date[3] = BCD2UpperCh(month);
    Date[4] = BCD2LowerCh(month);
    Date[6] = BCD2UpperCh(year);
    Date[7] = BCD2LowerCh(year);
}

void RTC_WriteByte(int address, int dataWrite)
{
    UCB0I2CSA = RTCAddress;                 // set slave address
    while(UCB0STAT & UCBBUSY);
    I2CWriteInit();
    UCB0CTLW0 |= UCTXSTT;                   // START condition.
    while (UCB0CTLW0 & UCTXSTT);
    UCB0TXBUF = address;
    while(!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = dataWrite;
    while(!(UCB0IFG & UCTXIFG0));
    UCB0CTLW0 |= UCTXSTP;
    while(UCB0CTLW0 & UCTXSTP);             // wait for stop
    UCB0STATW &= ~UCBBUSY;
	
    __delay_cycles(2400);                   //Delay for 100us (24*100)

}

int RTC_ReadByte(int address)
{
    UCB0I2CSA = RTCAddress;                 // set slave address
    int data;
    while(UCB0STAT & UCBBUSY);
    I2CWriteInit();
    UCB0CTLW0 |= UCTXSTT;                   // Transmitter mode and START condition.
    while(UCB0CTLW0 & UCTXSTT);
    UCB0TXBUF = address;
    while(!(UCB0IFG & UCTXIFG0));
    I2CReadInit();
    UCB0CTLW0 |= UCTXSTT;                   // START condition
    while(UCB0CTLW0 & UCTXSTT);             // make sure start has been cleared
    UCB0CTLW0 |= UCTXSTP;                   // STOP condition
    while(!(UCB0IFG & UCRXIFG0));
    data = UCB0RXBUF;
    while(UCB0CTLW0 & UCTXSTP);
	
    __delay_cycles(1200);                   // Delay for 50us(24*50)
	
    return data;
}

int Binary2BCD(int a)
{
   int t1, t2;
   t1 = a%10;
   t1 = t1 & 0x0F;
   a = a/10;
   t2 = a%10;
   t2 = 0x0F & t2;
   t2 = t2 << 4;
   t2 = 0xF0 & t2;
   t1 = t1 | t2;
   return t1;
}  

int BCD2Binary(int a)
{
   int r,t;
   t = a & 0x0F;
   r = t;
   a = 0xF0 & a;
   t = a >> 4;
   t = 0x0F & t;
   r = t*10 + r;
   return r;
} 

char BCD2UpperCh( char bcd)
{
  return ((bcd >> 4) + '0');
}   


char BCD2LowerCh( char bcd)
{
  return ((bcd & 0x0F) + '0');
} 

/*---------------------------------------------------------------------------*/
// Description:
//   Initialization of the I2C Module for Write operation.
/*---------------------------------------------------------------------------*/
void I2CWriteInit(void)
{
    UCB0CTLW0 |= UCTR;                         // UCTR=1 => Transmit Mode (R/W bit = 0)
}

/*----------------------------------------------------------------------------*/
// Description:
//   Initialization of the I2C Module for Read operation.
/*----------------------------------------------------------------------------*/
void I2CReadInit(void)
{
    UCB0CTLW0 &= ~UCTR;                        // UCTR=0 => Receive Mode (R/W bit = 1)
}
//#########################################################################
// I2C FUNCTIONS: END
//#########################################################################
