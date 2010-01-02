#include <avr/io.h>
#include <stdint.h>

int main (void) {
	DDRD = 0xff;
	PORTD = 0;

	PORTD |= 1 << DDD5;

	OCR1A = 512;

	TCCR1A  =  (1 << COM1A1) | (1 << WGM12) | (1 << WGM11) | (1 << WGM10);
	TCCR1B  =  (1 << CS10);

	while (1);
	return 0;
}
