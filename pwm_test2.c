#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

#ifndef F_CPU
#warning "F_CPU war noch nicht definiert, wird nun mit 3686400 definiert"
#define F_CPU 3686400UL     /* Quarz mit 3.6864 Mhz  */
#endif
#include <util/delay.h>

/*
volatile unsigned int  tmp = 0;
ISR (TIMER1_COMPA_vect) {
	
	++tmp;
	if (tmp >= 1000) {
		tmp = 0;
		PORTD |= (1 << DDD5);
	}
	if (tmp == 500)
		PORTD &= ~(1 << DDD5);
}
*/
int main (void) {
	DDRD = 0xff;
	PORTD = 0;

	PORTD |= 1 << DDD5;

	OCR1A = 1000;

	TCCR1A  =  (1 << WGM10) | (1 << WGM11);
	TCCR1A  =  (1 << COM1A1) |/* (1 << COM1A0) |*/ /*(1 << WGM13) |*/ (1 << WGM12) | (1 << WGM11) | (1 << WGM10);
	TCCR1B  =  (1 << CS10);
	//TIMSK   =  (1 << OCIE1A);
//	TIFR    =  (1 << OCF1A);

	sei();
	while (1) {
	for(OCR1A=20;OCR1A<=300;OCR1A++)
   {
   _delay_ms(1);
   }
	for(OCR1A=300;OCR1A>=21;OCR1A--)
   {
   _delay_ms(1);
   }

	}


	return 0;
}
