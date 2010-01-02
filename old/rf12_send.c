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
unsigned int rfxx_wrt_cmd(unsigned int aCmd){
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
  rfxx_wrt_cmd(0x80D8);//EL,EF,433band,12.5pF
  rfxx_wrt_cmd(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
  rfxx_wrt_cmd(0xA640);//A140=430.8MHz
  rfxx_wrt_cmd(0xC647);//19.2kbps
  rfxx_wrt_cmd(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
  rfxx_wrt_cmd(0xC2AC);//AL,!ml,DIG,DQD4
  rfxx_wrt_cmd(0xCA81);//FIFO8,SYNC,!ff,DR
  rfxx_wrt_cmd(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
  rfxx_wrt_cmd(0x9850);//!mp,9810=30kHz,MAX OUT
  rfxx_wrt_cmd(0xE000);//NOT USE
  rfxx_wrt_cmd(0xC800);//NOT USE
  rfxx_wrt_cmd(0xC400);//1.66MHz,2.2V
}
void rf12_send(unsigned char aByte){
  while(PIND&(1<<2));//wait for previously TX over
  rfxx_wrt_cmd(0xB800+aByte);
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
	  rfxx_wrt_cmd(0x0000);//read status register
	  rfxx_wrt_cmd(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
	  ChkSum=0;
	  rf12_send(0xAA);//PREAMBLE
	 	LEDR_OFF();
		rf12_send(0xAA);//PREAMBLE
  rf12_send(0xAA);//PREAMBLE
  rf12_send(0x2D);//SYNC HI BYTE
  rf12_send(0xD4);//SYNC LOW BYTE
  rf12_send(0x30);//DATA BYTE 0
  ChkSum+=0x30;
  rf12_send(0x31);//DATA BYTE 1
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
     rf12_send(ChkSum); //send chek sum
     rf12_send(0xAA);//DUMMY BYTE
     rf12_send(0xAA);//DUMMY BYTE
     rf12_send(0xAA);//DUMMY BYTE
     rfxx_wrt_cmd(0x8201);
     LEDR_OFF();
     LEDG_OFF();
		 _delay_ms(1000);
  };
}

