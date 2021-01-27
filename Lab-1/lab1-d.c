// Code that flashes the red LED at twice the frequency of the green LED
#include <msp430.h>
#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7

void main(void) {

    volatile unsigned int i;
    volatile unsigned int j;

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Disable GPIO power-on default high impedance mode

    P1DIR |= redLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off

    P9DIR |= greenLED; // Direct pin as output
    P9OUT |= greenLED; // Turn LED on
//  P9OUT &= ~greenLED; // Turn LED off

    for(;;) {

        for(i=0; i<2; i++) {
            //P1OUT ^= redLED;
            for(j=0; j<40000; j++) {}
            P1OUT ^= redLED;
        }

        P9OUT ^= greenLED;
    }
}
