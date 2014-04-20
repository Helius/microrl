#include "../../src/config.h"
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

/*
AVR platform specific implementation routines(for Atmega8, rewrite for your MC)
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


void put_char(void* user_handle, unsigned char ch);


//*****************************************************************************
void init(void* user_handle)
{
	(void)user_handle; /* Not used in this example */
	UBRRL = 8; // 19200 bps on 16MHz
	UCSRC =(1<<URSEL) |(1<<UCSZ1) |(1<<UCSZ0);
	UCSRB =(1<<RXEN) |(1<<TXEN);
	DDRB=0xFF;
	DDRD=0xFF;
}

////*****************************************************************************
//void put_char(void* user_handle, unsigned char ch)
//{
//		while(!( UCSRA &(1<<UDRE)));
//		UDR =(unsigned char) ch;
//}

//*****************************************************************************
void print(void* user_handle, const char * str)
{
	(void)user_handle; /* Not used in this example */
	int i = 0;
	while(str [i] != 0) {
		while(!( UCSRA &(1<<UDRE)));
		UDR = str[i++];
	}
}

//*****************************************************************************
char get_char(void* user_handle)
{
	(void)user_handle; /* Not used in this example */
	while(!(UCSRA &(1<<RXC)));
	return UDR;
}

//*****************************************************************************
void print_help(void* user_handle)
{
	print(user_handle, "Use TAB key for completion\n\rCommand:\n\r");
	print(user_handle, "\tclear               - clear screen\n\r");
	print(user_handle, "\tset_port port pin   - set 1 port[pin] value, support only 'port_b' and 'port_d'\n\r");
	print(user_handle, "\tclear_port port pin - set 0 port[pin] value, support only 'port_b' and 'port_d'\n\r");
}

//*****************************************************************************
void set_port_val(void* user_handle, unsigned char * port, int pin, int val)
{
	if((*port == PORTD) &&(pin < 2) &&(pin > 7)) {
		print(user_handle, "only 2..7 pin avialable for PORTD\n\r");
		return;
	}
	
	if((*port == PORTB) &&(pin > 5)) {
		print(user_handle, "only 0..5 pin avialable for PORTB\n\r");
		return;
	}

	if(val) {
		(*port) |= 1 << pin;
	} else {
		(*port) &= ~(1<<pin);
	}
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
int execute(void* user_handle, int argc, const char * const * argv)
{
	int i = 0;
	// just iterate through argv word and compare it with your commands
	while(i < argc) {
		if(strcmp(argv[i], _CMD_HELP) == 0) {
			print(user_handle,"microrl v");
			print(user_handle,MICRORL_LIB_VER);
			print(user_handle," library AVR DEMO v");
			print(user_handle,_AVR_DEMO_VER);
			print(user_handle,"\n\r");
			print_help(user_handle);        // print help
		} else if(strcmp(argv[i], _CMD_CLEAR) == 0) {
			print(user_handle,"\033[2J");    // ESC seq for clear entire screen
			print(user_handle,"\033[H");     // ESC seq for move cursor at left-top corner
		} else if((strcmp(argv[i], _CMD_SET) == 0) || 
							(strcmp(argv[i], _CMD_CLR) == 0)) {
			if(++i < argc) {
				int val = strcmp(argv[i-1], _CMD_CLR);
				unsigned char * port = NULL;
				int pin = 0;
				if(strcmp(argv[i], _SCMD_PD) == 0) {
					port =(unsigned char *)&PORTD;
				} else if(strcmp(argv[i], _SCMD_PB) == 0) {
					port =(unsigned char *)&PORTB;
				} else {
					print(user_handle,"only '");
					print(user_handle,_SCMD_PB);
					print(user_handle,"' and '");
					print(user_handle,_SCMD_PD);
					print(user_handle,"' support\n\r");
					return 1;
				}
				if(++i < argc) {
					pin = atoi(argv[i]);
					set_port_val(user_handle, port, pin, val);
					return 0;
				} else {
					print(user_handle,"specify pin number, use Tab\n\r");
					return 1;
				}
			} else {
					print(user_handle,"specify port, use Tab\n\r");
				return 1;
			}
		} else {
			print(user_handle,"command: '");
			print(user_handle,(char*)argv[i]);
			print(user_handle,"' Not found.\n\r");
		}
		i++;
	}
	return 0;
}

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet(void* user_handle, int argc, const char * const * argv)
{
	(void)user_handle; /* Not used in this example */
	int j = 0;

	compl_world [0] = NULL;

	// if there is token in cmdline
	if(argc == 1) {
		// get last entered token
		char * bit =(char*)argv [argc-1];
		// iterate through our available token and match it
		for(int i = 0; i < _NUM_OF_CMD; i++) {
			// if token is matched(text is part of our token starting from 0 char)
			if(strstr(keyworld [i], bit) == keyworld [i]) {
				// add it to completion set
				compl_world [j++] = keyworld [i];
			}
		}
	}	else if((argc > 1) &&((strcmp(argv[0], _CMD_SET)==0) || 
													(strcmp(argv[0], _CMD_CLR)==0))) { // if command needs subcommands
		// iterate through subcommand
		for(int i = 0; i < _NUM_OF_SETCLEAR_SCMD; i++) {
			if(strstr(set_clear_key [i], argv [argc-1]) == set_clear_key [i]) {
				compl_world [j++] = set_clear_key [i];
			}
		}
	} else { // if there is no token in cmdline, just print all available token
		for(; j < _NUM_OF_CMD; j++) {
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
void sigint(void* user_handle)
{
	print(user_handle,"^C catched!\n\r");
}
