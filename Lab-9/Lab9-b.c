//Program reads lux values using I2C from light sensor and prints over UART
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7

#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer

#define NULL '\0'

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

void uart_write_string(char* string) {

    int i = 0;

    while (string[i] != '\0') {
        uart_write_char(string[i++]);
    }
    uart_write_char('\n');
    uart_write_char('\r');
}

// Configure eUSCI in I2C master mode
void initialize_I2C(void) {

    // Enter reset state before the configuration starts...
    UCB1CTLW0 |= UCSWRST;

    // Divert pins to I2C functionality
    P4SEL1 |= (BIT1|BIT0);
    P4SEL0 &= ~(BIT1|BIT0);

    // Keep all the default values except the fields below...
    // (UCMode 3:I2C) (Master Mode) (UCSSEL 1:ACLK, 2,3:SMCLK)
    UCB1CTLW0 |= UCMODE_3 | UCMST | UCSSEL_3;

    // Clock divider = 8 (SMCLK @ 1.048 MHz / 8 = 131 KHz)
    UCB1BRW = 8;

    // Exit the reset mode
    UCB1CTLW0 &= ~UCSWRST;
}

// Read a word (2 bytes) from I2C (address, register)
int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int * data) {

    unsigned char byte1, byte2;

    // Initialize the bytes to make sure data is received every time
    byte1 = 111;
    byte2 = 111;

    //********** Write Frame #1 ***************************
    UCB1I2CSA = i2c_address; // Set I2C address
    UCB1IFG &= ~UCTXIFG0;
    UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
    UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal
    while ((UCB1IFG & UCTXIFG0) ==0) {}
    UCB1TXBUF = i2c_reg; // Byte = register address
    while((UCB1CTLW0 & UCTXSTT)!=0) {}
    if(( UCB1IFG & UCNACKIFG )!=0) return -1;
    UCB1CTLW0 &= ~UCTR; // Master reads (R/W bit = Read)
    UCB1CTLW0 |= UCTXSTT; // Initiate a repeated Start Signal

    //****************************************************
    //********** Read Frame #1 ***************************
    while ( (UCB1IFG & UCRXIFG0) == 0) {}
    byte1 = UCB1RXBUF;

    //****************************************************
    //********** Read Frame #2 ***************************
    while((UCB1CTLW0 & UCTXSTT)!=0) {}
    UCB1CTLW0 |= UCTXSTP; // Setup the Stop Signal
    while ( (UCB1IFG & UCRXIFG0) == 0) {}
    byte2 = UCB1RXBUF;
    while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

    //****************************************************
    // Merge the two received bytes
    *data = ( (byte1 << 8) | (byte2 & 0xFF) );

    return 0;
}


// Write a word (2 bytes) to I2C (address, register)
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int data) {

    unsigned char byte1, byte2;
    byte1 = (data >> 8) & 0xFF; // MSByte
    byte2 = data & 0xFF; // LSByte
    UCB1I2CSA = i2c_address; // Set I2C address
    UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
    UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal
    while ((UCB1IFG & UCTXIFG0) ==0) {}
    UCB1TXBUF = i2c_reg; // Byte = register address
    while((UCB1CTLW0 & UCTXSTT)!=0) {}

    //********** Write Byte #1 ***************************
    UCB1TXBUF = byte1;
    while ( (UCB1IFG & UCTXIFG0) == 0) {}

    //********** Write Byte #2 ***************************
    UCB1TXBUF = byte2;
    while ( (UCB1IFG & UCTXIFG0) == 0) {}
    UCB1CTLW0 |= UCTXSTP;
    while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

    return 0;
}

void uart_write_uint16(unsigned int number) {

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
}

unsigned int pow(unsigned int number, unsigned int power) {

    unsigned int i = 0;
    unsigned int final_val = 1;

    for (i = 0; i < power; i++) {
        final_val*= number;
    }

    return final_val;
}

void dec_to_hex(unsigned int* number) {

    unsigned int curr_digit;
    unsigned int hex_number = 0;
    unsigned int tens_power = 0;

    while (*number > 9) {
       curr_digit = *number%16;
       *number /= 16;
       hex_number += curr_digit*(pow(10, tens_power++));
    }
    hex_number += *number*(pow(10, tens_power++));

    *number = hex_number;
    return;
}

int main(void) {

    unsigned int data;
    unsigned int x;
    unsigned int counter = 0;

    unsigned char i2c_config_reg = 0x01; // found in sbos681b (sensor data sheet)
    unsigned char i2c_result_reg = 0x00; // found in sbos681b (sensor data sheet)
    unsigned char i2c_address = 0x44; // found in slau599a (BoosterPack user guide)

    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    initialize_UART();
    initialize_I2C();

    data = 0x7604;
    i2c_write_word(i2c_address, i2c_config_reg, data);

    while(1) {

        uart_write_uint16(counter++);
        uart_write_char(' ');

        i2c_read_word(i2c_address, i2c_result_reg, &data);
        uart_write_uint16(data*1.28);

        uart_write_char('\n');
        uart_write_char('\r');

        for(x = 0; x < 60000; x++) {}
    }
}
