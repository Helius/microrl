/*
 * File:   uart.h
 */

#ifndef UART_H
#define	UART_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif


extern void uart_setup();
extern void uart_write(uint8_t size, const uint8_t* buffer);
extern BOOL uart_read(uint8_t* buffer, uint8_t nbToRead);
extern void uart_clean(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

