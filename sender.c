#define F_CPU 4000000UL

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <util/crc16.h>

#include "rf12_cfg.h"

#include "rfxx.h"


#include "usart.h"


uint8_t buffer[4];

volatile uint8_t id = 0;
volatile uint8_t tmp;

volatile uint8_t queue = 0;

/*! interupt */
ISR (USART_RXC_vect) {
	
	//PORTD ^= 0xff;
	++id;
	tmp = UDR;

	if (tmp == '_')
		id = 0;
	buffer[id] = tmp;

	if (id == 2) {
		rf12_send_data(buffer+1, 2);
	}


	PORTC ^= (1 << PC1);

	// loopback
//	tmp = UDR;
//	rf02_send_data(&tmp, 1);
}

int main(void) {
	uint8_t i, ChkSum;

	sei();

	// for debugging purposes
	DDRC  =  0xff;

	init_usart();
	_delay_ms(200);
	rfxx_init();
	rf12_init(1);

	DDRD &= ~(1 << RFXX_nIRQ);

	while (1);
	return 0;

	ChkSum = 0;
	uint8_t data[17];
	for (i = 0; i < 16; ++i) {
		data[i] = 0x30 + i;
		ChkSum += data[i];
	}
	data[16] = ChkSum;

	while (1) {	

		PORTC |=  (1 << PC0);
		rf12_send_data(data, 17);
		PORTC &= ~(1 << PC0);
		for (i = 0; i < 20; ++i)
			_delay_ms(100);
	}
		/*
		tmp = id_saved;
		if (tmp > id_send) { 
			rf02_send_data(MEINBUFFER + id_send, tmp-id_send);
			id_send = tmp;

		  // integer overflow occured
		} else if (tmp < id_send) {
			rf02_send_data(MEINBUFFER + id_send, 256-id_send);
			id_send = 0;
		}
	}
*/
	//	rf02_send_data((uint8_t*)"hallo, was geht", 15);


	while (1);
	/*{
		PORTC |= (1 << PC0);
		rf02_send_data(data, 17);
		PORTC &= ~(1 << PC0);

		for (i = 0; i < 200; ++i)
		  _delay_ms(10);
	}
	*/
	return 0;
}

/* vim: set sts=0: */
