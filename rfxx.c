#include <avr/io.h>

//! \file rf12_cfg.h
#include "rf12_cfg.h"
//! \file rfxx.h
#include "rfxx.h"


/** 
 * \brief Kommando ans Funkmodul senden und empfangen
 *
 * Diese Funktion sendet \a cmd an ein RFxx Funkmodul
 * unter Nutzung der SPI-Kommando Schnittstelle der Module.
 *
 * Die Übertragung verläuft full-duplex sodass,
 * senden und empfagen gleichzeitig stattfindet.
 *
 * \param	cmd		Befehl fürs Funkmodul
 * \return			Antwort des Moduls auf den Befehl (z.b. FIFO read)
 */

uint16_t rfxx_wrt_cmd(uint16_t cmd){
	uint16_t response = 0;
	// chip select (SS low active)
	PORT_SPI &= ~(1 << SPI_SS);

#if SOFT_SPI
	uint8_t  i;

	PORT_SPI &= ~(1 << SPI_SCK);
	
	for (i = 0; i < 16; ++i) {
		if (cmd & (1 << 15))
			PORT_SPI |=  (1 << SPI_MOSI);
		else
			PORT_SPI &= ~(1 << SPI_MOSI);

		PORT_SPI |=  (1 << SPI_SCK);
	  
		response <<= 1;
		if (PIN_SPI & (1 << SPI_MISO))
			response |= 0x0001;

		PORT_SPI &= ~(1 << SPI_SCK);
		cmd <<= 1;
	}
	PORT_SPI |= (1 << SPI_SS);
#else
	// split 16bit to 2 x 8bit (
	uint8_t hi  = (cmd >> 8) & 0xff;
	uint8_t low = cmd & 0xff;


	// send cmd's hi-byte first (write MOSI)
	SPDR = hi;
	// wait until transfer is complete
	while ((SPSR & (1 << SPIF)) == 0);
	// receive answer's hi-byte (read MISO)
	response = (SPDR << 8) & 0xff;

	// send cmd's low-byte 
	SPDR = low;
	while ((SPSR & (1 << SPIF)) == 0);
	// receive answer's low-byte
	response |= SPDR & 0xff;

#endif
	// disable chip select
	PORT_SPI |= (1 << SPI_SS);
	return response;
}

/**
 * \brief ein Byte per Funk senden
 */

void rf12_send(uint8_t data) {
	while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ)); // wait for prev TX to be over
	// the data is encoded into a command that is sent via spi
	rfxx_wrt_cmd(0xb800 | data);
}

/**
 * \brief Initialisierung der SPI Schnittstelle zur Kommunikation mit den Funkmodulen
 */
void rfxx_init(void) {
	//_delay_ms(200);

	DDR_SPI  |=  (1 << SPI_SS);
	DDR_SPI  |=  (1 << SPI_MOSI);
	DDR_SPI  &= ~(1 << SPI_MISO);
	DDR_SPI  |=  (1 << SPI_SCK); 

	// disable chip select (low active)
	PORT_SPI |= (1 << SPI_SS);

	RFXX_nIRQ_PORT &= ~(1 << RFXX_nIRQ);

#if SOFT_SPI
	PORT_SPI |=  (1 << SPI_MOSI);
	PORT_SPI &= ~(1 << SPI_SCK);
#else
	// hardware spi init: spi enable, master mode, fOSC/16 sck freq
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
#endif

}

/**
 * \brief ein byte Daten lesen
 */

uint8_t rf12_recv(void) {

	uint16_t data;
	// while (RFXX_nIRQ_PIN & (1 << RFXX_nIRQ));

	// send a status read command
	// THIS IS NOT DOCUMENTED - was just used in example code
	// and did NOT work without this
	//
	// notice: 
	// this command would send the fifo data after 16 status bits
	// but: the command is 16bit long so we cant read it without
	// sending a new command
	//
	// conclosion: the receiption is initiated by this command
	// but read in the following
	rfxx_wrt_cmd(0x0000);

	// send the real read command - the data is clocked out while receiption
	// seems as this acts just as a dummy for receiving
	// the transfer of data is initiated by the previous command
	data = rfxx_wrt_cmd(0xb000);
	
	return (uint8_t) 0x00ff & data;
}
/**
 * \brief Lese \a num emfpangene Bytes vom Funkmodul
 */

// this function was used while debugging 
// but this is blocking mode
// and we need nonblocking mode => irq
void rf12_recv_data(uint8_t *data, uint8_t num) {
	uint8_t i;
	// disabe fifo
	rfxx_wrt_cmd(0xCA81);
	// enable FIFO
	rfxx_wrt_cmd(0xCA83);
	for (i = 0; i < num; ++i)
		*data++ = rf12_recv();
}

/**
 * \brief Sende \a num Bytes per Funk
 */
void rf12_send_data(uint8_t *data, uint8_t num) {
	uint8_t i;
	
	// again a status read command as while receiption
	rfxx_wrt_cmd(0x0000);
	// enable TX, PLL, synthesizer, crystal
	rfxx_wrt_cmd(0x8239); 

	// preamble
	for (i = 0; i < 3; ++i)
		rf12_send(0xAA);

	// syncron pattern
	rf12_send(0x2D);
	rf12_send(0xD4);

	// DATA
	for (i = 0; i < num; ++i)
		rf12_send(data[i]);

	// preamble / dummy byte
	for (i = 0; i < 3; ++i)
		rf12_send(0xAA);

	// disable tx again
	rfxx_wrt_cmd(0x8201);
}

/**
 * \brief Initialisierung des Funkmoduls
 */
void rf12_init(uint8_t transfer) {

	rfxx_wrt_cmd(0x80D8); //EL,EF,433band,12.5pF

	rfxx_wrt_cmd(0x8209 | (transfer ? 0x0030 : 0x00D0));

	// the following command are taken from example code
	rfxx_wrt_cmd(0xA640); //434MHz
	rfxx_wrt_cmd(0xC647); //4.8kbps
	rfxx_wrt_cmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
	rfxx_wrt_cmd(0xC2AC); //AL,!ml,DIG,DQD4
	rfxx_wrt_cmd(0xCA81); //FIFO8,SYNC,!ff,DR{
	rfxx_wrt_cmd(0x80D8); //EL,EF,433band,12.5pF
	rfxx_wrt_cmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
	rfxx_wrt_cmd(0x9850); // !mp,9810=30kHz,MAX OUT
	rfxx_wrt_cmd(0xE000); //NOT USE
	rfxx_wrt_cmd(0xC800); //NOT USE
	rfxx_wrt_cmd(0xC400); //1.66MHz,2.2V

}


// the following is old stuff
// we had other types of radio-chips
// these are the old routines..
#if 0
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
void rf02_send_data(uint8_t *data, uint8_t num) {
	rfxx_wrt_cmd(0xC039); // START TX
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
	rfxx_wrt_cmd(0xC001); // CLOSE TX
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

void rf02_init(void) {
	rfxx_wrt_cmd(0xCC00);
	rfxx_wrt_cmd(0x8B81); // 433BAND,+/-60kHz
	rfxx_wrt_cmd(0xA640); // 434MHz
	rfxx_wrt_cmd(0xC847); // 4.8kbps
	rfxx_wrt_cmd(0xC220); // ENABLE BIT SYNC
	rfxx_wrt_cmd(0xC001); // CLOSE ALL

	PORTB = (1 << RFXX_FSK);
}
#endif
/* vim: set sts=0 fenc=utf-8: */
