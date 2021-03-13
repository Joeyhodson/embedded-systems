//Program prints a counting unsigned int variable and prints on serial over UART
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer

#define NULL '\0'

unsigned int counter = 0;

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

// Configure UART to the popular configuration
// 9600 baud, 8-bit data, LSB first, no parity bits, 1 stop bit
// no flow control
// Initial clock: SMCLK @ 1.048 MHz with oversampling
void initialize_UART(void){

    // Divert pins to UART functionality
    P3SEL1 &= ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);

    // Use SMCLK clock; leave other settings default
    UCA1CTLW0 |= UCSSEL_2;

    // Configure the clock dividers and modulators
    // UCBR=6, UCBRF=13, UCBRS=0x22, UCOS16=1 (oversampling)
    UCA1BRW = 6;
    UCA1MCTLW = UCBRS5 | UCBRF3 | UCOS16;

    // Exit the reset state (so transmission/reception can begin)
    UCA1CTLW0 &= ~UCSWRST;
}

void uart_write_char(unsigned char ch){

    // Wait for any ongoing transmission to complete
    while ((FLAGS & TXFLAG) == 0) {}

    // Write the byte to the transmit buffer
    TXBUFFER = ch;
}

// The function returns the byte; if none received, returns NULL
unsigned char uart_read_char(void){

    unsigned char temp;

    // Return NULL if no byte received
    if((FLAGS & RXFLAG) == 0)
        return NULL;

    // Otherwise, copy the received byte (clears the flag) and return it
    temp = RXBUFFER;
    return temp;
}

void uart_print_uint16(unsigned int number) {

    if (number < 10) {
        // + 48 converts to decimal ascii vals
        uart_write_char(number + 48);
    }
    else {

        int i = 0;
        int parsed_num_to_ascii[5] = -1;
        do {
            parsed_num_to_ascii[i++] = (number % 10) + 48;
            number /= 10;
        } while (number >= 10);
        parsed_num_to_ascii[i] = number + 48;

        for (i = 5; i >= 0; i--) {
            if (parsed_num_to_ascii[i] >= 0) {
                uart_write_char(parsed_num_to_ascii[i]);
            }
        }
    }
    uart_write_char('\n');
    uart_write_char('\r');
}

int main(void) {

    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;
    P9OUT &= ~greenLED;

    config_ACLK_to_32KHz_crystal();
    // Configure Timer_A
    // ACLK, divide by 1, up mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    // timer period
    TA0CCR0 = 8000; // 0.25 second(s)
    // set up interrupt
    TA0CCTL0 = CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit

    initialize_UART();
    _enable_interrupts();

    while(1) {}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {

    uart_print_uint16(counter++);
    P1OUT ^= redLED; // toggle red LED
    // Hardware clears the flag (CCIFG in TA0CCTL0)
}
