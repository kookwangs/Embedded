#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>

void initADC()
{
  //Set reference voltage to AVcc
  //Set to use AVC1
    ADMUX |= (1 << REFS0)|(1 << MUX0);
  //Enable ADC with prescalar 128
    ADCSRA |= (1 << ADEN)|(1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void initLED()
{   //Set LED to be output to the port PB1
    DDRB |= (1 << DDB1);         
    PORTB &= ~(1 << PORTB1);
}

void initTimer()
{
    //TCCR - Timer Counter Control Register
    //It's a register for controlling the timer
    //WGM10 = 1 : PWM, phase correct, 8-bit
    //COM1A0 = 1 && COM1A1 = 1: set when compare match
    TCCR1A |= (1 << COM1A0)| (1 << COM1A1) | (1 << WGM10);
    //Select clock
    TCCR1B |= (1 << CS10); 
}

//Function to set the duty cycle from the ADC value (get from potentiometer)
void _setDutyCycle(uint16_t adcValue)
{
    uint16_t dutyCycle = 0;
    if (adcValue < 26) 
    {
        //Set duty cycle to be 10% of 255
        dutyCycle = 25.5;
    } 
    else if (adcValue < 64) 
    {
        //Set duty cycle to be 25% of 255
        dutyCycle = 63.75;
    } 
    else if (adcValue < 128) 
    {
        //Set duty cycle to be 50% of 255
        dutyCycle = 127.5; 
    } 
    else if (adcValue < 192) 
    {
        //Set duty cycle to be 75% of 255
        dutyCycle = 191.25;
    } 
    else {
        //Set duty cycle to be 100% of 255
        dutyCycle = 255;
    }
    //Set duty cycle
    OCR1A = dutyCycle; 
}

int main(void) {
    initADC();
    initLED();
    initTimer();
    while (1) {
        //Start conversion
        ADCSRA |= (1 << ADSC); 
        //Wait for ADIF
        while (ADCSRA & (1 << ADSC)); 
        //
        uint16_t adcValue = ADC;
        _setDutyCycle(adcValue);
    }
    return 0;
}