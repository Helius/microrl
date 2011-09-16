#include <string.h>
#include <avr/io.h>

/*
AVR platform specific implementation routines (for Atmega8, rewrite for your MC)
*/

//*****************************************************************************
void init (void)
{
	UBRRL = 51; // 19200 bps on 16MHz
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
	UCSRB = (1<<RXEN) | (1<<TXEN);
}

//*****************************************************************************
void print (char * str)
{
	int i = 0;
	while (str [i] != 0) {
		while (!( UCSRA & (1<<UDRE)));
		UDR = str[i++];
	}
}

//*****************************************************************************
char get_char (void) 
{
	while (!(UCSRA & (1<<RXC)));
	return UDR;
}
