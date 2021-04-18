// Code to print to the LCD pixel display on the Educational BoosterPack

#include "msp430fr6989.h"
#include "Grlib/grlib/grlib.h"          // Graphics library (grlib)
#include "LcdDriver/lcd_driver.h"       // LCD driver
#include <stdio.h>

extern const struct tImage logo4BPP_UNCOMP;

#define redLED BIT0
#define greenLED BIT7
#define button BIT1

int switchScreen = 0;
int screenOneOn = 0;
int screenTwoOn = 0;

// Divert and configure the pins
void HAL_LCD_PortInit(void);

// Configure eUSCI for SPI operation
void HAL_LCD_SpiInit(void);

void screenOne() {

    Graphics_Context g_sContext;        // Declare a graphic library context
    tImage* ucfImage = &logo4BPP_UNCOMP;            // Declare Image struct
    Crystalfontz128x128_Init();         // Initialize the display

    // Set the screen orientation
    Crystalfontz128x128_SetOrientation(0);

    // Initialize the context
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);

    // Set background and foreground colors
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);

    // Set the default font for strings
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

    // Clear the screen
    Graphics_clearDisplay(&g_sContext);
    ////////////////////////////////////////////////////////////////////////////////////////////

    Graphics_drawImage(&g_sContext, ucfImage, 0, 0);

    // Set global screen indication variables accordingly
    screenOneOn = 1;
    screenTwoOn = 0;
}

void screenTwo() {

    char myString[20];
    unsigned int delay = 0;
    uint8_t n = 0;

    Graphics_Context g_sContext;        // Declare a graphic library context
    Graphics_Context circle1;        // Declare a graphic library context for circle 1
    Graphics_Context circle2;        // Declare a graphic library context for circle 2
    Graphics_Context circle3;        // Declare a graphic library context for circle 3
    Crystalfontz128x128_Init();         // Initialize the display

    Graphics_Rectangle rect;
    rect.xMin = 15;
    rect.yMin = 75;
    rect.xMax = 110;
    rect.yMax = 85;

    // Set the screen orientation
    Crystalfontz128x128_SetOrientation(0);

    // Initialize the context
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_initContext(&circle1, &g_sCrystalfontz128x128);
    Graphics_initContext(&circle2, &g_sCrystalfontz128x128);
    Graphics_initContext(&circle3, &g_sCrystalfontz128x128);

    // Set background and foreground colors
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE_SMOKE);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_ROYAL_BLUE);

    Graphics_setForegroundColor(&circle1, GRAPHICS_COLOR_LIME_GREEN);
    Graphics_setBackgroundColor(&circle1, GRAPHICS_COLOR_WHITE_SMOKE);

    Graphics_setForegroundColor(&circle2, GRAPHICS_COLOR_DARK_RED);
    Graphics_setBackgroundColor(&circle2, GRAPHICS_COLOR_WHITE_SMOKE);

    Graphics_setForegroundColor(&circle3, GRAPHICS_COLOR_HOT_PINK);
    Graphics_setBackgroundColor(&circle3, GRAPHICS_COLOR_WHITE_SMOKE);

    // Set the default font for strings
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

    // Clear the screen
    Graphics_clearDisplay(&g_sContext);
    Graphics_clearDisplay(&circle1);
    Graphics_clearDisplay(&circle2);
    Graphics_clearDisplay(&circle3);
    ////////////////////////////////////////////////////////////////////////////////////////////


    Graphics_drawStringCentered(&g_sContext, "EEL 4742", AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);

    Graphics_drawRectangle(&g_sContext, &rect);

    sprintf(myString, "Graphics Demo!");
    Graphics_drawStringCentered(&g_sContext, myString, AUTO_STRING_LENGTH, 64, 55, OPAQUE_TEXT);

    sprintf(myString, "ctr");
    Graphics_drawStringCentered(&g_sContext, myString, AUTO_STRING_LENGTH, 80, 80, OPAQUE_TEXT);

    Graphics_fillCircle(&circle1, 25, 100, 5);
    Graphics_fillCircle(&circle2, 65, 100, 5);
    Graphics_fillCircle(&circle3, 100, 100, 5);

    while (switchScreen) {
        for (delay = 0; delay <= 10000; delay++) {}
        if (n == 255) {
            n = 0;
            sprintf(myString, "   ");
            Graphics_drawStringCentered(&g_sContext, myString, AUTO_STRING_LENGTH, 55, 80, OPAQUE_TEXT);
        }
        sprintf(myString, "%d", n++);
        Graphics_drawStringCentered(&g_sContext, myString, AUTO_STRING_LENGTH, 55, 80, OPAQUE_TEXT);
    }

    // Set global screen indication variables accordingly
    screenTwoOn = 1;
    screenOneOn = 0;
}


void main(void)
{
    volatile unsigned int counter = 0;

    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Disable GPIO power-on default high-impedance mode

    P1DIR |= redLED;    P1OUT &= ~redLED;
    P9DIR |= greenLED;  P9OUT &= ~greenLED;
    P1DIR &= ~button; P1REN |= button; P1OUT |= button; // button, resistor, pull-up

    P1IE |= (button); // 1: enable interrupts
    P1IES |= (button); // 1: interrupt on falling edge
    P1IFG &= ~(button); // 0: clear the interrupt flags

    _enable_interrupts();

    HAL_LCD_SpiInit();
    HAL_LCD_PortInit();

    // Configure SMCLK to 8 MHz (used as SPI clock)
    CSCTL0 = CSKEY;                 // Unlock CS registers
    CSCTL3 &= ~(BIT4|BIT5|BIT6);    // DIVS=0
    CSCTL0_H = 0;                   // Relock the CS registers

    while(1){
        if (switchScreen) {
            if (!screenTwoOn) {
                screenTwo();
            }
        }
        else {
            if (!screenOneOn) {
                screenOne();
            }
        }
    }

}

#pragma vector = PORT1_VECTOR // Write the vector name
__interrupt void Port1_ISR() {

    // Detect button 1 (button in P1IFG is 1)
    if ((P1IFG & button) == button) {

        // Toggle the red LED
        P1OUT ^= redLED;

        // Toggle between screens
        switchScreen ^= 1;

        // Clear button ins P1IFG
        P1IFG &= ~button;
    }
}


