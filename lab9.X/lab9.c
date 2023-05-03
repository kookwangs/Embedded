#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

void commitData()
{
    //Let E = 1 (for updating data to LCD)
    //E of LCD which connects to port PD6
    PORTD |= (1 << PORTD6);
	_delay_us(10);
	PORTD &= ~(1 << PORTD6);
	_delay_us(10);
}

void sendLcdCommand(uint8_t command)
{
    _delay_us(100);
    // Let RS = 0 
    // RS of LCD which connects to port PD5
	PORTD &= ~(1 << PORTD5);

	// Put high nibble (4 bits) of the command
    // (4 bit in the left hand)
	PORTD &= 0xF0; //clear last 4 bit to receive command
	PORTD |= command >> 4; //shift 4 bits in the left to right
	commitData(); //commit 4 bits first (left)
    
    _delay_us(100);
	// Put low nibble (4 bits) of the command
    // (4 bit in the right hand)
	PORTD &= 0xF0; //clear last 4 bit to receive command
	PORTD |= (command & 0x0F); //clear 4 bits in the left, remain only 4 bit right
	commitData(); //commit last 4 bits (right)
}

void sendLCDData(uint8_t command)
{   
    _delay_us(100);
    // Let RS = 1
    // RS of LCD which connects to port PD5
	PORTD |= (1 << PORTD5);

	// Put high nibble (4 bits) of the data
    // (4 bit in the left hand)
	PORTD &= 0xF0; //clear last 4 bit to receive data
	PORTD |= command >> 4; //shift 4 bits in the left to right
	commitData(); //commit 4 bits first (left)
    
    _delay_us(100);
	// Put low nibble (4 bits) of the data
    // (4 bit in the right hand)
	PORTD &= 0xF0; //clear last 4 bit to receive data
	PORTD |= (command & 0x0F); //clear 4 bits in the left, remain only 4 bit right
	
    commitData(); //commit last 4 bits (right)
}
void LCDDisplayString(const char* str)
{
  while(*str != '\0') // Display until it's the last string
  {
    sendLCDData(*str);
    str++;
  }
}
void initLCD()
{
    // Set the output to the LCD
    // D4 D5 D6 D7 which connect to port PD0 PD1 PD2 PD3
	DDRD |= 0x0F;
    // Above command equals to DDRD |= (1 << DDD0) | (1 << DDD1)| (1 << DDD2) | (1 << DDD3);   
	// Let all output we set up above to be 0 before we start
	PORTD &= 0xF0;

	// Set the outputs to the LCD which are E, RS
    // E connects to PD6, RS connects to PD5
	DDRD |= (1 << DDD5) | (1 << DDD6);
	// Let all output we set up above to be 0 before we start
	PORTD &= ~(1 << DDD5) & ~(1 << DDD6);

    // Below commands: set up to communicate in 4-bit mode
	sendLcdCommand(0x33);
	sendLcdCommand(0x32);
	sendLcdCommand(0x28);
	sendLcdCommand(0x0E);
}


int main(void) {
    

    initLCD();

    // Set SS (PB2) and SCK (PB5)
    DDRB |= (1<<DDB2) | (1 <<DDB5);
    // Set SS (CS) = 1 (Default)
    PORTB |= (1<<PORTB2);
    
    // Set SPE = 1: SPI enable
    // MSTR = 1: Set AVR to be master (controller)
    // Since max clk that it can accept 1.6 Mhz
    // but the CPU clk is 8 Mhz
    // so we have to devide with 8 
    // by set SPR0 = 1, SPI2X = 1 : Set prescaler (fosc/16)
    SPCR|= (1 << SPE)|(1 << MSTR)|(1 << SPR0);
    SPSR |= (1 << SPI2X) ;

    sendLcdCommand(0x01);
    sendLcdCommand(0x80);
    LCDDisplayString("    Temperature");
   
   
    char buffer[16];
    
    
while(1){
    // Before transmit data, set SS (CS) = 0
    PORTB &= ~(1<<PORTB2);

    // write the data we need in SPDR then it will send automatically
    // then, read a value from the other end
    
    //SPIF: tell that it's already send successfully
    
    // try to send any values in SPDR
    SPDR = 0;
    
    // wait until SPIF == 1 (send successfully)
    while (! (SPSR & (1 << SPIF)));
    // read high byte value from SPDR
    uint8_t highByte = SPDR;
    
    // clear flag
    SPSR |= (1 << SPIF);

    // try to send any values in SPDR
    SPDR = 0;
    // wait until SPIF == 1 (send successfully)
    while (!(SPSR & (1 << SPIF)));
    // read low byte value from SPDR
    uint8_t lowByte = SPDR;
    
    // clear flag
    SPSR |= (1 << SPIF);
    
 
    
    // clear first 3 bit in high byte (cause it's waste and may be noise)
    // shift all to right because the last bit is repeated of the first bit (also waste)
    int adcValue = (((highByte & 0x1F)<<8) | lowByte) >> 1;
    
    float adcValue2 = adcValue;
    adcValue2 = adcValue2 * 5 / 4096;
    int xx = (adcValue2 - 0.5)/0.01;


    sendLcdCommand(0xC0);
    sprintf(buffer, "%u", xx);
    LCDDisplayString(buffer);
    _delay_ms(1000);

}
}