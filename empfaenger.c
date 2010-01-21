//#define F_CPU 16000000UL
#define F_CPU  1000000UL

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

volatile uint8_t hinderniss;
volatile uint8_t rwd;
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


void init_special()
{
	DDRA &= ~((1 << PA3) & (1 << PA4));

//  TCCR0 |= (1 << CS00 ) | ( 1 << CS02 );
	TCCR0 |= (1 << CS02);
	TIMSK |= (1 << TOIE0 );
  
	sei();

}
uint16_t ReadADC(uint8_t channel)
{
  uint8_t i;
  uint16_t result;
 
  ADMUX = channel;                      
  ADMUX |= (1<<REFS1) | (1<<REFS0); // Vref = 2.56V
  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);    // Frequenzteiler 32
  ADCSRA |= (1<<ADSC);
  while ( ADCSRA & (1<<ADSC) ) {}
  result = ADCW;  
  result = 0; 
  //Eigentliche Messung beginnt jetzt
  for ( i=0; i<4; ++i )
  {
    ADCSRA |= (1<<ADSC);            // single conversion
    while ( ADCSRA & (1<<ADSC) ) {}
    result += ADCW;	
  }
  ADCSRA &= ~(1<<ADEN);
 
  result /= 4; //Mittelwert und zurück
  return result;
}


ISR (TIMER0_OVF_vect)
{
	cli();
  //Overflow
  //ADC LESEN
  uint16_t adcvalue;
  uint8_t  erg;

  adcvalue = ReadADC(3); //Kanal 3 lesen, SHARP Sensor
  erg = adcvalue/4;  //Durch 4 teilen => Spannung als 8-Bit Wert mal 100 (ohne Komma von 0-255)
  /*   	
		IR-Sensor
		---------

		adcwert * (Uref/1024) = Vout 
		Abstand (cm) y = 22/(Vout-0.13)
		Bremsen bei y < 50cm

		LDR
		---
		
		Hell	-	1K Ohm
		Dunkel	-	500K Ohm
  
  */
  if((erg >= 50))  //ca. 30cm
  {
	  hinderniss = 1;
    PORTC |=  (1 << PC3);
	if (!rwd)
		cmd('S', 0);
  }else
  {
	  hinderniss = 0;
    PORTC &= ~(1 << PC3);
  }
      
  adcvalue = ReadADC(4); //Kanal 4 lesen, LDR
  erg = adcvalue/4; 
  if((erg >= 130))
  {
    PORTC |= (1 << PC0);
  }else
  {
    PORTC &= ~(1 << PC0);
  }
  PORTC ^= (1 << PC1);
  sei();
}


void cmd(uint8_t _action, int8_t _param) {
	if (_action == 'S') {
		LED0_TRG();
		if (_param == 0) {
			OCR1A = 0;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE &= ~(1 << ENGINE_LEFT);
		}	else if (_param > 0) {
			rwd = 1;

			OCR1A = ((uint16_t) _param) << 3;

			PORT_ENGINE &= ~(1 << ENGINE_RIGHT);
			PORT_ENGINE |=  (1 << ENGINE_LEFT);
		} else if (_param < 0 & !hinderniss) {
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

/*! TODO: interupt: Wofür die dieser interupt?  */
#if 0
ISR (INT2_vect) {

	//cli();
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
	//sei();
}
#endif

int main(void)
{
	/* wait 300ms to give the rf12's POR (Power-On Reset) time to
	 * initialize the registers etc.. (initializing wouldnt work without)
	 *
	 * .. this is NOT documented in the datasheet :\
	 *
	 * notice:
	 *  the producer did the same in the example code but let it uncommented
	 *
	 * TODO: try avoid waiting 300ms by using the interrupt generated by POR
	 *       disadvantage: IRQ will lengthen (IRQ on INT0) 
	 */
	DDRC = 0xff;
//	DDRC  |= (1 << PC0);
//	PORTC |= (1 << PC0);

	DDRC  |=  (1 << DDC1);
	PORTC |=  (1 << PC1);

	_delay_ms(400);

	rfxx_init();
	rf12_init(0);

	// things just needed for debugging
	//DDRA = 0xff;
	init_special();


	// enable external interrupt 1
	//// MCUCSR &= ~(1 << ISC2);
	// GICR  = (1 << INT2);
	
	//not used
	/////MCUCR = (1 << ISC01) | (1 << ISC00);

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


	LED_OUTPUT();
	DDRC = 0xff;

	DDRC  |=  (1 << DDC5);
	PORTC |=  (1 << PC5);

	// enable interrupts (global)
	//	sei();

	PORTC &=  ~(1 << PC1);

//	cmd('S',0x81);
#if 1
	while (1) {
		while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ));

		PORTC ^= (1 << PC2);

		uint8_t data = rf12_recv();

//		PORTC &= ~(1 << PC3);
		PORTC &= ~(1 << PC4);

		switch (id) {
			case 0:
				action = data;
				id = 1;
//				PORTC |= (1 << PC3);
				break;

			case 1: // parameter for command
				param = data;
				id = 2;
				break;

			case 2:	// checksum
				if (data == _crc_ibutton_update(_crc_ibutton_update(0, action), param))
					cmd(action,param);
				id = 0;
				rfxx_wrt_cmd(0xCA81); // reset fifo
				rfxx_wrt_cmd(0xCA83); //  - || -
				PORTC |= (1 << PC4);
				break;
		}
	}
#endif
}

/* vim: set sts=0 fenc=utf-8: */
