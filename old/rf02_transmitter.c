#define F_CPU 4000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define BAUD 9600UL
#include <util/setbaud.h>

#define SOFT_SPI   0


#define RFXX_nIRQ  PB0
#define RFXX_FSK   PB1

#define PORT_SPI   PORTB
#define DDR_SPI    DDRB
#define SPI_SS     PB4
#define SPI_MOSI   PB5
#define SPI_MISO   PB6
#define SPI_SCK    PB7

#include "rfxx.h"
#include "usart.h"

uint8_t MEINBUFFER[256];
uint8_t id_saved = 0;
uint8_t id_send = 0;

ISR (USART_RXC_vect) {
  	cli();
	//PORTD ^= 0xff;


	MEINBUFFER[id_saved++] = UDR;
	PORTC ^= (1 << PC1);

	// loopback
//	tmp = UDR;
//	rf02_send_data(&tmp, 1);
	//while ((UCSRA & (1 << UDRE)) == 0);
	// Send it back
	//UDR = tmp;	
	sei();
}

volatile uint8_t to_transmit;
ISR (INT0_vect) {
	cli();
	PORTC ^= (1 << PC0);
	
	usart_transmit(PINA);
	sei();
}

void rf02_init(void) {
	RFXX_WRT_CMD(0xCC00);
	RFXX_WRT_CMD(0x8B81); // 433BAND,+/-60kHz
	RFXX_WRT_CMD(0xA640); // 434MHz
	RFXX_WRT_CMD(0xC847); // 4.8kbps
	RFXX_WRT_CMD(0xC220); // ENABLE BIT SYNC
	RFXX_WRT_CMD(0xC001); // CLOSE ALL

	PORTB = (1 << RFXX_FSK);
}

int main(void) {
	//	uint8_t i, ChkSum;

	DDRA  =  0;
	GICR   = (1 << INT0);
	MCUCR  = (1 << ISC01) | (1 << ISC00);

	sei();
	// for debugging purposes
	DDRC  =  0xff;

	init_usart();
	rfxx_init();
	rf02_init();

	DDRB &= ~(1 << RFXX_nIRQ);
	DDRB |=  (1 << RFXX_FSK);

	uint8_t tmp;
	
	while (1) {
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

	//	rf02_send_data((uint8_t*)"hallo, was geht", 15);

	/*
    ChkSum = 0;
	uint8_t data[17];
	for (i = 0; i < 16; ++i) {
		data[i] = 0x30 + i;
		ChkSum += data[i];
	}
	data[16] = ChkSum;
*/
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

