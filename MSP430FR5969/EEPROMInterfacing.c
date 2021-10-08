//#########################################################################
// Code to interface MSP430FR5969 with EEPROM 
// Devices : MSP430FR5969
// Author : Shikha Singhal
// Code : Interfacing with EEPROM
//#########################################################################
//  In this code MSP430FR5969 transmits data to 0x50 slave address. In this 
//  case I2C slave is EEPROM.
//
//                                /|\  /|\
//                		  10k  10k (Pull-up resistors)    
//                MSP430FR5969     |    |         EEPROM
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

#define     EEPROMAddress    0x50							// set I2C slave EEPROM address 

void ConfigWDT(void);
void Delay_ms(unsigned int);
void InitI2C(void);
void EEPROM_WriteByte(int, int);
int EEPROM_ReadByte( int);
void I2CWriteInit(void);
void I2CReadInit(void);

int Address, Data, address;
int read_val;

int main(void)
{
	ConfigWDT();									// Configure Watchdog Timer
        configureClocks();                						// Configure Clocks
	InitI2C();									// Initialize I2C 

	Address = 9;									// Address value to write on EEPROM

	EEPROM_WriteByte(Address,'B');							// Write char on EEPROM Address
	Delay_ms(1000);
	read_val = EEPROM_ReadByte(Address);						// Read value from address to verify 
}

void ConfigWDT()
{
    WDTCTL = WDTPW | WDTHOLD;					 // Configure Watchdog Timer
}

void configureClocks()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY >> 8;                   		          // Unlock CS registers
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

/*----------------------------------------------------------------------------*/
// Description:
//   Byte Write Operation. The communication via the I2C bus with an EEPROM
//   is realized. A data byte is written into a user defined address.
/*----------------------------------------------------------------------------*/
void EEPROM_WriteByte(int address, int dataWrite)
{
    UCB0I2CSA = EEPROMAddress;              // Set slave address
    UCB0CTLW0 |= UCMSB;

    unsigned char adr_hi;
    unsigned char adr_lo;
	
    adr_hi = address >> 8;                  // Calculate high byte
    adr_lo = address & 0xFF;                // and low byte of address
	
    while(UCB0STAT & UCBBUSY);
    I2CWriteInit();
    UCB0CTLW0 |= UCTXSTT;                    // START condition.
    while (UCB0CTLW0 & UCTXSTT);
    UCB0TXBUF = adr_hi;
    while(!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = adr_lo;
    while(!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = dataWrite;
    while(!(UCB0IFG & UCTXIFG0));
    UCB0CTLW0 |= UCTXSTP;
    while(UCB0CTLW0 & UCTXSTP);               // wait for stop
    UCB0STATW &= ~UCBBUSY;
	
    __delay_cycles(84000);                    // Delay for 3500us (3500*24)
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
