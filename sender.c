#include "usart_cfg.h"
#include "usart.h"

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <util/crc16.h>

#include "rf12_cfg.h"
#include "rfxx.h"

uint8_t buffer[20];

/*volatile*/ uint8_t id = 0;
/*volatile*/ uint8_t tmp;


/* Serial Data Input Reception Interrupt (RX/USART)
 *
 * this interrupt will be executed, when one byte of incoming data
 * is received from the pc side (respectively FT232)
 *
 * USART is atmels hardware implementation of protocol 
 * the RS-232 interface also uses
 */
ISR (USART_RXC_vect) {
	cli(); // disable interrupts
	++id;

	tmp = UDR;

	if (tmp == 0xAA)
		id = 0;
	else 
		buffer[id] = tmp;

	if (id == 2) {
		buffer[3] = _crc_ibutton_update(
				_crc_ibutton_update(0, buffer[1]),
				buffer[2]);
		PORTC |= (1 << PC6);
		rf12_send_data(buffer + 1, 3);
		PORTC &= ~(1 << PC6);
		PORTC ^= (1 << PC0);
	}
	PORTC ^= (1 << PC1);
	sei(); // enable interrupts
}

int main(void) {
	// for debugging purposes
	DDRC  =  0xff;

	init_usart();
	sei();
	// wait 200ms for POR initialization 
	// (see empfaenger.c for further information)
	_delay_ms(200);

	rfxx_init();
	// 1 = transfer mode, 0 = receive mode
	rf12_init(1);

	RFXX_nIRQ_DDR &= ~(1 << RFXX_nIRQ);

	while(1);

	return 0;
}

/* vim: set sts=0 fenc=utf-8: */
