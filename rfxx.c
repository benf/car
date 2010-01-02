#include <avr/io.h>

#include "rf12_cfg.h"
#include "rfxx.h"

void _delay_ms(double ms);


uint16_t RFXX_WRT_CMD(uint16_t aCmd){
uint16_t temp = 0;
#if SOFT_SPI
	uint8_t  i;

	PORT_SPI &= ~(1 << SPI_SCK);
	PORT_SPI &= ~(1 << SPI_SS);
	
	for (i = 0; i < 16; ++i) {
		if (aCmd & (1 << 15))
			PORT_SPI |=  (1 << SPI_MOSI);
		else
			PORT_SPI &= ~(1 << SPI_MOSI);

		PORT_SPI |=  (1 << SPI_SCK);
	  
		temp <<= 1;
		if (PIN_SPI & (1 << SPI_MISO))
			temp |= 0x0001;

		PORT_SPI &= ~(1 << SPI_SCK);
		aCmd <<= 1;
	}
	PORT_SPI |= (1 << SPI_SS);
	return temp;
#else
	uint8_t hi  = (aCmd >> 8) & 0xff;
	uint8_t low = aCmd & 0xff;

	// chip select (SS low active)
	PORT_SPI &= ~(1 << SPI_SS);
	uint8_t tmp;
	SPDR = hi;
	while ((SPSR & (1 << SPIF)) == 0) {
	}
//	tmp = SPDR;
//	temp = tmp << 8;

	
	temp <<= 8;

	SPDR = low;
	while ((SPSR & (1 << SPIF)) == 0) {
	}
	tmp = SPDR;

	temp = tmp;

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

#ifdef RFXX_nIRQ
#ifdef RFXX_nIRQ_PIN

void RF12_SEND(uint8_t data) {
	while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ)); // wait for previous TX
	RFXX_WRT_CMD(0xb800 | data);
}

#endif 
#endif

void rfxx_init(void) {
	//_delay_ms(200);

	DDR_SPI  |=  (1 << SPI_SS);
	DDR_SPI  |=  (1 << SPI_MOSI);
	DDR_SPI  &= ~(1 << SPI_MISO);
	DDR_SPI  |=  (1 << SPI_SCK); 

	// disable chip select (low active)
	PORT_SPI |= (1 << SPI_SS);


	DDRD = ~(1 << RFXX_nIRQ);

#if SOFT_SPI
	PORT_SPI |=  (1 << SPI_MOSI);
	PORT_SPI &= ~(1 << SPI_SCK);
#else
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
#endif

}

uint8_t RF01_RDFIFO(void) {
	uint8_t data;
	uint8_t i;
#if SOFT_SPI

	PORT_SPI &= ~(1 << SPI_SCK);
	PORT_SPI &= ~(1 << SPI_MOSI);
	PORT_SPI &= ~(1 << SPI_SS);

	for (i = 0; i < 16; ++i) { // skip status bits
		PORT_SPI |=  (1 << SPI_SCK);
		PORT_SPI |=  (1 << SPI_SCK);
		PORT_SPI &= ~(1 << SPI_SCK);
		PORT_SPI &= ~(1 << SPI_SCK);
	}

	data = 0;
	for (i = 0; i < 8; ++i) { //read fifo data byte
		data <<= 1;
		if (PORT_SPI & (1 << SPI_MISO))
			data |= 0x01;

		PORT_SPI |=  (1 << SPI_SCK);
		PORT_SPI |=  (1 << SPI_SCK);
		PORT_SPI &= ~(1 << SPI_SCK);
		PORT_SPI &= ~(1 << SPI_SCK);
	}

	PORT_SPI |= (1 << SPI_SS);
#else
	// chip select (SS low active)
	PORT_SPI &= ~(1 << SPI_SS);
	
	uint8_t tmp;
	
	// read two bytes (status bytes)
	for (i = 0; i < 2; ++i) {
	  SPDR = 0x00;
	  while ((SPSR & (1 << SPIF)) == 0);
	  tmp = SPDR;
	}

	SPDR = 0x00;
	while ((SPSR & (1<<SPIF)) == 0);
	data = SPDR;

	// disable chip select
	PORT_SPI |= (1 << SPI_SS);
#endif
	return data;
}

uint8_t rf12_recv(void) {

	uint16_t data;
	// while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ));

	RFXX_WRT_CMD(0x0000);
	data = RFXX_WRT_CMD(0xb000);
	
	return (uint8_t) 0x00ff & data;
}
void rf12_recv_data(uint8_t *data, uint8_t num) {
	uint8_t i;
	RFXX_WRT_CMD(0xCA81);
	//Enable FIFO
	RFXX_WRT_CMD(0xCA83);
	for (i = 0; i < num; ++i)
		*data++ = rf12_recv();
}

void rf12_send_data(uint8_t *data, uint8_t num) {
	uint8_t i;
	RFXX_WRT_CMD(0x0000); // WHY THAT? bsp says read status  - but why?
	RFXX_WRT_CMD(0x8239); // enable TX, PLL, synthesizer, crystal

	// preamble
	for (i = 0; i < 3; ++i)
		RF12_SEND(0xAA);

	// syncron pattern
	RF12_SEND(0x2D);
	RF12_SEND(0xD4);

	// DATA
	for (i = 0; i < num; ++i)
		RF12_SEND(data[i]);

	// preamble / dummy byte
	for (i = 0; i < 3; ++i)
		RF12_SEND(0xAA);

	RFXX_WRT_CMD(0x8201);
}

void rf02_send_data(uint8_t *data, uint8_t num) {
	RFXX_WRT_CMD(0xC039); // START TX
 	RF02B_SEND(0xAA); // PREAMBLE
 	RF02B_SEND(0xAA); // PREAMBLE
 	RF02B_SEND(0xAA); // PREAMBLE

	RF02B_SEND(0x2D);//HEAD HI BYTE
	  RF02B_SEND(0xD4);//HEAD LOW BYTE

	uint8_t i;
	for (i = 0; i < num; ++i)
		RF02B_SEND(data[i]);
	
	RF02B_SEND(0xAA);     // DUMMY BYTE
	//RF02B_SEND(0xAA);     // DUMMY BYTE
	//RF02B_SEND(0xAA);     // DUMMY BYTE
	RFXX_WRT_CMD(0xC001); // CLOSE TX
}

void rf12_init_send(void) {
	RFXX_WRT_CMD(0x80D8);//EL,EF,433band,12.5pF
	RFXX_WRT_CMD(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
	RFXX_WRT_CMD(0xA640);//A140=430.8MHz
	RFXX_WRT_CMD(0xC647);//19.2kbps
	RFXX_WRT_CMD(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	RFXX_WRT_CMD(0xC2AC);//AL,!ml,DIG,DQD4
	RFXX_WRT_CMD(0xCA81);//FIFO8,SYNC,!ff,DR
	RFXX_WRT_CMD(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
	RFXX_WRT_CMD(0x9850);//!mp,9810=30kHz,MAX OUT
	RFXX_WRT_CMD(0xE000);//NOT USE
	RFXX_WRT_CMD(0xC800);//NOT USE
	RFXX_WRT_CMD(0xC400);//1.66MHz,2.2V
}

void rf12_init(uint8_t transfer) {
	RFXX_WRT_CMD(0x80D8);//EL,EF,433band,12.5pF

	RFXX_WRT_CMD(0x8209 | (transfer ? 0x0030 : 0x00D0));
 // RFXX_WRT_CMD(0x82D9);//ER,EBB,!et,ES,EX,!eb,!ew,DC

	RFXX_WRT_CMD(0xA640);//434MHz
	RFXX_WRT_CMD(0xC647);//4.8kbps
	RFXX_WRT_CMD(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	RFXX_WRT_CMD(0xC2AC);//AL,!ml,DIG,DQD4
	RFXX_WRT_CMD(0xCA81);//FIFO8,SYNC,!ff,DR{
	RFXX_WRT_CMD(0x80D8);//EL,EF,433band,12.5pF

	RFXX_WRT_CMD(0x8209 | (transfer ? 0x0030 : 0x00D0));
	// RFXX_WRT_CMD(0x82D9);//ER,EBB,!et,ES,EX,!eb,!ew,DC

	RFXX_WRT_CMD(0xA640);//434MHz
	RFXX_WRT_CMD(0xC647);//4.8kbps
	RFXX_WRT_CMD(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	RFXX_WRT_CMD(0xC2AC);//AL,!ml,DIG,DQD4
	RFXX_WRT_CMD(0xCA81);//FIFO8,SYNC,!ff,DR
	RFXX_WRT_CMD(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
	RFXX_WRT_CMD(0x9850);//!mp,9810=30kHz,MAX OUT
	RFXX_WRT_CMD(0xE000);//NOT USE

	RFXX_WRT_CMD(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
	RFXX_WRT_CMD(0x9850);//!mp,9810=30kHz,MAX OUT
	RFXX_WRT_CMD(0xE000);//NOT USE
	RFXX_WRT_CMD(0xC800);//NOT USE
	RFXX_WRT_CMD(0xC400);//1.66MHz,2.2V

}

void rf01_init(void) {
	RFXX_WRT_CMD(0x0000);
	RFXX_WRT_CMD(0x898A); //433BAND,134kHz
	RFXX_WRT_CMD(0xA640); //434MHz
	RFXX_WRT_CMD(0xC847); //4.8kbps
	RFXX_WRT_CMD(0xC69B); //AFC setting
	RFXX_WRT_CMD(0xC42A); //Clock recovery manual control,Digital filter,DQD=4
	RFXX_WRT_CMD(0xC240); //output 1.66MHz
	RFXX_WRT_CMD(0xC080);
	RFXX_WRT_CMD(0xCE84); //use FIFO
	RFXX_WRT_CMD(0xCE87);
	RFXX_WRT_CMD(0xC081); //OPEN RX
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

/* vim: set sts=0: */
