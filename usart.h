#ifndef _USART_H_
#define _USART_H_

#include "usart_cfg.h"

unsigned char usart_receive(void);

void usart_transmit(unsigned char data);

int uputc(unsigned char c);
 
void uart_puts(char *s);

void init_usart(void);

#endif
/* vim: set sts=0 fenc=utf-8: */
