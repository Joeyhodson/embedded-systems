// Code that flashes the red LED with a longer delay
#include <msp430.h>
#include <msp430fr6989.h>
#include <stdint.h>
#define redLED BIT0 // Red LED has value of 0x01.

void main(void) {

    volatile uint32_t i;

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Disable GPIO power-on default high impedance mode

    P1DIR |= redLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off

    for(;;) {

        for(i=0; i<120000; i++) {}

        P1OUT ^= redLED;
    }
}
