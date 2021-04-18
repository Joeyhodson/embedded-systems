// Program cycles through 6 different brightness levels for 1 second each
#include <msp430fr6989.h>
#define PWM_PIN BIT0
#define greenLED BIT7

unsigned int brightness = 0;

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
    PM5CTL0 &= ~LOCKLPM5;

    P9DIR |= greenLED;
    P9OUT |= greenLED;

    // Divert pin to TA0.1 functionality (complete last two lines)
    P1DIR |= PWM_PIN; // P1DIR bit = 1
    P1SEL1 &= ~PWM_PIN; // P1SEL1 bit = 0
    P1SEL0 |= PWM_PIN; // P1SEL0 bit = 1

    // Configure ACLK to the 32 KHz crystal
    config_ACLK_to_32KHz_crystal();

    // Starting the timer in the up mode; period = 0.001 seconds
    // (ACLK @ 32 KHz) (Divide by 1) (Up mode)
    TA0CCR0 = (33-1); // @ 32 KHz --> 0.001 seconds (1000 Hz)
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    // Configure timer to cycle through brightness levels
    TA1CCR0 = 32000;
    TA1CCTL0 = CCIE;
    TA1CCTL0 &= ~CCIFG;
    TA1CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    // Configuring Channel 1 for PWM
    TA0CCTL1 |= OUTMOD_7; // Output pattern: Reset/set
    TA0CCR1 = 1; // Modify this value between 0 and 32 to adjust the brightness level

    _enable_interrupts();
    for(;;) {}

    return;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR() {

    P9OUT ^= greenLED;
    TA0CCR1 = brightness;
    if (brightness == 30) {
        brightness = 0;
    }
    else {
        brightness += 5;
    }
    TA1CCTL0 &= ~CCIFG;
    TA1CTL &= ~TAIFG;

}
