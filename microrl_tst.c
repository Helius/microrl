#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h> 
#include "microrl.h"

#define DBG(...) printf("\033[33m");printf(__VA_ARGS__);printf("\033[0m");

#define _NUM_OF_CMD 5
char * keyworld [] = {"fuck","help","list","function", "lisp"};
char ** compl_world [_NUM_OF_CMD + 1];

//*****************************************************************************
// print callback for microrl library
void print (char * str)
{
	fprintf (stdout, "%s", str);
}

//*****************************************************************************
void print_help ()
{
	print ("Use TAB key for completion\nCommand:\n\thelp - this message\n\tlist - list all commands in tree\n");
}

//*****************************************************************************
// execute callback for microrl library
int execute (int argc, const char * const * tkn_arr)
{
	int i = 0;
	while (i < argc) {
		if (strcmp (tkn_arr[i], "help") == 0) {
			print ("microrl library based shell v 1.0\n");
			print_help ();
			return 1;
		} else {
			print ("command: '");
			print (tkn_arr[i]);
			print ("' Not found.\n");
		}
		i++;
	}
	return 0;
}

//*****************************************************************************
// completion callback for microrl library
char ** complet (int argc, const char * const * argv)
{
	int j = 0;
	compl_world [0] = NULL;
	if (argc > 0) {
		char * bit = argv [argc-1];
		for (int i = 0; i < _NUM_OF_CMD; i++) {
			if (strstr(keyworld [i], bit) == keyworld [i]) {
				compl_world [j++] = keyworld [i];
			}
		}
	} else {
		for (; j < _NUM_OF_CMD; j++) {
			compl_world[j] = keyworld [j];
		}
	}
	compl_world [j] = NULL;
	return compl_world;
}

//*****************************************************************************
// get char user pressed, no waiting Enter input
char get_char (void)
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

//*****************************************************************************
int main (int argc, char ** argv)
{
	// create microrl object and pointer on it
	microrl_t rl;
	microrl_t * prl = &rl;
	// call init with ptr to microrl instance and print callback
	microrl_init (prl, print);
	// set callback for execute
	microrl_set_execute_callback (prl, execute);
	// set callback for completion
	microrl_set_complite_callback (prl, complet);
	
	while (1) {
		// put received char from stdin to microrl lib
		microrl_insert_char (prl, get_char());
	}
	return 0;
}

