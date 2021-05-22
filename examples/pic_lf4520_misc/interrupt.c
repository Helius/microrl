#include <xc.h>
#include <stdint.h>
#include "uart_loc.h"

uint8_t UART_writeIndex = 0;
//extern uint8_t nbCharRecu;

void interrupt high_priority isr_hi(void)
{
  //IT I2C
  if (PIE1bits.SSPIE & PIR1bits.SSPIF)
  {
    PIR1bits.SSPIF = 0;
    //i2c_interupt_done = TRUE;
    return;
  }

  //IT TIMER
  if (INTCONbits.TMR0IE & INTCONbits.TMR0IF)
  {
    //timer0_interupt_done = TRUE;
    //clear interrupt flags
    INTCONbits.TMR0IF = 0;
    return;
  }

  //IT A/D conversion
  if (PIE1bits.ADIE & PIR1bits.ADIF)
  {
    PIR1bits.ADIF = 0;
    return;
  }

  //IT UART RECEPTION
  if (PIE1bits.RC1IE & PIR1bits.RCIF)
  {
    uint8_t dummy;
    //nbCharRecu++;
    //USART character reception
    //get status firt RCSTA;
    if (RCSTAbits.FERR)
    {
      //framing error
      UART_errorStatus.framingError++;
      //LATD |= 0x02;
      dummy = RCREG;
      RESET(); //will perform a recalibration and should remove frame error
    }
    else if (RCSTAbits.OERR)
    {
      //overrun error, set reception again
      UART_errorStatus.overrunError++;
      dummy = RCREG;
      //LATD |= 0x04;
      RCSTAbits.CREN = 0;
    }
    else if (UART_rxBufferNb < USART_RX_BUFFER_SIZE)
    {
      UART_rxBuffer[UART_writeIndex] = RCREG;
      UART_writeIndex++;
      if (UART_writeIndex >= USART_RX_BUFFER_SIZE)
      {
        UART_writeIndex = 0;
      }
      UART_rxBufferNb++;
    }
    else
    {
      //we can not store the char, read and indicate error
      UART_errorStatus.bufferOverrunError++;
      dummy = RCREG;
    }
    return ;
  }
}


/***
 *  Just to remember, will be not used - not necessary
 **/

/*
void interrupt low_priority isr_lo(void)
{
    gData[0]= 0;
}
*/



