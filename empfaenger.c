//#define F_CPU 16000000UL
#define F_CPU  1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "rf12_cfg.h"
#include "rfxx.h"

#include "sensor.h"
#include "control.h"

volatile uint8_t id = 0;

ISR (TIMER0_OVF_vect) {
	cli();

	// this function reads sensors and executed 
	// the appropriate functions 
	sensor_irq();
	sei();
}

/*
 * RF12's FIFO full irq
 *
 * this irq is executed when 8 bit are received 
 * and are ready to be read by us 
 *
 * each package consists of 3 bytes:
 *
 *  ________   _______   _____
 * | action | | param | | crc |
 * 
 * each package is syncronized by a syncrhon pattern
 * so the fifo is reset when one package is received
 */

ISR (INT2_vect) {

	cli();
	PORTC ^= (1 << PC2);

	uint8_t data = rf12_recv();

	PORTC &= ~(1 << PC4);

	// save the received data in the correct variable
	// or execute a function if package is complete
	switch (id) {
		case 0: // first byte = action
			action = data;
			// increment id
			id = 1;
			break;
		case 1: // parameter for action
			param = data;
			// increment id
			id = 2;
			break;
		case 2: // checksum

			// matches checksum and data?
			if (data == _crc_ibutton_update(_crc_ibutton_update(0, action), param)) {
				control_cmd(action, param);
				// inicate that the package was received succesfull
				PORTC |= (1 << PC4);
			}

			// reset id for next package
			id = 0;

			// reset fifo 
			rfxx_wrt_cmd(0xCA81); // reset fifo
			rfxx_wrt_cmd(0xCA83); //  - || -
			break;
	}
	sei();
}

int main(void)
{

	// engine ctrl led and reset indicator
	DDRC  |=  (1 << DDC1);
	PORTC |=  (1 << PC1);


	// inicator for: received package is ok
	DDRC  |=  (1 << PC4);
	PORTC &= ~(1 << PC4);

	/* wait 400ms to give the rf12's POR (Power-On Reset) time to
	 * initialize the registers etc.. (initializing wouldnt work without)
	 *
	 * .. this is NOT documented in the datasheet :\
	 *
	 * notice:
	 *  the producer did the same in the example code but let it uncommented
	 *
	 */
	_delay_ms(400);

	rfxx_init();
	// init rf12 as receiver
	rf12_init(0);

	init_sensor();
	init_control();

	// enable external interrupt 2
	GICR  = (1 << INT2);
	
	// Interrupt PIN is Input
	RFXX_nIRQ_PORT &= ~(1 << RFXX_nIRQ);
	// enable interrupts (global)
	sei();

	// enable receiver's FIFO
	rfxx_wrt_cmd(0xCA83);

	// init finished 
	PORTC &=  ~(1 << PC1);

	while (1);
}

/* vim: set sts=0 fenc=utf-8: */
