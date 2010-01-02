#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#define SOFT_SPI   0

#define RFXX_nIRQ  PD2
#define RFXX_FSK   PB1

#define PORT_SPI   PORTB
#define DDR_SPI    DDRB
#define SPI_SS     PB4
#define SPI_MOSI   PB5
#define SPI_MISO   PB6
#define SPI_SCK    PB7

#include "rfxx.h"

#define PORT_LED     PORTC
#define DDR_LED      DDRC
#define LED_OUTPUT() DDR_LED  |=0x0F
#define LED0_ON()    PORT_LED &=~(1<<0)
#define LED0_OFF()   PORT_LED |= (1<<0)
#define LED0_TRG()   PORT_LED ^= (1<<0)

uint8_t RF_RXBUF[22];

volatile uint8_t num = 0;
ISR (INT0_vect) {
 	cli();
	DDRC  |= (1 << DDD2);
	PORTC |= (1 << PD2);

	uint8_t data = RF01_RDFIFO();
	rfxx_wrt_cmd(0xCE84);   // reset FIFO for next frame recognition
	rfxx_wrt_cmd(0xCE87);

	DDRC  |= (1 << DDC1);
	PORTC ^= (1 << PC1);

	DDRA  = 0xff;	
	PORTA = data;
	//send interrupt
	PORTC &= ~(1 << PC5);
	PORTC |=  (1 << PC5);
	sei();
	
	/*
	uint8_t ChkSum, j;
	RF_RXBUF[num++] = data;
	if (num == 17) {
		num = 0;
		rfxx_wrt_cmd(0xCE84);   // reset FIFO for next frame recognition
		rfxx_wrt_cmd(0xCE87);
		ChkSum = 0;

		for (j = 0; j < 16; ++j)
			ChkSum += RF_RXBUF[j]; // calculate checksum

		if (ChkSum == RF_RXBUF[16]) // frame check
			LED0_TRG(); // receive indication
	}
	*/
}

void rf01_init(void) {
	rfxx_wrt_cmd(0x0000);
	rfxx_wrt_cmd(0x898A); //433BAND,134kHz
	rfxx_wrt_cmd(0xA640); //434MHz
	rfxx_wrt_cmd(0xC847); //4.8kbps
	rfxx_wrt_cmd(0xC69B); //AFC setting
	rfxx_wrt_cmd(0xC42A); //Clock recovery manual control,Digital filter,DQD=4
	rfxx_wrt_cmd(0xC240); //output 1.66MHz
	rfxx_wrt_cmd(0xC080);
	rfxx_wrt_cmd(0xCE84); //use FIFO
	rfxx_wrt_cmd(0xCE87);
	rfxx_wrt_cmd(0xC081); //OPEN RX
}

int main(void)
{
	rfxx_init();
	rf01_init();

	LED_OUTPUT();
	DDRC = 0xff;

	GICR  = (1 << INT0);
//	MCUCR = (1 << ISC01) | (1 << ISC00);
	sei();

	DDRC  |=  (1 << DDC5);
	PORTC |=  (1 << PC5);

	while (1);
	return 0;
}

