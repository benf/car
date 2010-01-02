//FUSE Settings:
//-U lfuse:w:0xee:m -U hfuse:w:0x99:m 
#ifndef F_CPU
#define F_CPU 4000000UL    // Systemtakt in Hz
#endif
 
#define BAUD 9600UL          // Baudrate

#if 0
// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include <stdint.h>
#include <inttypes.h>

#include "usart.h"

volatile uint8_t tmp;
ISR (USART_RXC_vect) {
	PORTD ^= 0xff;

	// loopback
	tmp = UDR;
	while ((UCSRA & (1 << UDRE)) == 0);
	// Send it back
	UDR = tmp;	
}
/*
unsigned char usart_receive(void) {
	while ((UCSRA & (1 << RXC)) == 0);
	return UDR;
}

void usart_transmit(unsigned char data) {
	while ((UCSRA & (1 << UDRE)) == 0);
	UDR = data;
}

int uputc(unsigned char c) {
	usart_transmit(c);
  return 0;
}
 
void uart_puts(char *s) {
	while (*s)
		uputc(*s++);
}

static void init_usart(void) {
	UCSRB |= (1 << TXEN)  | (1 << RXEN)  | (1 << RXCIE);    // UART TX RX einschalten + RX IRQ
	UCSRC |= (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);    // Asynchron 8N1 

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#if USE_2X // maybe set after baud-calculation by setbaud.h
	UCSRA |=  (1 << U2X);
#else
	UCSRA &= ~(1 << U2X);
#endif
	//old
	//UBRRH = UBRR_VAL >> 8;
	//UBRRL = UBRR_VAL & 0xFF;
}
*/

int main(void)
{
	init_usart();
	
	DDRB = 0xff;
	PORTB = 0;
	
	_delay_ms(2000);
	
	sei();
	PORTB = 0xff;
	_delay_ms(1000);
	PORTB = 0;
	uart_puts("hallo wie gehts\nja super :D\n");
	uart_puts("foo");

	//while(1);
	while (1) {
	// Wait until we got some data
	// while ((UCSRA & (1 << RXC)) == 0);

	// Read UART data
	//uint8_t data = UDR;

	// Send it back
	//UDR = data;
	}

	//{
	//while (!(UCSRA & (1<<UDRE)))  /* warten bis Senden moeglich    */
    //   {
      // }
//       UDR = 'x';
//			 UDR = '\n';
  //  }
	return 0;
}


// http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Der_UART#UART_initialisieren

