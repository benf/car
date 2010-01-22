// \author Jan Klemkow
// \author Maik Rungberg
#ifndef _USART_H_
#define _USART_H_

#include <stdint.h>
#include "usart_cfg.h"

uint8_t usart_receive(void);

void usart_transmit(uint8_t data);

int uputc(char c);
 
void uart_puts(char *s);

void init_usart(void);

#endif
/* vim: set sts=0 fenc=utf-8: */
