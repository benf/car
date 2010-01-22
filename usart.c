// \author Jan Klemkow
// \author Maik Rungberg
#include <avr/io.h>

#include "usart_cfg.h"
#include "usart.h"

uint8_t usart_receive(void) {
	// TODO: No Error Checks are here
	while ((UCSRA & (1 << RXC)) == 0);
	return UDR;
}

void usart_transmit(uint8_t data) {
	while ((UCSRA & (1 << UDRE)) == 0);
	UDR = data;
}

// functions used for debugging
int uputc(char c) {
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

	// UBBR{H,L}_VALUE, USE_2X and U2X is set by setbaud.h
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#if USE_2X // maybe set after baud-calculation by setbaud.h
	UCSRA |=  (1 << U2X);
#else
	UCSRA &= ~(1 << U2X);
#endif

}

/* vim: set sts=0 fenc=utf-8: */
