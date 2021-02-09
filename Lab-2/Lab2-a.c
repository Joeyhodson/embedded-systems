#include <msp430.h>
#include <msp430fr6989.h>

// Turning on the red LED while button S1 is pushed

#define redLED BIT0 // Red Led at P1.0
#define greenLED BIT7 // Green Led at P9.7
#define BUT1 BIT1 // Button S1 at P 1.1


void main(void) {

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins

    // Configure and initialize LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off
    P9OUT &= ~greenLED; // Turn LED Off

    // Configure buttons
    P1DIR &= ~BUT1; // Direct pin as input
    P1REN |= BUT1; // Enable built-in resistor
    P1OUT |= BUT1;  // Set resistor as pull-up

    // Polling the button in an infinite loop
    for(;;) {

        if (!(P1IN & BUT1)) //... button S1 is pushed ... // since active-low
            P1OUT |= redLED; // Turn red LED on
        else P1OUT &= ~redLED; // Turn red LED off
    }
}
