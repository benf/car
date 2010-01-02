#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#define nop() __asm volatile ("nop")


int main (void) {
	DDRD = 0xff;
	PORTD = 0;

	while (1) {
		PORTD ^= 0xff;
		nop();nop();
	}

	return 0;
}
