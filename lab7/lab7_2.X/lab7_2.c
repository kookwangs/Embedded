#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
int temp = 0;
int checkPress = 0;
int checkPress_2 = 0;
int stop = 0;
int stop_2 = 0;

ISR (TIMER1_OVF_vect){
    TCNT1 = 65535-7812;
    
    if(temp == 0 && stop==0){ 
        PORTC &= ~(1 << PORTC2);
        PORTC &= ~(1 << PORTC1);
        PORTC |= (1 << PORTC0);
        temp = 1;
        if(checkPress!=0 && checkPress <5){
            checkPress += 1; //should be 2
        }
    }
    
    else if(temp == 1 && stop==0){
        PORTC &= ~(1 << PORTC0);
        PORTC &= ~(1 << PORTC2);
        PORTC |= (1 << PORTC1); 
        temp = 2;
        if(checkPress!=0 && checkPress <5){
            checkPress += 1; //should be 3
        }
    }
    
    else if(temp == 2 && stop==0){
        PORTC &= ~(1 << PORTC1);
        PORTC &= ~(1 << PORTC0);
        PORTC |= (1 << PORTC2); 
        temp = 3;
        if(checkPress!=0 && checkPress <5){
            checkPress += 1; //should be 4
        }
    }
    
    else if(temp == 3 && stop==0){
        PORTC &= ~(1 << PORTC2); 
        PORTC &= ~(1 << PORTC0);
        PORTC |= (1 << PORTC1); 
        temp = 0;
        if(checkPress!=0 && checkPress <5){
            checkPress += 1; 
        }
    }
    
    if(checkPress==4){
            PORTC &= ~(1 << PORTC0); 
            PORTC &= ~(1 << PORTC1); 
            PORTC &= ~(1 << PORTC2); 
            temp=-1;
        }
    
     if(checkPress_2 == 1 && stop==1){ 
          checkPress_2 += 1; //should be 2
        }
     else if(checkPress_2 == 2 && stop==1){ 
          checkPress_2 += 1; //should be 2
        }
     else if(checkPress_2 == 3 && stop==1){ 
          checkPress_2 += 1; //should be 2
        }
     
    
     if(checkPress_2==4){
            PORTC &= ~(1 << PORTC0); 
            PORTC &= ~(1 << PORTC1); 
            PORTC &= ~(1 << PORTC2); 
            temp=-1;
            stop_2 = 1;
        }
    
    }
    

int main(void) {
    DDRC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2);
    PORTC &= ~(1 << PORTC0) & ~(1 << PORTC1) & ~(1 << PORTC2); //clear PORTB0
    
    //input depends on button
    PORTD |= (1 << PORTD2);
    
    //using Timer 1
    TCCR1A = 0b00000000;
    TCCR1B |= (1 << CS12) | (1 << CS10);
    TCNT1 = 65535-7812;
    TIMSK1 |= (1 << TOIE1);
    
    sei();
    while(1){
        
        while(PIND & (1 << PIND2)); //wait to press
        //pressing
        _delay_ms(30);
        checkPress = 1;
        
        while(!(PIND & (1 << PIND2)));
        
        _delay_ms(30);
        stop=1;
   
        
        while(PIND & (1 << PIND2)); //wait to press
        _delay_ms(30);
        checkPress_2 = 1;
        while(!(PIND & (1 << PIND2))); //wait to release
        stop = 0;
        checkPress_2 = 0;
        _delay_ms(30);
        
        
        if(stop_2 == 1 && temp ==-1){
            while(PIND & (1 << PIND2)); //wait to press
            _delay_ms(30);
            while(!(PIND & (1 << PIND2))); //wait to release
            _delay_ms(30);
            temp = 0;
            stop_2=0;
        }
        else if(temp==-1){
            temp=0;
        }
        
        checkPress=0;    
    }
}