#include "../../src/config.h"
#include <string.h>
#include <stdlib.h>
#include <xc.h>
#include "config.h"
#include <stdarg.h>
#include "uart.h"

/*
PIC platform specific implementation routines (for PIC18F4520 chip)
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
  OSCTUNE = 0x00;

  //set up internal clock to 8MHz, not used external clock at 8Mhz chosen
  OSCCON  = 0x70;

  //configure PORT D in output for led display
  PORTD = 0x0;
  TRISD = 0x0;

  //put RB0 and RB1 in output
  //put RB2 also in output
  //RB0 = _RESET
  //RB1 = SLEEP_RQ
  //RB2 = BATT_SENSOR_ENABLED  ===> not present for cmd
  LATB = 0x00;
  TRISB = 0xF8;

  uart_setup();


  INTCONbits.PEIE = 1; //activate peripherical interrupt
  INTCONbits.GIE = 1; //activate global interrupt
}

////*****************************************************************************
//void put_char (unsigned char ch)
//{
//		while (!( UCSRA & (1<<UDRE)));
//		UDR = (unsigned char) ch;
//}

//*****************************************************************************
void print (const char * str)
{
  int size = strlen(str);
  uart_write(size, str);
}

//*****************************************************************************
char get_char (void)
{
  BOOL hasChar;
  char c;
  do
  {
    hasChar = uart_read(&c, 1);
  }
  while (hasChar == FALSE);
  return c;
}

//*****************************************************************************
void print_help (void)
{
	print ("Use TAB key for completion\n\rCommand:\n\r");
	print ("\tclear               - clear screen\n\r");
	print ("\tset_port port pin   - set 1 port[pin] value, support only 'port_b' and 'port_d'\n\r");
	print ("\tclear_port port pin - set 0 port[pin] value, support only 'port_b' and 'port_d'\n\r");
}

//*****************************************************************************
void set_port_val (unsigned char * port, int pin, int val)
{
	if ((*port == PORTD) && (pin < 2) && (pin > 7)) {
		print ("only 2..7 pin available for PORTD\n\r");
		return;
	}

	if ((*port == PORTB) && (pin > 5)) {
		print ("only 0..5 pin available for PORTB\n\r");
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
int execute (int argc, const char * const * argv)
{
	int i = 0;
	// just iterate through argv word and compare it with your commands
	while (i < argc) {
		if (strcmp (argv[i], _CMD_HELP) == 0) {
			print ("microrl v");
			print (MICRORL_LIB_VER);
			print (" library PIC DEMO v");
			print (_AVR_DEMO_VER);
			print("\n\r");
			print_help ();        // print help
		} else if (strcmp (argv[i], _CMD_CLEAR) == 0) {
			print ("\033[2J");    // ESC seq for clear entire screen
			print ("\033[H");     // ESC seq for move cursor at left-top corner
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
					print ("only '");
					print (_SCMD_PB);
					print ("' and '");
					print (_SCMD_PD);
					print ("' support\n\r");
					return 1;
				}
				if (++i < argc) {
					pin = atoi (argv[i]);
					set_port_val (port, pin, val);
					return 0;
				} else {
					print ("specify pin number, use Tab\n\r");
					return 1;
				}
			} else {
					print ("specify port, use Tab\n\r");
				return 1;
			}
		} else {
			print ("command: '");
			print ((char*)argv[i]);
			print ("' Not found.\n\r");
		}
		i++;
	}
	return 0;
}

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet (int argc, const char * const * argv)
{
	int j = 0;

	compl_world [0] = NULL;

	// if there is token in cmdline
	if (argc == 1) {
		// get last entered token
		char * bit_str = (char*)argv [argc-1];
		// iterate through our available token and match it
		for (int i = 0; i < _NUM_OF_CMD; i++) {
			// if token is matched (text is part of our token starting from 0 char)
			if (strstr(keyworld [i], bit_str) == keyworld [i]) {
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
void sigint (void)
{
	print ("^C catched!\n\r");
}
