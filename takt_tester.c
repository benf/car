#define F_CPU 4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#define nop() __asm volatile ("nop")


int main (void) {
	DDRD = 0xff;
	PORTD = 0;

	while (1) {
		PORTD ^= 0xff;
		_delay_ms(1000);
	}

	return 0;
}
