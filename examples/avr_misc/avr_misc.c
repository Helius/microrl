#include "../../src/microrl.h"
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

/*
AVR platform specific implementation routines (for Atmega8, rewrite for your MC)
*/
#define _AVR_DEMO_VER "1.0"

// definition commands word
#define _CMD_HELP   "help"
#define _CMD_CLEAR  "clear"
#define _CMD_CLR    "clear_port"
#define _CMD_SET    "set_port"
// arguments for set/clear
	#define _SCMD_PB  "port_b"
	#define _SCMD_PD  "port_d"

#define _NUM_OF_CMD 4
#define _NUM_OF_SETCLEAR_SCMD 2

//available  commands
char * keyworld [] = {_CMD_HELP, _CMD_CLEAR, _CMD_SET, _CMD_CLR};
// 'set/clear' command argements
char * set_clear_key [] = {_SCMD_PB, _SCMD_PD};

// array for comletion
char * compl_world [_NUM_OF_CMD + 1];


void put_char (unsigned char ch);


//*****************************************************************************
void init (void)
{
	UBRRL = 8; // 19200 bps on 16MHz
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
	UCSRB = (1<<RXEN) | (1<<TXEN);
	DDRB=0xFF;
	DDRD=0xFF;
}

////*****************************************************************************
//void put_char (unsigned char ch)
//{
//		while (!( UCSRA & (1<<UDRE)));
//		UDR = (unsigned char) ch;
//}

//*****************************************************************************
void print (void * pThis, const char * str)
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

//*****************************************************************************
void print_help (void * pThis, void)
{
	print (pThis, "Use TAB key for completion\n\rCommand:\n\r");
	print (pThis, "\tclear               - clear screen\n\r");
	print (pThis, "\tset_port port pin   - set 1 port[pin] value, support only 'port_b' and 'port_d'\n\r");
	print (pThis, "\tclear_port port pin - set 0 port[pin] value, support only 'port_b' and 'port_d'\n\r");
}

//*****************************************************************************
void set_port_val (void * pThis, unsigned char * port, int pin, int val)
{
	if ((*port == PORTD) && (pin < 2) && (pin > 7)) {
		print (pThis, "only 2..7 pin avialable for PORTD\n\r");
		return;
	}
	
	if ((*port == PORTB) && (pin > 5)) {
		print (pThis, "only 0..5 pin avialable for PORTB\n\r");
		return;
	}

	if (val) {
		(*port) |= 1 << pin;
	} else {
		(*port) &= ~(1<<pin);
	}
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
int execute (void * pThis, int argc, const char * const * argv)
{
	int i = 0;
	// just iterate through argv word and compare it with your commands
	while (i < argc) {
		if (strcmp (argv[i], _CMD_HELP) == 0) {
			print (pThis, "microrl v");
			print (pThis, MICRORL_LIB_VER);
			print (pThis, " library AVR DEMO v");
			print (pThis, _AVR_DEMO_VER);
			print("\n\r");
			print_help (pThis);        // print help
		} else if (strcmp (argv[i], _CMD_CLEAR) == 0) {
			print (pThis, "\033[2J");    // ESC seq for clear entire screen
			print (pThis, "\033[H");     // ESC seq for move cursor at left-top corner
		} else if ((strcmp (argv[i], _CMD_SET) == 0) || 
							(strcmp (argv[i], _CMD_CLR) == 0)) {
			if (++i < argc) {
				int val = strcmp (argv[i-1], _CMD_CLR);
				unsigned char * port = NULL;
				int pin = 0;
				if (strcmp (argv[i], _SCMD_PD) == 0) {
					port = (unsigned char *)&PORTD;
				} else if (strcmp (argv[i], _SCMD_PB) == 0) {
					port = (unsigned char *)&PORTB;
				} else {
					print (pThis, "only '");
					print (pThis, _SCMD_PB);
					print (pThis, "' and '");
					print (pThis, _SCMD_PD);
					print (pThis, "' support\n\r");
					return 1;
				}
				if (++i < argc) {
					pin = atoi (argv[i]);
					set_port_val (pThis, port, pin, val);
					return 0;
				} else {
					print (pThis, "specify pin number, use Tab\n\r");
					return 1;
				}
			} else {
					print (pThis, "specify port, use Tab\n\r");
				return 1;
			}
		} else {
			print (pThis, "command: '");
			print (pThis, (char*)argv[i]);
			print (pThis, "' Not found.\n\r");
		}
		i++;
	}
	return 0;
}

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet (void * pThis, int argc, const char * const * argv)
{
	int j = 0;

	compl_world [0] = NULL;

	// if there is token in cmdline
	if (argc == 1) {
		// get last entered token
		char * bit = (char*)argv [argc-1];
		// iterate through our available token and match it
		for (int i = 0; i < _NUM_OF_CMD; i++) {
			// if token is matched (text is part of our token starting from 0 char)
			if (strstr(keyworld [i], bit) == keyworld [i]) {
				// add it to completion set
				compl_world [j++] = keyworld [i];
			}
		}
	}	else if ((argc > 1) && ((strcmp (argv[0], _CMD_SET)==0) || 
													 (strcmp (argv[0], _CMD_CLR)==0))) { // if command needs subcommands
		// iterate through subcommand
		for (int i = 0; i < _NUM_OF_SETCLEAR_SCMD; i++) {
			if (strstr (set_clear_key [i], argv [argc-1]) == set_clear_key [i]) {
				compl_world [j++] = set_clear_key [i];
			}
		}
	} else { // if there is no token in cmdline, just print all available token
		for (; j < _NUM_OF_CMD; j++) {
			compl_world[j] = keyworld [j];
		}
	}

	// note! last ptr in array always must be NULL!!!
	compl_world [j] = NULL;
	// return set of variants
	return compl_world;
}
#endif

//*****************************************************************************
void sigint (void * pThis)
{
	print (pThis, "^C catched!\n\r");
}
