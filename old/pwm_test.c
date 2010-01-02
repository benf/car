#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

#ifndef F_CPU
#warning "F_CPU war noch nicht definiert, wird nun mit 3686400 definiert"
#define F_CPU 3686400UL     /* Quarz mit 3.6864 Mhz  */
#endif

volatile unsigned int  tmp = 0;
ISR (TIMER1_OVF_vect) {
	++tmp;
	if (tmp >= 50) {
		tmp = 0;
		PORTD |= (1 << DDD5);
	}
	if (tmp == 6)
		PORTD &= ~(1 << DDD5);
}

int main (void) {
	DDRD = 0xff;
	PORTD = 0;

	PORTD |= 1 << DDD5;


	TCCR1A  =  (1 << WGM10)  | (1 << WGM11);
	TCCR1B  =  (1 << CS10);
	TIMSK   =  /*(1 << OCIE1A) |*/ (1 << TOIE1);

	TIFR    =  (1 << TOV1);

	sei();
	while (1);
	return 0;
}
