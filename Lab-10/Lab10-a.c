//Program reads analog input from joy-stick, converts and displays with ADC and UART respectively (just x-axis)
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer

void initialize_UART(void) {

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

void initialize_ADC() {

    // Divert the pins to analog functionality
    // X-axis: A10/P9.2, for A10 (P9DIR=x, P9SEL1=1, P9SEL0=1)
    P9SEL1 |= BIT2;
    P9SEL0 |= BIT2;

    // Turn on the ADC module
    ADC12CTL0 |= ADC12ON;

    // Turn off ENC (Enable Conversion) bit while modifying the configuration
    ADC12CTL0 &= ~ADC12ENC;

    //*************** ADC12CTL0 ***************
    // Set ADC12SHT0 (select the number of cycles that you determined)
    ADC12CTL0 &= ~(ADC12SHT01 | ADC12SHT02| ADC12SHT03);
    ADC12CTL0 |= ADC12SHT00; // 4 cycles (when dividing clock by 8) -> (low power consumption, slow conversion time)

    //*************** ADC12CTL1 ***************
    // Set ADC12SHS (select ADC12SC bit as the trigger)
    // Set ADC12SHP bit
    // Set ADC12DIV (select the divider you determined)
    // Set ADC12SSEL (select MODOSC)
    ADC12CTL1 &= ~(ADC12SHS0 | ADC12SHS1 | ADC12SHS2);
    ADC12CTL1 |= ADC12SHP;
    ADC12CTL1 |= (ADC12DIV0 | ADC12DIV1 | ADC12DIV2); // Divide frequency by 8
    ADC12CTL1 &= ~(ADC12SSEL0 | ADC12SSEL1);

    //*************** ADC12CTL2 ***************
    // Set ADC12RES (select 12-bit resolution)
    // Set ADC12DF (select unsigned binary format)
    ADC12CTL2 &= ~ADC12RES0;
    ADC12CTL2 |= ADC12RES1;
    ADC12CTL2 &= ~ADC12DF;

    //*************** ADC12CTL3 ***************
    // Leave defaults

    //*************** ADC12MCTL0 ***************
    // Set ADC12VRSEL (select VR+=AVCC, VR-=AVSS)
    // Set ADC12INCH (select channel A10)
    ADC12MCTL0 &= ~(ADC12VRSEL0 | ADC12VRSEL1 | ADC12VRSEL2 | ADC12VRSEL3);
    ADC12MCTL0 |= ADC12INCH_10;

    // Turn on ENC (Enable Conversion) bit at the end of the configuration
    ADC12CTL0 |= ADC12ENC;

    return;
}

void uart_write_char(unsigned char ch){

    // Wait for any ongoing transmission to complete
    while ((FLAGS & TXFLAG) == 0) {}

    // Write the byte to the transmit buffer
    TXBUFFER = ch;
}

void uart_write_12_bit_int(unsigned int number) {

    if (number < 10) {
        // + 48 converts to decimal ascii vals
        uart_write_char(number + 48);
    }
    else {

        int i = 0;
        int parsed_num_to_ascii[4] = -1;
        do {
            parsed_num_to_ascii[i++] = (number % 10) + 48;
            number /= 10;
        } while (number > 0);

        for (i = 3; i >= 0; i--) {
            if (parsed_num_to_ascii[i] >= 0) {
                uart_write_char(parsed_num_to_ascii[i]);
            }
        }
    }
}

int main(void) {

    unsigned int i = 0;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

	P1DIR |= redLED;
	P9DIR |= greenLED;
	P1OUT &= ~redLED;
	P9OUT &= ~greenLED;

	initialize_UART();
	initialize_ADC();

	while(1) {

	    ADC12CTL0 |= ADC12SC; // Starts ADC conversion

	    while(ADC12CTL1 & ADC12BUSY) {} // Waits until conversion is complete
	    uart_write_12_bit_int(ADC12MEM0);
	    uart_write_char('\n');
	    uart_write_char('\r');

	    P1OUT ^= redLED;
	    for(i = 0; i < 50000; i++) {}
	}
	
}
