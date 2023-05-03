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

void TIMER1_Init() {
	// Set up the timer
    // ICNC1 = 1 : To cancel the input noise
    // CS11 = 1 : (CLK i/o) / 8
    TCCR1B |= (1 << ICNC1) | (1 << CS11);
}

int main(void) {
    
    
    // Intcialize timer & LCD
    TIMER1_Init();
    initLCD();
    
    // Temp variable for distance
    char distanceString[20] = {};
   
    // Set trigger port in the sensor to be 1
    // Trigger connects to port PD7
    DDRD |= (1 << DDD7);
    // Clear the output port
    PORTD &= ~(1 << PORTD7);
    
    while (1) {
        
        // Clear LCD display 
  		sendLcdCommand(0x01);
        
        // Part 1: Start
        // Let the trigger = 1
        PORTD |= (1 << PORTD7);
        _delay_us(10);
        // Let the trigger = 0
        PORTD &= ~(1 << PORTD7);
        
        
        // Part 2: Capture the rising edge
        // ICES1 = 1 : capture rising edge
        TCCR1B |= (1 << ICES1);
        
        // Check ICF1 for capture event
        while(!(TIFR1 & (1 << ICF1)));

        // Set TCNT1 = 0
        TCNT1 = 0;
        
        // Reset ICF1 flag
        TIFR1 |= (1 << ICF1);
        
        
        // Part 3: Capture the falling edge
        // ICES1 = 0 : capture falling edge
        TCCR1B &= ~(1 << ICES1);
        
        // Check ICF1 for capture event
        while(!(TIFR1 & (1 << ICF1)));

        // Copy 16-bit data out from ICR1 ()
        uint16_t sensorOutput = ICR1;

        // Reset ICF1 flag
        TIFR1 |= (1 << ICF1);
        
        
        // Part 4: Calculate the distance and display to LCD
        // Since velocity of the sound is 340 m/s
        //       and the value we get from the wave go and come back
        //       so we divide by 2
        uint8_t distance = ((sensorOutput / 1000000.0) * 340 * 100) / 2.0;
        
        // Convert the distance we calculate to be string 
        //  and give them to variable distanceString
  	  	sprintf(distanceString, "%d", distance);
        
        LCDDisplayString("Distance: ");
        LCDDisplayString(distanceString);
        sendLcdCommand(0xC5);
        LCDDisplayString("centimetres");
        
        _delay_ms(1000);
    }
}