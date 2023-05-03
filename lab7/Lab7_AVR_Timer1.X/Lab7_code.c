#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
int temp = 0;

ISR (TIMER1_OVF_vect){
    TCNT1 = 65535/1.3;
    if(temp == 0){
    PORTB ^= (1 << PORTB0);   
    }
    else if(temp == 1) {
    PORTB &= ~(1 << PORTB0); //clear PORTB0 
    }
}

int main(void) {
    DDRB |= (1 << PORTB0);
    PORTB &= ~(1 << PORTB0); //clear PORTB0
    
    //input depends on button
    PORTD |= (1 << PORTD2);
    
    //using Timer 1
    
    TCCR1A = 0b00000000;
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCNT1 = 65535/1.3;
    TIMSK1 |= (1 << TOIE1);
    
    sei();
    while(1){
        
        while(PIND & (1 << PIND2));
        _delay_ms(30);
        while(!(PIND & (1 << PIND2)));
        _delay_ms(30);
        
        //PORTB &= ~(1 << PORTB0); //clear PORTB0
        temp=1;
    
        while(PIND & (1 << PIND2));
        _delay_ms(30);
        while(!(PIND & (1 << PIND2)));
        _delay_ms(30);
        
        temp =0;
    }
}