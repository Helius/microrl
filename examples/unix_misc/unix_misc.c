#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> 
#include <string.h>
#include "../../src/microrl.h"

//*****************************************************************************
//dummy function, no need on linux-PC
void init (void){
};

//*****************************************************************************
// print callback for microrl library
void print (void * pThis, const char * str)
{
	fprintf (stdout, "%s", str);
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


// definition commands word
#define _CMD_HELP  "help"
#define _CMD_CLEAR "clear"
#define _CMD_LIST  "list"
#define _CMD_LISP  "lisp" // for demonstration completion on 'l + <TAB>'
#define _CMD_NAME  "name"
#define _CMD_VER   "version"
// sub commands for version command
	#define _SCMD_MRL  "microrl"
	#define _SCMD_DEMO "demo"

#define _NUM_OF_CMD 6
#define _NUM_OF_VER_SCMD 2

//available  commands
char * keyworld [] = {_CMD_HELP, _CMD_CLEAR, _CMD_LIST, _CMD_NAME, _CMD_VER, _CMD_LISP};
// version subcommands
char * ver_keyworld [] = {_SCMD_MRL, _SCMD_DEMO};

// array for comletion
char * compl_world [_NUM_OF_CMD + 1];

// 'name' var for store some string
#define _NAME_LEN 8
char name [_NAME_LEN];
int val;


//*****************************************************************************
void print_help (void * pThis)
{
	print (pThis, "Use TAB key for completion\n\rCommand:\n\r");
	print (pThis, "\tversion {microrl | demo} - print version of microrl lib or version of this demo src\n\r");
	print (pThis, "\thelp  - this message\n\r");
	print (pThis, "\tclear - clear screen\n\r");
	print (pThis, "\tlist  - list all commands in tree\n\r");
	print (pThis, "\tname [string] - print 'name' value if no 'string', set name value to 'string' if 'string' present\n\r");
	print (pThis, "\tlisp - dummy command for demonstation auto-completion, while inputed 'l+<TAB>'\n\r");
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
			print (pThis, "microrl library based shell v 1.0\n\r");
			print_help (pThis);        // print help
		} else if (strcmp (argv[i], _CMD_NAME) == 0) {
			if ((++i) < argc) { // if value preset
				if (strlen (argv[i]) < _NAME_LEN) {
					strcpy (name, argv[i]);
				} else {
					print (pThis, "name value too long!\n\r");
				}
			} else {
				print (pThis, name);
				print (pThis, "\n\r");
			}
		} else if (strcmp (argv[i], _CMD_VER) == 0) {
			if (++i < argc) {
				if (strcmp (argv[i], _SCMD_DEMO) == 0) {
					print (pThis, "demo v 1.0\n\r");
				} else if (strcmp (argv[i], _SCMD_MRL) == 0) {
					print (pThis, "microrl v 1.2\n\r");
				} else {
					print (pThis, (char*)argv[i]);
					print (pThis, " wrong argument, see help\n\r");
				}
			} else {
				print (pThis, "version needs 1 parametr, see help\n\r");
			}
		} else if (strcmp (argv[i], _CMD_CLEAR) == 0) {
			print (pThis, "\033[2J");    // ESC seq for clear entire screen
			print (pThis, "\033[H");     // ESC seq for move cursor at left-top corner
		} else if (strcmp (argv[i], _CMD_LIST) == 0) {
			print (pThis, "available command:\n");// print all command per line
			for (int i = 0; i < _NUM_OF_CMD; i++) {
				print (pThis, "\t");
				print (pThis, keyworld[i]);
				print (pThis, "\n\r");
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
	}	else if ((argc > 1) && (strcmp (argv[0], _CMD_VER)==0)) { // if command needs subcommands
		// iterate through subcommand for command _CMD_VER array
		for (int i = 0; i < _NUM_OF_VER_SCMD; i++) {
			if (strstr (ver_keyworld [i], argv [argc-1]) == ver_keyworld [i]) {
				compl_world [j++] = ver_keyworld [i];
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
