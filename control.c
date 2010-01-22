// \author Benjamin Franzke
#include <avr/io.h>

#include "control.h"
#include "sensor.h"

void init_control() {

	// set engine pins to output
	DDR_ENGINE |= (1 << ENGINE_LEFT) | (1 << ENGINE_RIGHT)| 
			(1 << ENGINE_ENABLE);
	PORT_ENGINE &= ~(1 << ENGINE_ENABLE);

	// set direction pins to output
	DDR_DIRECTION |= (1 << DIR_LEFT) | (1 << DIR_RIGHT) |
			(1 << DIR_EN);
	DIRECTION &= ~(1 << DIR_EN);

	// PWM configuration
	OCR1A   = 0;
	TCCR1A  =  (1 << COM1A1) | (1 << WGM12) | 
			(1 << WGM11) | (1 << WGM10);
	TCCR1B  =  (1 << CS10);


}

void control_cmd (uint8_t _action, int8_t _param) {
	if (_action == 'S') {
		PORTC ^= (1 << PC1);
		if (_param == 0) {
			OCR1A = 0;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE &= ~(1 << ENGINE_LEFT);
		}	else if (_param > 0) {
			rwd = 1;

			OCR1A = ((uint16_t) _param) << 3;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE |=  (1 << ENGINE_LEFT);
		} else if ((_param < 0) & !hinderniss) {
			rwd = 0;
			OCR1A = ((uint16_t) (- _param)) << 3;

			PORT_ENGINE &= ~(1 << ENGINE_LEFT);
			PORT_ENGINE |=  (1 << ENGINE_RIGHT);
		}
	} else if (_action == 'D') {
		if (_param >= -70 && _param <= 70) {
			DIRECTION &= ~(1 << DIR_LEFT);
			DIRECTION &= ~(1 << DIR_RIGHT);
			DIRECTION &= ~(1 << DIR_EN);

		} else if (_param > 70) {
			DIRECTION &= ~(1 << DIR_RIGHT);
			DIRECTION |=  (1 << DIR_LEFT);
			DIRECTION |=  (1 << DIR_EN);

		} else if (_param < -70) {
			DIRECTION &= ~(1 << DIR_LEFT);
			DIRECTION |=  (1 << DIR_RIGHT);
			DIRECTION |=  (1 << DIR_EN);
		}
	}

}

/* vim: set sts=0 fenc=utf-8: */
