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
#define LED_OUTPUT() DDR_LED  |= 0x0F
#define LED0_ON()    PORT_LED &= ~(1<<1)
#define LED0_OFF()   PORT_LED |= (1<<1)
#define LED0_TRG()   PORT_LED ^= (1<<1)

uint8_t RF_RXBUF[22];
volatile uint8_t num = 0;

volatile uint8_t id = 0;

volatile uint8_t action;
volatile uint8_t param;

void cmd(uint8_t action, uint8_t param) {


//	if (action == 'S')

}


ISR (INT0_vect) {

	DDRC  |= (1 << DDC2);
	PORTC ^= (1 << PC2);

	uint8_t data = rf12_recv();
//	RFXX_WRT_CMD(0xCA81);
//	RFXX_WRT_CMD(0xCA83);
	
	//RFXX_WRT_CMD(0xCE84);   // reset FIFO for next frame recognition
	//RFXX_WRT_CMD(0xCE87);



/*
	switch (id++) {
		case 0:
			action = data;
			break;
		// parameter for command
		case 1:
			param  = data;
			break;
		// checksum
		case 2:
			if (data == _crc_ibutton_update(_crc_ibutton_update(0, action), param))
				cmd(action,param);
			id = 0;
			break;
	}
	*/		
	/*
	DDRC  |= (1 << DDC1);
	PORTC ^= (1 << PC1);

	DDRA  = 0xff;	
	PORTA = data;
	//send interrupt
	PORTC &= ~(1 << PC5);
	PORTC |=  (1 << PC5);
*/	
	uint8_t ChkSum, j;
	RF_RXBUF[num++] = data;
	if (num == 17) {
		num = 0;
//		RFXX_WRT_CMD(0xCE84);   // reset FIFO for next frame recognition
//		RFXX_WRT_CMD(0xCE87);
		ChkSum = 0;

		for (j = 0; j < 16; ++j)
			ChkSum += RF_RXBUF[j]; // calculate checksum

		if (ChkSum == RF_RXBUF[16]) {  // frame check
			LED0_TRG(); // receive indication
			PORTA = ChkSum;
		}

		RFXX_WRT_CMD(0xCA81); // reset fifo
		RFXX_WRT_CMD(0xCA83); //  - || -
	}
}


int main(void)
{
//	_delay_ms(500);
	rfxx_init();
	rf12_init(0);


	LED_OUTPUT();
	DDRC = 0xff;

	DDRD &= ~(1 << RFXX_nIRQ);

	GICR  = (1 << INT0);
	//	MCUCR = (1 << ISC01) | (1 << ISC00);

	sei();


	DDRA = 0;

	DDRC  |=  (1 << DDC5);
	PORTC |=  (1 << PC5);

	RFXX_WRT_CMD(0xCA83);

	while (1);
	return 0;
	/*
	uint8_t i, ChkSum = 0;
	uint8_t data[17];
	

	while (1) {
		rf12_recv_data(data,17);

		for (i = 0; i < 16; ++i)
			ChkSum += data[i];

		if (ChkSum == data[16])
			LED0_TRG();
	}
	return 0;
	*/
}

