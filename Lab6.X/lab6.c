#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int temp = 0;
//Interrupt Service Routine
ISR (INT0_vect) {
    _delay_ms(50);
    PORTB &= ~(1 << PORTB1);
    temp ^= 1;
    return temp;     
}

int main(void) {
    //output connects to the LED
    DDRB |= (1 << DDB1);
    //input depends on button
    PORTD |= (1 << PORTD2);
    
    //INT0 generates an interrupt
    EICRA |= (1 << ISC01);
    //enable external interrupt INT0
    EIMSK |= (1 << INT0);
    //enable interrupt
    sei();

    while (1) {
        if(temp) {
           PORTB |= (1 << PORTB1);
           _delay_ms(2000);
           PORTB &= ~(1 << PORTB1);
           _delay_ms(2000);  
        }
        else {
           PORTB &= ~(1 << PORTB1);
        }
    }
}
