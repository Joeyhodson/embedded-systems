// Button interrupt trggers redled to light 3 seconds, no interruptions during
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define BUT1 BIT1 // Button S1 at P1.1

void config_ACLK_to_32KHz_crystal();

int halt = 0;

int main(void) {

    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    // Configure LEDs
    P1DIR |= redLED; // Pins as output
    P1OUT &= ~redLED; // Red off

    // Configuring buttons with interrupt
    P1DIR &= ~BUT1; // 0: input
    P1REN |= BUT1; // 1: enable built-in resistors
    P1OUT |= BUT1; // 1: built-in resistor is pulled up to Vcc
    P1IE |= BUT1; // 1: enable interrupts
    P1IES |= BUT1; // 1: interrupt on falling edge
    P1IFG &= ~BUT1; // 0: clear the interrupt flags

    // Configure Channel 0 for up mode with interrupt
    TA0CCR0 = 49152; // Fill to get 3 second @ 32 KHz
    TA0CCTL0 &= ~CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit
    // Timer_A: ACLK, div by 2, up mode, clear TAR (leaves TAIE=0)
    TA0CTL = TASSEL_1 | ID_1 | MC_1 | TACLR;

    // Configure Aux. clock to 32kHz
    config_ACLK_to_32KHz_crystal();

    for(;;) {
        _low_power_mode_3();
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {

    P1OUT &= ~redLED;
    TA0CCTL0 &= ~CCIE; // enable bit is only set high when the button is pushed (turning LED on)
    // Hardware clears the flag (CCIFG in TA0CCTL0)
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {

    // Detect button 1 (BUT1 in P1IFG is 1)
    if ((P1IFG & BUT1) == BUT1) {
        if ((TA0CCTL0 & CCIE) != CCIE) {
            P1OUT |= redLED;
            // initiate 3 second timer
            TA0CCTL0 |= CCIE;
            TA0CTL |= TACLR;
            //TA0CCTL0 |= CCIFG;
        }
        // Clear BUT1 in P1IFG
        P1IFG &= ~BUT1;
    }
}

void config_ACLK_to_32KHz_crystal() {
    // By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz
    // Reroute pins to LFXIN/LFXOUT functionality
    PJSEL1 &= ~BIT4;
    PJSEL0 |= BIT4;
    // Wait until the oscillator fault flags remain cleared
    CSCTL0 = CSKEY; // Unlock CS registers

    do {
        CSCTL5 &= ~LFXTOFFG; // Local fault flag
        SFRIFG1 &= ~OFIFG; // Global fault flag
    } while((CSCTL5 & LFXTOFFG) != 0);

    CSCTL0_H = 0; // Lock CS registers

    return;
}

