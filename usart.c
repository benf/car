#include <avr/io.h>

#include "usart_cfg.h"
#include "usart.h"

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

void init_usart(void) {
	UCSRB |= (1 << TXEN)  | (1 << RXEN)  | (1 << RXCIE);    // UART TX RX einschalten + RX IRQ
	UCSRC |= (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);    // Asynchron 8N1 

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#if USE_2X // maybe set after baud-calculation by setbaud.h
	UCSRA |=  (1 << U2X);
#else
	UCSRA &= ~(1 << U2X);
#endif
	/* //old
	UBRRH = UBRR_VAL >> 8;
	UBRRL = UBRR_VAL & 0xFF;
	*/
}

/* vim: set sts=0: */
