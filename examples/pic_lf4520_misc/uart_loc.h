#ifndef UART_LOC_H
#define	UART_LOC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define USART_RX_BUFFER_SIZE          (50)

typedef struct
{
  uint8_t  framingError;
  uint8_t  overrunError;
  uint8_t  bufferOverrunError;
} UART_errorT;

extern UART_errorT UART_errorStatus;
extern uint8_t UART_rxBufferNb;
extern uint8_t UART_rxBuffer[USART_RX_BUFFER_SIZE];
extern uint8_t UART_writeIndex;

#ifdef	__cplusplus
}
#endif

#endif	/* UART_LOC_H */

