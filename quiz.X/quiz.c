#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

int state = 0;

ISR (TIMER1_OVF_vect){
    TCNT1 = 65535-7812;



}

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

void USART_Init(unsigned int ubrr) { 
    // Set baud rate (The number of bit that send in 1 second )
    UBRR0 = ubrr;
    // Enable receiver & transmitter
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    // Set frame format (8 data, 2 stop bits)
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
}

void USART_Transmit(unsigned char data ) { 
    // Wait for empty transmit buffer
    while ( !( UCSR0A & (1 << UDRE0)) );
    // Put data into buffer, sends data (Auto)
    UDR0 = data; 
}



int main(void) {
    
    PORTD |= (1 << PORTD6) | (1 << PORTD7);
    
    initLCD();
    
    // Temp variable for distance
    char distanceString[20] = {};
    sendLcdCommand(0x01);
    sendLcdCommand(0x80);
    
    LCDDisplayString("    Distance");
    sendLcdCommand(0xC0);
    LCDDisplayString("2.3");
    LCDDisplayString("x");
    
    //using Timer 1
    TCCR1A = 0b00000000;
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCNT1 = 65535-7.8125;
    TIMSK1 |= (1 << TOIE1);
  
    sei();
    
    while (1) {
        
        while(PIND & (1 << PIND6)); //wait to press
        _delay_ms(100);
        while(!(PIND & (1 << PIND6))); //wait to release
        state = 1;
        
   
        LCDDisplayString("00.00.00");
      
  		
     
             
        
       
        
      
    }
}