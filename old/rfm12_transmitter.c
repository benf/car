#ifndef F_CPU
#define F_CPU 4000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define DDR_IN        0
#define DDR_OUT       1

#define PORT_SEL      PORTB
#define PIN_SEL       PINB
#define DDR_SEL       DDRB

#define PORT_SDI      PORTB
#define PIN_SDI       PINB
#define DDR_SDI       DDRB

#define PORT_SCK      PORTB
#define PIN_SCK       PINB
#define DDR_SCK       DDRB

#define PORT_SDO      PORTB
#define PIN_SDO       PINB
#define DDR_SDO       DDRB

#define PORT_DATA     PORTD
#define PIN_DATA      PIND
#define DDR_DATA      DDRD

//#define PB7         PB7   //---
//#define PB6         PB6   //  |
#define RFXX_SCK      PB5   //  |
#define RFXX_SDO      PB4   //  |-- RF_PORT
#define RFXX_SDI      PB3   //  |
#define RFXX_SEL      PB2   //  |
#define NC            PB1   //  |
//#define PB0         PB0   //---

#define SEL_OUTPUT()  DDR_SEL  |=  (1 << RFXX_SEL)
#define HI_SEL()      PORT_SEL |=  (1 << RFXX_SEL)
#define LOW_SEL()     PORT_SEL &= ~(1 << RFXX_SEL)

#define SDI_OUTPUT()  DDR_SDI  |=  (1 << RFXX_SDI)
#define HI_SDI()      PORT_SDI |=  (1 << RFXX_SDI)
#define LOW_SDI()     PORT_SDI &= ~(1 << RFXX_SDI)

#define SDO_INPUT()   DDR_SDO  &= ~(1 << RFXX_SDO)
#define SDO_HI()      PIN_SDO  &   (1 << RFXX_SDO)

#define SCK_OUTPUT()  DDR_SCK  |=  (1 << RFXX_SCK)
#define HI_SCK()      PORT_SCK |=  (1 << RFXX_SCK)
#define LOW_SCK()     PORT_SCK &= ~(1 << RFXX_SCK)

#define RF12_DATA     PD4
#define DATA_OUT      DDR_DATA  |= (1 << RF12_DATA)
#define HI_DATA       PORT_DATA |= (1 << RF12_DATA)

#define LEDG_OUTPUT() DDRD  |= ~(1 << PD6)
#define LEDR_OUTPUT() DDRD  |= ~(1 << PD7)
#define LEDG_ON()     PORTD &= ~(1 << PD6)
#define LEDG_OFF()    PORTD |= ~(1 << PD6)
#define LEDR_ON()     PORTD &= ~(1 << PD7)
#define LEDR_OFF()    PORTD |= ~(1 << PD7)

void RFXX_PORT_INIT(void) {
  HI_SEL();
  HI_SDI();
  LOW_SCK();

  SEL_OUTPUT();
  SDI_OUTPUT();
  SDO_INPUT();
  SCK_OUTPUT();
}

uint16_t rfxx_wrt_cmd(uint16_t aCmd) {
	uint8_t  i;
	uint16_t temp = 0;

	LOW_SCK();
	LOW_SEL();

	//for (i=0; i<16; i++) {
	for (i = 0; i < sizeof(uint16_t); ++i) {

		temp <<= 1;
		if (SDO_HI()) temp |= 0x0001;

		LOW_SCK();
		(aCmd & 0x8000) ? (HI_SDI()) : (LOW_SDI());
		/*
		if (aCmd & 0x8000)
			HI_SDI();
		else
			LOW_SDI();
		*/
		HI_SCK();

		aCmd <<= 1;
	}

	LOW_SCK();
	HI_SEL();

	return temp;
}

void RF12_INIT(void){
  rfxx_wrt_cmd(0x80D8); //EL,EF,433band,12.5pF
  rfxx_wrt_cmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
  rfxx_wrt_cmd(0xA640); //A140=430.8MHz
  //rfxx_wrt_cmd(0xC647); //19.2kbps
	rfxx_wrt_cmd(0xC847); // 4.8kbps (for rf01)

  rfxx_wrt_cmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
  rfxx_wrt_cmd(0xC2AC); //AL,!ml,DIG,DQD4
  rfxx_wrt_cmd(0xCA81); //FIFO8,SYNC,!ff,DR
  rfxx_wrt_cmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
  rfxx_wrt_cmd(0x9850); //!mp,9810=30kHz,MAX OUT
  rfxx_wrt_cmd(0xE000); //NOT USE
  rfxx_wrt_cmd(0xC800); //NOT USE
  rfxx_wrt_cmd(0xC400); //1.66MHz,2.2V
}

void rf12_send(uint8_t aByte) {
  while (PIND & (1 << PD2)); //wait for previously TX over
//  rfxx_wrt_cmd(0xB800 + aByte);
	rfxx_wrt_cmd(0xb800 | aByte);

}

#if 0
void Delay_ms(unsigned char amS){
  unsigned char i;
  unsigned int j;
  for(i=0;i<amS;i++)for(j=0;j<914;j++);
}
#endif

int main(void) {
	uint8_t i;
	uint8_t ChkSum;

	cli();
	//#asm("cli");

	DDRB = 0x00; //PB INPUT;
	DDRD = 0x00; //PD INPUT;

	//POWER ON indication: both LEDR and LEDG blink 3 times
	LEDG_OFF();
	LEDR_OFF();
	LEDG_OUTPUT();
	LEDR_OUTPUT();

	for (i=0; i<3; i++) {
		_delay_ms(200);
		//Delay_ms(200);
		LEDG_ON();
		LEDR_ON();
		_delay_ms(200);
		//Delay_ms(200);
		LEDG_OFF();
		LEDR_OFF();
	}
	LEDG_OFF();
	LEDR_OFF();
	RFXX_PORT_INIT();
	RF12_INIT();

	DDRD  |=  (1 << RF12_DATA); 
	PORTD |=  (1 << RF12_DATA); // SET nFFS pin HI when using TX register
	DDRD  &= ~(1 << DDD2);      // PD2(INT0)

	while (1) {

		LEDR_ON();

		rfxx_wrt_cmd(0x0000); //read status register
		rfxx_wrt_cmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
		ChkSum=0;

		rf12_send(0xAA); //PREAMBLE
		rf12_send(0xAA); //PREAMBLE
		rf12_send(0xAA); //PREAMBLE

		rf12_send(0x2D); //SYNC HI BYTE
		rf12_send(0xD4); //SYNC LOW BYTE

		// send data bytes
		for (i = 0x30; i<=0x3F; ++i) {
			rf12_send(i);
			ChkSum += i;
		}

		/*
			 rf12_send(0x30); //DATA BYTE 0
			 ChkSum+=0x30;
			 rf12_send(0x31); //DATA BYTE 1
			 ChkSum+=0x31;
			 rf12_send(0x32);
			 ChkSum+=0x32;
			 rf12_send(0x33);
			 ChkSum+=0x33;
			 rf12_send(0x34);
			 ChkSum+=0x34;
			 rf12_send(0x35);
			 ChkSum+=0x35;
			 rf12_send(0x36);
			 ChkSum+=0x36;
			 rf12_send(0x37);
			 ChkSum+=0x37;
			 rf12_send(0x38);
			 ChkSum+=0x38;
			 rf12_send(0x39);
			 ChkSum+=0x39;
			 rf12_send(0x3A);
			 ChkSum+=0x3A;
			 rf12_send(0x3B);
			 ChkSum+=0x3B;
			 rf12_send(0x3C);
			 ChkSum+=0x3C;
			 rf12_send(0x3D);
			 ChkSum+=0x3D;
			 rf12_send(0x3E);
			 ChkSum+=0x3E;
			 rf12_send(0x3F);    //DATA BYTE 15
			 ChkSum+=0x3F;
			 */

		rf12_send(ChkSum); //send chek sum

		rf12_send(0xAA); //DUMMY BYTE
		rf12_send(0xAA); //DUMMY BYTE
		rf12_send(0xAA); //DUMMY BYTE

		rfxx_wrt_cmd(0x8201);
		LEDR_OFF();
		LEDG_OFF();

		//for(i=0;i<10000;i++)
		//  for(j=0;j<123;j++); //sleep 1 second appr.
		_delay_ms(1000);
	}
}

