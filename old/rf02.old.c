#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
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

/*

uint16_t RFXX_WRT_CMD(uint16_t aCmd){
#if SOFT_SPI
	uint8_t  i;
	uint16_t temp = 0;

	PORT_SPI &= ~(1 << SPI_SCK);
	PORT_SPI &= ~(1 << SPI_SS);
	
	for (i = 0; i < 16; i++) {
		temp <<= 1;
		if (PINB & (1 << RFXX_nIRQ))
			temp |= 0x0001;

		PORT_SPI &= ~(1 << SPI_SCK);

		if (aCmd & (1 << 15))
			PORT_SPI |=  (1 << SPI_MOSI);
		else
			PORT_SPI &= ~(1 << SPI_MOSI);

		PORT_SPI |=  (1 << SPI_SCK);
    
		aCmd <<= 1;
  }

	PORT_SPI &= ~(1 << SPI_SCK);
	PORT_SPI |=  (1 << SPI_SS);
	return temp;
#else
	uint8_t hi  = (aCmd >> 8) & 0xff;
	uint8_t low = aCmd & 0xff;

	// chip select (SS low active)
	PORT_SPI &= ~(1 << SPI_SS);

	SPDR = hi;
	while ((SPSR & (1 << SPIF)) == 0);
	SPDR = low;
	while ((SPSR & (1 << SPIF)) == 0);

	// disable chip select
	PORT_SPI |= (1 << SPI_SS);
	return 0;
#endif
}

void RF02B_SEND(uint8_t data) {
   uint8_t i;
   for (i = 0; i < 8; ++i) {
      while (  PINB & (1 << RFXX_nIRQ)); // Polling nIRQ
      while (!(PINB & (1 << RFXX_nIRQ)));

			if (data & (1 << 7))
        PORTB |=  (1 << RFXX_FSK);
      else
        PORTB &= ~(1 << RFXX_FSK);
	    data <<= 1;
  }
}
*/
/*
void rfx_init(void) {
	_delay_ms(100);
#if SOFT_SPI
	// disable chip select (low active)
	PORT_SPI |=  (1 << SPI_SS);
	PORT_SPI |=  (1 << SPI_MOSI);
	PORT_SPI &= ~(1 << SPI_SCK);

//	PORTB    |=  (1 << RFXX_FSK);
#else
	SPCR      = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
#endif

	DDR_SPI  |=  (1 << SPI_SS);
	DDR_SPI  |=  (1 << SPI_MOSI);
	DDR_SPI  |=  (1 << SPI_SCK); 

}
*/
/*
void rf02_init(void) {
	RFXX_WRT_CMD(0xCC00);
	RFXX_WRT_CMD(0x8B81); // 433BAND,+/-60kHz
	RFXX_WRT_CMD(0xA640); // 434MHz
	RFXX_WRT_CMD(0xC847); // 4.8kbps
	RFXX_WRT_CMD(0xC220); // ENABLE BIT SYNC
	RFXX_WRT_CMD(0xC001); // CLOSE ALL

	PORTB = (1 << RFXX_FSK);
}
*/
int main(void) {
	uint8_t i, ChkSum;

	// for debugging purposes
	DDRC  =  0xff;

	rfxx_init();
	rf02_init();

	DDRB &= ~(1 << RFXX_nIRQ);
	DDRB |=  (1 << RFXX_FSK);


	//	rf02_send_data((uint8_t*)"hallo, was geht", 15);

  ChkSum = 0;
	uint8_t data[17];
	for (i = 0; i < 16; ++i) {
		data[i] = 0x30 + i;
		ChkSum += data[i];
	}
	data[16] = ChkSum;

	while (1) {
		PORTC |= (1 << PC0);
		rf02_send_data(data, 17);
		PORTC &= ~(1 << PC0);

		for (i = 0; i < 200; ++i)
		  _delay_ms(10);
	}
}

