//Microchip PIC18F25K22 - I2C - MASTER WRITE TO SLAVE EXAMPLE

//next line sets oscillator to internal and turns off PLL.
#pragma config FOSC = INTIO67, FCMEN = OFF, IESO = OFF, PLLCFG = OFF //CONFIG1H
#pragma config PWRTEN = OFF, BOREN = OFF, BORV = 250                 //CONFIG2L
#pragma config WDTEN = OFF, WDTPS = 32768                            //CONFIG2H
#pragma config MCLRE = EXTMCLR, PBADEN = OFF, HFOFST = OFF	     //CONFIG3H
#pragma config STVREN = ON, LVP = OFF, XINST = OFF                   //CONFIG4L
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF            //CONFIG5L
#pragma config CPB = OFF, CPD = OFF                                  //CONFIG5H
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF        //CONFIG6L
#pragma config WRTB = OFF, WRTC = OFF, WRTD = OFF                    //CONFIG6H
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF    //CONFIG7L
#pragma config EBTRB = OFF                                           //CONFIG7H

#include <xc.h>
#include "plib/delays.h"
#include "plib/i2c.h"

/* Note: PIC18F25K22 uses I2C_V6 (see pconfig.h). It helps to know this
when you are studying the I2C source code in /sources/pic18/plib/i2c */

void main(void) {
    unsigned char slave7bitAddr = 0x2A; //7-bit address of Slave.  MSB=Don't care.
    unsigned char slaveAddrWrite = (slave7bitAddr << 1); //LSB=0, Master Write request.
    signed char writeStat;
    unsigned char message[11] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};

    OSCCONbits.IRCF = 0b111; //Set internal oscillator to 16mHz

    //Must set SCL(pin RC3)and SDA(pin RC4) as inputs and enable digital buffers.
    TRISCbits.TRISC3 = 1; //set input
    TRISCbits.TRISC4 = 1;
    ANSELCbits.ANSC3 = 0; //enable digital buffer
    ANSELCbits.ANSC4 = 0;

    OpenI2C1(MASTER, SLEW_OFF); //If you switch to 400kHz (see below) set SLEW_ON
    /* You can ignore all I2C "unable to resolve identifier" errors assuming
    you didn't make any typos.  Don't forget the "1"'s. */

    /* Now set the I2C clock speed.  I2C Master always controls clock.
    For 400kHz use 1k pullup resistors and for 100kHz use 2.2k ohms */

    SSP1ADD = 0x27; //100Khz = FOSC/(4 * (SSPADD + 1)) = 16E6/((39 + 1) * 4)note:39=0x27
    //SSP1ADD = 0x09; //400Khz = FOSC/(4 * (SSPADD + 1)) = 16E6/((9 + 1) * 4)

    while (1) {
        IdleI2C1(); //Wait for bus to become idle.
        StartI2C1(); //Begin I2C communication
        IdleI2C1();

        //send slave address (w/write request bit) and wait for slave to reply.
        do {
            writeStat = WriteI2C1(slaveAddrWrite); //Send address with LSB=Write
            if (writeStat == -1) { //Detected bus collision - More than one master?
                unsigned char data = SSP1BUF; //clear the buffer by reading it.
                SSPCON1bits.WCOL = 0; //clear the bus collision status bit
            } else if (writeStat == -2) { //NACK (no acknowledge rx'd)
                //Is the slave on and ready?  Did we send the correct address?
            }
        } while (writeStat != 0); //Keep repeating until slave acknowledges.

        //Slave has Ack'd so we can send our Hello World message now.
        for (int x = 0; x <= 10; x++) {
            do {
                writeStat = (WriteI2C1(message[x]));
                if (writeStat == -2) { //NACK (no acknowledge rx'd)
                    //Is the slave on and ready?  Using the correct pullups?
                }
            } while (writeStat != 0); //Keep repeating until slave acknowledges.
        }

        IdleI2C1();
        StopI2C1();

        //Delay about 1 sec and then repeat.  1sec = ((10K*200*2)/(16E6/4)
        Delay10KTCYx(200);
        Delay10KTCYx(200);
    }
}
