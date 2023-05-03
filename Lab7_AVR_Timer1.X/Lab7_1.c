#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

ISR (TIMER1_COMPA_vect){
    PORTB ^= (1 << PORTB0);
}

int main(void) {
    DDRB |= (1 << PORTB0);
    PORTB &= ~(1 << PORTB0); //clear PORTB0
    
    //using Timer 1
    
    TCCR1A = 0b00000000;
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
    OCR1A  = 15624;
    TIMSK1 |= (1 << OCIE1A);
   
    sei();
    while(1){
    
    }
}


