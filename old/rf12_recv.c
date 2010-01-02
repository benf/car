#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DDR_IN   0
#define DDR_OUT  1
#define PORT_SEL PORTB
#define PIN_SEL  PINB
#define DDR_SEL  DDRB
#define PORT_SDI PORTB
#define PIN_SDI  PINB
#define DDR_SDI  DDRB
#define PORT_SCK PORTB
#define PIN_SCK  PINB
#define DDR_SCK  DDRB
#define PORT_SDO PORTB
#define PIN_SDO  PINB
#define DDR_SDO  DDRB
#define PORT_IRQ PORTD
#define PIN_IRQ        PIND
#define DDR_IRQ        DDRD
#define PORT_DATA      PORTD
#define PIN_DATA       PIND
#define DDR_DATA       DDRD
#define PB7            7//--
#define PB6            6// |
#define RFXX_SCK       7 // |
#define RFXX_SDO       6// |RF_PORT
#define RFXX_SDI       5// |
#define RFXX_SEL       4// |
#define NC             1// |
#define PB0            0//--
#define SEL_OUTPUT()   DDR_SEL |= (1<<RFXX_SEL)
#define HI_SEL()       PORT_SEL|= (1<<RFXX_SEL)
#define LOW_SEL()      PORT_SEL&=~(1<<RFXX_SEL)
#define SDI_OUTPUT()   DDR_SDI |= (1<<RFXX_SDI)
#define HI_SDI()       PORT_SDI|= (1<<RFXX_SDI)
#define LOW_SDI()      PORT_SDI&=~(1<<RFXX_SDI)
#define SDO_INPUT()    DDR_SDO&= ~(1<<RFXX_SDO)
#define LOW_SDO()      PORT_SDO&= (1<<RFXX_SDO)
#define SDO_HI()       PIN_SDO&(1<<RFXX_SDO)
#define SCK_OUTPUT()   DDR_SCK |= (1<<RFXX_SCK)
#define HI_SCK()       PORT_SCK|= (1<<RFXX_SCK)
#define LOW_SCK()      PORT_SCK&=~(1<<RFXX_SCK)
#define RF12_IRQ       2
#define IRQ_IN()       DDR_IRQ &=~(1<<RF12_IRQ)
#define WAIT_IRQ_LOW() while(PIND&(1<<RF12_IRQ))
#define RF12_DATA      4//PD4
#define DATA_OUT()     DDR_DATA|=1<<RF12_DATA
#define HI_DATA()      PORT_DATA|=1<<RF12_DATA
#define LEDG_OUTPUT()  DDRC|=(1<<0)
#define LEDR_OUTPUT()  DDRC|=(1<<1)
#define LEDG_ON()      PORTC|=(1<<0)

#define LEDG_OFF()      PORTC&= ~(1<<0)
#define LEDR_ON()       PORTC|=(1<<1)
#define LEDR_OFF()      PORTC&= ~(1<<1)

#define SOFT_SPI 1
#define PORT_SPI   PORTB
#define PIN_SPI    PINB
#define DDR_SPI    DDRB
#define SPI_SS     PB4
#define SPI_MOSI   PB5
#define SPI_MISO   PB6
#define SPI_SCK    PB7

#define RFXX_nIRQ  PD2

#define RFXX_FSK   PB1

#include "rfxx.h"

void RFXX_PORT_INIT(void){
  HI_SEL();
  HI_SDI();
  LOW_SCK();
  //SET nFFS pin HI when using FIFO
  HI_DATA();
  SEL_OUTPUT();
  SDI_OUTPUT();
  SDO_INPUT();
  SCK_OUTPUT();
  IRQ_IN();
  DATA_OUT();
}
/*
unsigned int rfxx_wrt_cmd(unsigned int aCmd){
  unsigned char i;
  unsigned int temp;
  temp=0;
  LOW_SCK();
  LOW_SEL();
  for(i=0;i<16;i++){
     if(aCmd&0x8000){
       HI_SDI();
     }else{
       LOW_SDI();
     }
     HI_SCK();
     temp<<=1;
     if(SDO_HI()){
       temp|=0x0001;
     }
     LOW_SCK();
     aCmd<<=1;
  };
  HI_SEL();
  return(temp);
}
*/
void RF12_INIT(void){
  rfxx_wrt_cmd(0x80D8);//EL,EF,433band,12.5pF

  rfxx_wrt_cmd(0x82D9);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
  rfxx_wrt_cmd(0xA640);//434MHz
  rfxx_wrt_cmd(0xC647);//4.8kbps
  rfxx_wrt_cmd(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
  rfxx_wrt_cmd(0xC2AC);//AL,!ml,DIG,DQD4
  rfxx_wrt_cmd(0xCA81);//FIFO8,SYNC,!ff,DR
  rfxx_wrt_cmd(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
  rfxx_wrt_cmd(0x9850);//!mp,9810=30kHz,MAX OUT
  rfxx_wrt_cmd(0xE000);//NOT USE
  rfxx_wrt_cmd(0xC800);//NOT USE
  rfxx_wrt_cmd(0xC400);//1.66MHz,2.2V
}

unsigned char RF12_RECV(void){
  unsigned int FIFO_data;
  WAIT_IRQ_LOW();
  rfxx_wrt_cmd(0x0000);
  FIFO_data=rfxx_wrt_cmd(0xB000);
  return(FIFO_data&0x00FF);
}
void Delay_ms(unsigned char amS){
  unsigned char i;
  unsigned int j;
  for(i=0;i<amS;i++)for(j=0;j<914;j++);
}
void main(void)
{
  unsigned char i;
  unsigned char ChkSum;
  //POWER ON indication: both LEDR and LEDG blink 3 times
  LEDG_OFF();
  LEDR_OFF();
  LEDG_OUTPUT();
  LEDR_OUTPUT();
	
  for(i=0;i<3;i++){
    Delay_ms(200);
    LEDG_ON();
    LEDR_ON();
    Delay_ms(200);
    LEDG_OFF();
    LEDR_OFF();
  }
    LEDG_OFF();
    LEDR_OFF();
  //Initialize command port
	rfxx_init();
  //RFXX_PORT_INIT();
  //Initialize RF12 chip
	rf12_init_recv();

  //RF12_INIT();
  //Init FIFO
  //rfxx_wrt_cmd(0xCA81);
  while(1){
    //Enable FIFO
    rfxx_wrt_cmd(0xCA83);
    ChkSum=0;
    //Receive payload data
    for(i=0;i<16;i++){
      //ChkSum+=RF12_RECV();
			ChkSum += rf12_recv();
    }
    	rfxx_wrt_cmd(0xCA83);
    //Receive Check sum
	//	i=RF12_RECV();
	  i=rf12_recv();
    //Disable FIFO
    rfxx_wrt_cmd(0xCA81);
		
    //Package chkeck
    if(ChkSum==i){
      LEDG_ON();
			_delay_ms(200);
      LEDG_OFF();
    }
  }
}

