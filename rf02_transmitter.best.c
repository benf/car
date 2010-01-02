#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define RFXX_nIRQ  PA0
#define RFXX_FSK   PA1

#define PORT_SPI PORTB
#define DDR_SPI  DDRB
#define SPI_SS    PB4
#define SPI_MOSI  PB5
#define SPI_MISO  PB6
#define SPI_SCK   PB7

uint16_t RFXX_WRT_CMD(uint16_t aCmd){
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
}

void RF02B_SEND(uint8_t data){
   uint8_t i;
   for(i = 0; i < sizeof(uint8_t); ++i) {
      while (  PINB & (1 << RFXX_nIRQ)); // Polling nIRQ
      while (!(PINB & (1 << RFXX_nIRQ)));

			if (data & 0x80)
        PORTB |= (1 << RFXX_FSK);
      else
        PORTB &= ~(1 << RFXX_FSK);
	    data <<= 1;
  }
}

int main(void) {
  uint16_t i, ChkSum;

	DDR_SPI = (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);

	
  RFXX_WRT_CMD(0xCC00);
  RFXX_WRT_CMD(0x8B81); // 433BAND,+/-60kHz
  RFXX_WRT_CMD(0xA640); // 434MHz
  RFXX_WRT_CMD(0xC847); // 4.8kbps
  RFXX_WRT_CMD(0xC220); // ENABLE BIT SYNC
  RFXX_WRT_CMD(0xC001); // CLOSE ALL

	PORTB |= (1 << RFXX_FSK) | ~(1 << RFXX_nIRQ);
	DDRB  |= (1 << RFXX_FSK) | ~(1 << RFXX_nIRQ);

	DDRC  =  0xff;

  while(1) {
		
		PORTC |= (1 << PC0);

    ChkSum = 0;

    RFXX_WRT_CMD(0xC039); // START TX
		PORTC ^= (1 << PC1);
    RF02B_SEND(0xAA); // PREAMBLE
    RF02B_SEND(0xAA); // PREAMBLE
    RF02B_SEND(0xAA); // PREAMBLE

		RF02B_SEND(0x2D);//HEAD HI BYTE
    RF02B_SEND(0xD4);//HEAD LOW BYTE
	
		for (i = 0x30; i <= 0x3F; ++i) {
			RF02B_SEND(i);
			ChkSum += i;
		}

     RF02B_SEND(ChkSum);   // DATA16
     RF02B_SEND(0xAA);     // DUMMY BYTE
     RFXX_WRT_CMD(0xC001); // CLOSE TX

		 PORTC &= ~(1 << PC0);

		 for (i = 0; i < 200; ++i)
			  _delay_ms(10);


  };
}

