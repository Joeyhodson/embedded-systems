#include <msp430.h>
#include <msp430fr6989.h>

// Both LEDs toggle when their respective button is being held
// However, if both buttons are pressed neither will toggle (visible to the eye)

#define redLED BIT0 // Red Led at P1.0
#define greenLED BIT7 // Green Led at P9.7
#define BUT1 BIT1 // Button S1 at P1.1
#define BUT2 BIT2 // Button S2 at P1.2


void main(void) {

    volatile unsigned int i;

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins

    // Configure and initialize LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off
    P9OUT &= ~greenLED; // Turn LED Off

    // Configure buttons
    P1DIR &= ~BUT1; // Direct pin as input
    P1DIR &= ~BUT2; // Direct pin as input
    P1REN |= BUT1; // Enable built-in resistor
    P1REN |= BUT2; // Enable built-in resistor
    P1OUT |= BUT1;  // Set resistor as pull-up
    P1OUT |= BUT2;  // Set resistor as pull-up


    // Polling the button in an infinite loop
    for(;;) {

        while (!(P1IN & BUT1)) {
            for (i = 0; i < 10000; i++) {}
            P1OUT ^= redLED;
            if (!(P1IN & BUT2)) {
                break;
            }
        }
        P1OUT &= ~redLED;


        while (!(P1IN & BUT2)) {
            for (i = 0; i < 10000; i++) {}
            P9OUT ^= greenLED;
            if (!(P1IN & BUT1)) {
                break;
            }
        }
        P9OUT &= ~greenLED;

    }

}
