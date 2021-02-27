// Simple stopwatch
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

void initialize_LCD();
void config_ACLK_to_32KHz_crystal();
void print_u16int(unsigned int x);

unsigned int counter;
const unsigned char LCD_Num[10] = {0xFC, 0x60, 0xDB, 0xF3,
                                   0x67, 0xB7, 0xBF, 0xE0,
                                   0xFF, 0xF7};

int main(void) {

    volatile unsigned int n;
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    P1DIR |= redLED; // Pins as output
    P9DIR |= greenLED;
    P1OUT |= redLED; // Red on
    P9OUT &= ~greenLED; // Green off

    // Configure Channel 0 for up mode with interrupt
    TA0CCR0 = 32768; // Fill to get 1 second @ 32 KHz
    TA0CCTL0 = CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit
    // Timer_A: ACLK, div by 1, up mode, clear TAR (leaves TAIE=0)
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    // Initializes the LCD_C module
    initialize_LCD();

    // The line below can be used to clear all the segments
    LCDCMEMCTL = LCDCLRM; // Clears all the segments

    // config aux clock to 32kHz
    config_ACLK_to_32KHz_crystal();

    // set counter of stopwatch to 0
    counter = 0;

    // Enable the global interrupt bit (call an intrinsic function)
    _enable_interrupts();

    // Flash the red and green LEDs
    for(;;) {

        if (counter == 65535) {
            counter = 0;
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    // Toggle the LEDs
    P1OUT ^= redLED;
    P9OUT ^= greenLED;

    // print count
    print_u16int(counter++);

    // Hardware clears the flag (CCIFG in TA0CCTL0)
}

// Initializes the LCD_C module
// *** Source: Function obtained from MSP430FR6989’s Sample Code ***
void initialize_LCD() {

    PJSEL0 = BIT4 | BIT5; // For LFXT

    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~LFXTOFF; // Enable LFXT

    do {
        CSCTL5 &= ~LFXTOFFG; // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag

    CSCTL0_H = 0; // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
    LCDCCPCTL = LCDCPCLKSYNC; // Clock synchronization enabled
    LCDCMEMCTL = LCDCLRM; // Clear LCD memory

    //Turn LCD on
    LCDCCTL0 |= LCDON;

    return;
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

void print_u16int(unsigned int number) {

    if (number < 10) {
        LCDM8 = LCD_Num[number];
    }
    else {

        int i = 0;
        int parsed_num[5] = {-1,-1,-1,-1,-1};
        do {
            parsed_num[i++] = number % 10;
            number /= 10;
        } while (number >= 10);
        parsed_num[i] = number;

        i = 0;
        LCDM8 = LCD_Num[parsed_num[i++]];
        LCDM15 = LCD_Num[parsed_num[i++]];
        if (parsed_num[i] >= 0) {
            LCDM19 = LCD_Num[parsed_num[i++]];
        }
        if (parsed_num[i] >= 0) {
            LCDM4 = LCD_Num[parsed_num[i++]];
        }
        if (parsed_num[i] >= 0) {
            LCDM6 = LCD_Num[parsed_num[i]];
        }
    }
}
