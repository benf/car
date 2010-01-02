#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>


#define DDR_IN       0
#define DDR_OUT      1
#define PORT_SEL     PORTB
#define PIN_SEL      PINB
#define DDR_SEL      DDRB
#define PORT_SDI     PORTB
#define PIN_SDI      PINB
#define DDR_SDI      DDRB
#define PORT_SCK     PORTB
#define PIN_SCK      PINB
#define DDR_SCK      DDRB
#define PORT_SDO     PORTB
#define PIN_SDO      PINB
#define DDR_SDO      DDRB
#define PORT_DATA    PORTD
#define PIN_DATA     PIND
#define DDR_DATA     DDRD
#define PB7          7//--
#define PB6          6// |
#define RFXX_SCK     7// |
#define RFXX_SDO     6// |RF_PORT
#define RFXX_SDI     5// |
#define RFXX_SEL     4// |
#define NC           1// |
#define PB0          0//--
#define SEL_OUTPUT() DDR_SEL |= (1<<RFXX_SEL)
#define HI_SEL()     PORT_SEL|= (1<<RFXX_SEL)
#define LOW_SEL()    PORT_SEL&=~(1<<RFXX_SEL)
#define SDI_OUTPUT() DDR_SDI |= (1<<RFXX_SDI)
#define HI_SDI()     PORT_SDI|= (1<<RFXX_SDI)
#define LOW_SDI()    PORT_SDI&=~(1<<RFXX_SDI)
#define SDO_INPUT()  DDR_SDO&= ~(1<<RFXX_SDO)
#define SDO_HI()      PIN_SDO&(1<<RFXX_SDO)
#define SCK_OUTPUT()  DDR_SCK |= (1<<RFXX_SCK)
#define HI_SCK()      PORT_SCK|= (1<<RFXX_SCK)
#define LOW_SCK()     PORT_SCK&=~(1<<RFXX_SCK)
#define RF12_DATA     4//PD4
#define DATA_OUT      DDR_DATA|=1<<RF12_DATA
#define HI_DATA       PORT_DATA|=1<<RF12_DATA
#define LEDG_OUTPUT() DDRC|=(1<<1)
#define LEDR_OUTPUT() DDRC|=(1<<0)
#define LEDG_ON()     PORTC|=(1<<1)
#define LEDG_OFF()    PORTC&=~(1<<1)
#define LEDR_ON()     PORTC|=(1<<0)
#define LEDR_OFF()    PORTC&=~(1<<0)

void RFXX_PORT_INIT(void){
  HI_SEL();
  HI_SDI();
  LOW_SCK();
  SEL_OUTPUT();
  SDI_OUTPUT();
  SDO_INPUT();
  SCK_OUTPUT();
}
unsigned int RFXX_WRT_CMD(unsigned int aCmd){
  unsigned char i;
  unsigned int temp = 0;
  LOW_SCK();
  LOW_SEL();
  for(i=0;i<16;i++){
    temp<<=1;
    if(SDO_HI()){
      temp|=0x0001;
    }
    LOW_SCK();
    if(aCmd&0x8000){
      HI_SDI();
    }else{
      LOW_SDI();
    }
     HI_SCK();
     aCmd<<=1;
  };
  LOW_SCK();
  HI_SEL();
  return(temp);
}
void RF12_INIT(void){
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
void RF12_SEND(unsigned char aByte){
  while(PIND&(1<<2));//wait for previously TX over
  RFXX_WRT_CMD(0xB800+aByte);
}
void Delay_ms(unsigned char amS){
  unsigned char i;
  unsigned int j;
  for(i=0;i<amS;i++)for(j=0;j<914;j++);
}
void main(void)
{
  unsigned int i,j;
  unsigned char ChkSum;
  DDRB=0x00;//PB INPUT;
  DDRD=0x00;//PD INPUT;
  //POWER ON indication: both LEDR and LEDG blink 3 times
  LEDG_OFF();
  LEDR_OFF();
  LEDG_OUTPUT();
	LEDR_OUTPUT();
	for(i=0;i<3;i++){
		_delay_ms(200);
	  LEDG_ON();
	  LEDR_ON();
		_delay_ms(200);
	  LEDG_OFF();
	  LEDR_OFF();
	}

  LEDG_OFF();
  LEDR_OFF();
	RFXX_PORT_INIT();
	RF12_INIT();
	DDRD|=(1<<RF12_DATA);
	PORTD|=(1<<RF12_DATA);// SET nFFS pin HI when using TX register
	DDRD&=~(1<<2);        //PD2(INT0)
	while(1){
	 	LEDR_ON();
	  RFXX_WRT_CMD(0x0000);//read status register
	  RFXX_WRT_CMD(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
	  ChkSum=0;
	  RF12_SEND(0xAA);//PREAMBLE
	 	LEDR_OFF();
		RF12_SEND(0xAA);//PREAMBLE
  RF12_SEND(0xAA);//PREAMBLE
  RF12_SEND(0x2D);//SYNC HI BYTE
  RF12_SEND(0xD4);//SYNC LOW BYTE
  RF12_SEND(0x30);//DATA BYTE 0
  ChkSum+=0x30;
  RF12_SEND(0x31);//DATA BYTE 1
  ChkSum+=0x31;
  RF12_SEND(0x32);
  ChkSum+=0x32;
  RF12_SEND(0x33);
  ChkSum+=0x33;
  RF12_SEND(0x34);
  ChkSum+=0x34;
  RF12_SEND(0x35);
     ChkSum+=0x35;
     RF12_SEND(0x36);
     ChkSum+=0x36;
     RF12_SEND(0x37);
     ChkSum+=0x37;
     RF12_SEND(0x38);
     ChkSum+=0x38;
     RF12_SEND(0x39);
     ChkSum+=0x39;
     RF12_SEND(0x3A);
     ChkSum+=0x3A;
     RF12_SEND(0x3B);
     ChkSum+=0x3B;
     RF12_SEND(0x3C);
     ChkSum+=0x3C;
     RF12_SEND(0x3D);
     ChkSum+=0x3D;
     RF12_SEND(0x3E);
     ChkSum+=0x3E;
     RF12_SEND(0x3F);    //DATA BYTE 15
     ChkSum+=0x3F;
     RF12_SEND(ChkSum); //send chek sum
     RF12_SEND(0xAA);//DUMMY BYTE
     RF12_SEND(0xAA);//DUMMY BYTE
     RF12_SEND(0xAA);//DUMMY BYTE
     RFXX_WRT_CMD(0x8201);
     LEDR_OFF();
     LEDG_OFF();
		 _delay_ms(1000);
  };
}

