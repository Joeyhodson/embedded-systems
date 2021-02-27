// Board buttons, with interrupt, flashes LEDs
#include <msp430fr6989.h>

#define BUT1 BIT1 // Button S1 at Port 1.1
#define BUT2 BIT2 // Button S2 at Port 1.2
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7

void main(void) {

    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins

    // Configuring LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off
    P9OUT &= ~greenLED; // Turn LED Off

    // Configuring buttons with interrupt
    P1DIR &= ~(BUT1|BUT2); // 0: input
    P1REN |= (BUT1|BUT2); // 1: enable built-in resistors
    P1OUT |= (BUT1|BUT2); // 1: built-in resistor is pulled up to Vcc
    P1IE |= (BUT1|BUT2); // 1: enable interrupts
    P1IES |= (BUT1|BUT2); // 1: interrupt on falling edge
    P1IFG &= ~(BUT1|BUT2); // 0: clear the interrupt flags

    _enable_interrupts();

    for(;;) {}

}

#pragma vector = PORT1_VECTOR // Write the vector name
__interrupt void Port1_ISR() {

    // Detect button 1 (BUT1 in P1IFG is 1)
    if ((P1IFG & BUT1) == BUT1) {
        // Toggle the red LED
        P1OUT ^= redLED;
        // Clear BUT1 ins P1IFG
        P1IFG &= ~BUT1;
    }

    // Detect button 2 (BUT2 in P1IFG is 1)
    if ((P1IFG & BUT2) == BUT2) {
        // Toggle the green LED
        P9OUT ^= greenLED;
        // Clear BUT2 in P1IFG
        P1IFG &= ~BUT2;
    }
}
