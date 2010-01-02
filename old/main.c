#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#ifndef F_CPU
#warning "F_CPU war noch nicht definiert, wird nun mit 3686400 definiert"
#define F_CPU 3686400UL     /* Quarz mit 3.6864 Mhz  */
#endif
#include <util/delay.h>

volatile unsigned int  millisekunden = 0;
volatile unsigned int  sekunde=0;
volatile unsigned int  minute=0;
volatile unsigned int  stunde=0;
ISR (TIMER1_OVF_vect) {
	++millisekunden;
	if (millisekunden >= 100) {
		millisekunden = 0;
		PORTD |= (1 << DDD5);
	}

	if (millisekunden == 50)
		PORTD &= ~(1 << DDD5);
		
	//if (millisekunden & 1 << 1)

}

ISR (TIMER1_COMPA_vect) {
 //++millisekunden;
// if (millisekunden & 1) 
//	 PORTD ^= (1 << DDD5);

}

//ISR (TIMER0_COMP_vect) {
	/*
	if (++millisekunden == 1000) {
		millisekunden = 0;
		if (++sekunde == 60) {
			sekunde = 0;
			if (++minute == 60) {
				stunde++;
				minute = 0;
			}
		}
	}
	*/
	/*
	if (++millisekunden == 3) {
		millisekunden = 0;
		PORTB |= (1 << DDB3);
	}

	if (millisekunden == 1)
		PORTB &= ~(1<< DDB3);
*/
//}

int main (void) {
	DDRB  = 0xff;
//	PORTB = (1 << DDB0) | (1 << DDB1);
	PORTB = 0;

	DDRD = 0xff;
	PORTD = 0;

	PORTB |= 1 << DDD3;
	PORTD |= 1 << DDD5;


	// Timer config
//	TCCR0 = (1 << CS00);
	//OCR0 = 128;
	
	// CMP Int. aktivieren
//	TIMSK |= (1 << OCIE0);

	// http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/
	// Die_Timer_und_Z%C3%A4hler_des_AVR#16-Bit_Timer.2FCounter
	TCCR1A = /*(1 << COM1A0) | */(1 << WGM10) | (1 << WGM11);
	TCCR1B = (1 << CS10);
	TIMSK |= (1 << OCIE1A) | (1 << TOIE1);// | (1 << TICIE1);

	// TIFR seite 86

	TIFR |= (1 << TOV1);

	OCR1A = 400;
//	OCR1A = 128;
//	OCR1H = 0xef;
//	OCR1L = 0xff


	sei();

	while (1);
	/*
	int pwm_phase = 0, pwm_soll = 50;
	while (1) {
		pwm_phase++;
		if (pwm_soll == pwm_phase) {
			PORTB &= ~(1 << DDB2);		// LED+ Widerstand mit PB0 und +5V verbunden
		}
		if (pwm_phase == 100) {
			pwm_phase=0;
			PORTB |= (1 << DDB2);
		}
	}
 */
	/* wird nie erreicht */
	return 0;
}
