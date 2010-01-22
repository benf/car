#ifndef _USART_CFG_H_
#define _USART_CFG_H_

//#define F_CPU 4000000UL
#define F_CPU 14745600UL

//#define BAUD 9600UL
#define BAUD 2400UL
#include <util/setbaud.h>


// FUSE-FLAGS for external quarz
// -U lfuse:w:0xee:m -U hfuse:w:0x99:m 
#endif // _USART_CFG_H_

/* vim: set sts=0 fenc=utf-8: */
