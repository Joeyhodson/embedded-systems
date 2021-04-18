// Using Timer_A with 3 channels
// Using ACLK @ 32 KHz (undivided)
// Channel 0 toggles the red LED every 0.1 seconds
// Channel 1 toggles the green LED every 0.5 seconds
// Channel 2 toggles above segments every 4 seconds
#include <msp430fr6989.h>
#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

int halt = 0;

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

void main(void) {

    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;
    P9OUT &= ~greenLED;

    // Configure Channel 0
    TA0CCR0 = 819-1; // @ 8 KHz --> 0.1 seconds
    TA0CCTL0 |= CCIE;
    TA0CCTL0 &= ~CCIFG;

    // Configure Channel 1
    TA0CCR1 = 4096-1; // @ 8 KHz --> 0.5 seconds
    TA0CCTL1 |= CCIE;
    TA0CCTL1 &= ~CCIFG;

    // Configure Channel 2
    TA0CCR2 = 32768 - 1; // @ 8 KHz --> 4.0 seconds
    TA0CCTL2 |= CCIE;
    TA0CCTL2 &= ~CCIFG;

    config_ACLK_to_32KHz_crystal();

    // Configure timer (ACLK) (divide by 1) (continuous mode)
    TA0CTL = TASSEL_1 | ID_2 | MC_2;
    TA0CTL &= ~TAIFG;

    // Engage a low-power mode
    _low_power_mode_3();
    //_enable_interrupts();
    //for (;;) {}
    return;
}

// ISR of Channel 0 (A0 vector)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {

    TA0CCR0 += 819; // Schedule the next interrupt
    if (!(halt)) {
        P1OUT ^= redLED; // Toggle the red LED
    }
    // Hardware clears Channel 0 flag (CCIFG in TA0CCTL0)
}

// ISR of Channel 1 (A1 vector) ... fill the vector name below
#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A1_ISR() {

    if ((TA0CCTL1 & CCIFG) != 0) {
        TA0CCR1 += 4096; // Schedule the next interrupt
        if (!(halt)) {
            P9OUT ^= greenLED; // Toggle the green LED
        }
        TA0CCTL1 &= ~CCIFG; // Clear Channel 1 interrupt flag
        //return;
    }

    if ((TA0CCTL2 & CCIFG) != 0) {
        //P9OUT &= ~greenLED;
        //P1OUT &= ~redLED;
        halt ^= 1;
        TA0CCR2 += 32768; // Schedule the next interrupt
        TA0CCTL2 &= ~CCIFG;
    }
}
