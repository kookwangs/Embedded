#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

int state = 0;
int timerCountMS = 0;
int timerCountMin = 0;
int timerCountSec = 0;

ISR (TIMER1_OVF_vect){
    TCNT1 = 65535-7.8125;

    if(state == 2){
        timerCountMS = timerCountMS+1;
        if(timerCountMS == 1000){
            timerCountSec = timerCountSec + 1;
            timerCountMS = 0;
        }
        if (timerCountSec == 60){
            timerCountMin = timerCountMin+1;
            timerCountSec = 0;
        }
    }
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
    
    PORTB |= (1 << PORTB2) | (1 << PORTB3);
    
    initLCD();
    
    // Temp variable for distance
    char tempMS[20] = {};
    char tempMin[20] = {};
    char tempSec[20] = {};
    char tempTSMS[20] = {};
    char tempTSMin[20] = {};
    char tempTSSec[20] = {};
    
    //using Timer 1
    TCCR1A = 0b00000000;
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCNT1 = 65535-7.8125;
    TIMSK1 |= (1 << TOIE1);
  
    sei();
    
    PORTB |= (1 << PORTB2) | (1 << PORTB3);
    while (state == 0) {
        sendLcdCommand(0x01);
        sendLcdCommand(0x80);
    
        LCDDisplayString("    Timer");
        sendLcdCommand(0xC0);
        LCDDisplayString("Press 1: open");
        
        while(PINB & (1 << PINB3)); //wait to press
        _delay_ms(100);
        while(!(PINB & (1 << PINB3))); //wait to release
        
        state = 1;
        sendLcdCommand(0x01);
        sendLcdCommand(0x80);
    
        LCDDisplayString("    00.00.00");
        sendLcdCommand(0xC0);
        LCDDisplayString("Press 1: start");
        
        ////////// Press for start the timer ///////////////
      
  		while(PINB & (1 << PINB3)); //wait to press
        _delay_ms(100);
        while(!(PINB & (1 << PINB3))); //wait to release
        state = 2;
        
      
        while(PINB & (1 << PINB2)){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(timerCountMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
        } 
        
        int timeStampMS = timerCountMS;
        int timeStampSec = timerCountSec;
        int timeStampMin = timerCountMin;
        int timeStampConv = timeStampMS/10;
        
        sprintf(tempTSMS, "%d", timeStampConv);
        sprintf(tempTSSec, "%d", timeStampSec);
        sprintf(tempTSMin, "%d", timeStampMin);
        _delay_ms(100);
        
        
        while(!(PINB & (1 << PINB2))){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// First time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R1: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
 
        } 
        
         while(PINB & (1 << PINB2)){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// First time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R1: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        timeStampMS = timerCountMS;
        timeStampSec = timerCountSec;
        timeStampMin = timerCountMin;
        timeStampConv = timeStampMS/10;
        
        sprintf(tempTSMS, "%d", timeStampConv);
        sprintf(tempTSSec, "%d", timeStampSec);
        sprintf(tempTSMin, "%d", timeStampMin);
        _delay_ms(100);
        
        while(!(PINB & (1 << PINB2))){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Second time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R2: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        
         while(PINB & (1 << PINB2)){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Second time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R2: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        timeStampMS = timerCountMS;
        timeStampSec = timerCountSec;
        timeStampMin = timerCountMin;
        timeStampConv = timeStampMS/10;
        
        sprintf(tempTSMS, "%d", timeStampConv);
        sprintf(tempTSSec, "%d", timeStampSec);
        sprintf(tempTSMin, "%d", timeStampMin);
        _delay_ms(100);
        
        
        while(!(PINB & (1 << PINB2))){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Third time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R3: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        
         while(PINB & (1 << PINB2)){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Third time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R3: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        timeStampMS = timerCountMS;
        timeStampSec = timerCountSec;
        timeStampMin = timerCountMin;
        timeStampConv = timeStampMS/10;
        
        sprintf(tempTSMS, "%d", timeStampConv);
        sprintf(tempTSSec, "%d", timeStampSec);
        sprintf(tempTSMin, "%d", timeStampMin);
        _delay_ms(100);
        
        while(!(PINB & (1 << PINB2))){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Fourth time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R4: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
         while(PINB & (1 << PINB2)){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            //convert MS
            int convMS = timerCountMS/10;//0-999 -> 0-99
            LCDDisplayString("    ");
            sprintf(tempMS, "%d", timerCountMS);
            sprintf(tempSec, "%d", timerCountSec);
            sprintf(tempMin, "%d", convMS);
            if(timerCountMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMin);
            LCDDisplayString(":");
            if(timerCountSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempSec);
            LCDDisplayString(":");
            if(convMS<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempMS);
            
              ////////////////////// Fourth time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R4: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
        timeStampMS = timerCountMS;
        timeStampSec = timerCountSec;
        timeStampMin = timerCountMin;
        timeStampConv = timeStampMS/10;
        
        sprintf(tempTSMS, "%d", timeStampConv);
        sprintf(tempTSSec, "%d", timeStampSec);
        sprintf(tempTSMin, "%d", timeStampMin);
        _delay_ms(100);
        
        while(!(PINB & (1 << PINB2))){
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
           
            LCDDisplayString("    Press 1: restart");
            
            ////////////////////// Fifth time stamp /////////////////////////
            sendLcdCommand(0xC0);
            LCDDisplayString("R5: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        } 
        
            sendLcdCommand(0x01);
            sendLcdCommand(0x80);
            
            LCDDisplayString("    Press 1: restart");
            
              ////////////////////// Fifth time stamp /////////////////////////
             sendLcdCommand(0xC0);
            LCDDisplayString("R5: ");
            if(timeStampMin<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMin);
            LCDDisplayString(":");
            if(timeStampSec<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSSec);
            LCDDisplayString(":");
            if(timeStampConv<=9){
                LCDDisplayString("0");
            }
            LCDDisplayString(tempTSMS);
        
        while(PINB & (1 << PINB3)); //wait to press
        _delay_ms(100);
        while(!(PINB & (1 << PINB3))); //wait to release
  
        state = 0;
        timerCountMS = 0;
        timerCountMin = 0;
        timerCountSec = 0;
    }
}