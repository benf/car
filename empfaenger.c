#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "rf12_cfg.h"
#include "rfxx.h"

#define PORT_LED     PORTC
#define DDR_LED      DDRC
#define LED_OUTPUT() DDR_LED  |=  (1 << PC1)
#define LED0_ON()    PORT_LED &= ~(1 << PC1)
#define LED0_OFF()   PORT_LED |=  (1 << PC1)
#define LED0_TRG()   PORT_LED ^=  (1 << PC1)


volatile uint8_t id = 0;

volatile uint8_t action;
volatile uint8_t param;

#define DDR_ENGINE    DDRD
#define PORT_ENGINE   PORTD
#define ENGINE_LEFT   PD0
#define ENGINE_RIGHT  PD1
#define ENGINE_ENABLE PD5

#define DDR_DIRECTION DDRA
#define DIRECTION     PORTA
#define DIR_LEFT      PA0
#define DIR_RIGHT     PA1
#define DIR_EN        PA2

void cmd(uint8_t _action, int8_t _param) {

	if (_action == 'S') {
		LED0_TRG();
		if (_param == 0) {
			OCR1A = 0;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE &= ~(1 << ENGINE_LEFT);
		}	else if (_param > 0) {

			OCR1A = ((uint16_t) _param) << 3;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE |=  (1 << ENGINE_LEFT);
		} else if (_param < 0) {
			OCR1A = ((uint16_t) (- _param)) << 3;

			PORT_ENGINE &= ~(1 << ENGINE_LEFT);
			PORT_ENGINE |=  (1 << ENGINE_RIGHT);
		}
	} else if (_action == 'D') {
		if (_param == 0) {
			DIRECTION &= ~(1 << DIR_LEFT);
			DIRECTION &= ~(1 << DIR_RIGHT);
			DIRECTION &= ~(1 << DIR_EN);

		} else if (_param > 50) {
			DIRECTION &= ~(1 << DIR_RIGHT);
			DIRECTION |=  (1 << DIR_LEFT);
			DIRECTION |=  (1 << DIR_EN);

		} else if (_param < -50) {
			DIRECTION &= ~(1 << DIR_LEFT);
			DIRECTION |=  (1 << DIR_RIGHT);
			DIRECTION |=  (1 << DIR_EN);
		}
	}

}


ISR (INT0_vect) {
	cli();

	PORTC ^= (1 << PC2);

	uint8_t data = rf12_recv();

	PORTC &= ~(1 << PC3);
	PORTC &= ~(1 << PC4);

	switch (id) {
		case 0:
			action = data;
			id = 1;
			PORTC |= (1 << PC3);
			break;
		// parameter for command
		case 1:
			param = data;
			//break;
		//case 2: // checksum
			//if (data == _crc_ibutton_update(_crc_ibutton_update(0, action), param))
				cmd(action,param);
			id = 0;
			rfxx_wrt_cmd(0xCA81); // reset fifo
			rfxx_wrt_cmd(0xCA83); //  - || -
			PORTC |= (1 << PC4);

			break;
	}
	sei();
}


int main(void)
{
	_delay_ms(200);
	rfxx_init();
	rf12_init(0);



	RFXX_nIRQ_PORT  &= ~(1 << RFXX_nIRQ);

	// enable external interrupt 0
	//GICR  = (1 << INT0);
	
	//not used
	/////MCUCR = (1 << ISC01) | (1 << ISC00);

	// enable interrupts (global)
	//sei();

	// 
	DDR_ENGINE |= (1 << ENGINE_LEFT) | (1 << ENGINE_RIGHT) | (1 << ENGINE_ENABLE);
	PORT_ENGINE &= ~(1 << ENGINE_ENABLE);

	// 
	DDR_DIRECTION |= (1 << DIR_LEFT) | (1 << DIR_RIGHT) | (1 << DIR_EN);
	DIRECTION &= ~(1 << DIR_EN);

	// Interrupt PIN = Input
	RFXX_nIRQ_PORT &= ~(1 << RFXX_nIRQ);

	// enable receiver's FIFO
	rfxx_wrt_cmd(0xCA83);

	// PWM configuration
	OCR1A   = 0;
	TCCR1A  =  (1 << COM1A1) | (1 << WGM12) | (1 << WGM11) | (1 << WGM10);
	TCCR1B  =  (1 << CS10);

	// things just needed for debugging
	DDRA = 0xff;

	LED_OUTPUT();
	DDRC = 0xff;

	DDRC  |=  (1 << DDC5);
	PORTC |=  (1 << PC5);
  // </things>

	while (1) {
		while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ));

		PORTC ^= (1 << PC2);

		uint8_t data = rf12_recv();

		PORTC &= ~(1 << PC3);
		PORTC &= ~(1 << PC4);

		switch (id) {
			case 0:
				action = data;
				id = 1;
				PORTC |= (1 << PC3);
				break;
			// parameter for command
			case 1:
				param = data;
				//break;
			//case 2:	// checksum
			//	if (data == _crc_ibutton_update(_crc_ibutton_update(0, action), param))
					cmd(action,param);
				id = 0;
				rfxx_wrt_cmd(0xCA81); // reset fifo
				rfxx_wrt_cmd(0xCA83); //  - || -
				PORTC |= (1 << PC4);
				break;
		}
	}
}

/* vim: set sts=0: */
