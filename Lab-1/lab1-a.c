// Code that flashes the red LED
#include <msp430.h>
#include <msp430fr6989.h>
#define redLED BIT0 // Red LED as A first bit to a port or reg.

void main(void) {

    volatile unsigned int i;

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Disable GPIO power-on default high impedance mode

    P1DIR |= redLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off

    for(;;) {

        for(i=0; i<20000; i++) {}

        P1OUT ^= redLED;
    }
}
