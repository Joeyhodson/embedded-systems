// Sample code that prints any unsigned 16-bit int on LCD
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

void Initialize_LCD();
void print_u16int(unsigned int x);

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

    // Initializes the LCD_C module
    Initialize_LCD();

    // The line below can be used to clear all the segments
    LCDCMEMCTL = LCDCLRM; // Clears all the segments

    // Display 430 on the rightmost three digits
    print_u16int(1000);
    //LCDM8 = LCD_Num[0];
    //LCDM15 = LCD_Num[3];
    //LCDM19 = LCD_Num[4];

    // Flash the red and green LEDs
    for(;;) {
        for(n = 0; n <= 50000; n++) {} // Delay loop
        P1OUT ^= redLED;
        P9OUT ^= greenLED;
    }
}

// Initializes the LCD_C module
// *** Source: Function obtained from MSP430FR6989’s Sample Code ***
void Initialize_LCD() {

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
