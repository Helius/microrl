#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h> 
#include "microrl.h"

#define DBG(...) printf("\033[33m");printf(__VA_ARGS__);printf("\033[0m");

char ** compl_world [5];
char * world0 = "fuck";
char * world1 = "new";
char * world2 = "help";
char * world3 = "list";

//*****************************************************************************
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
int execute (int argc, const char * const * tkn_arr)
{
	int i = 0;
	DBG ("execute:\n");
	while (i < argc) {
		if (strcmp (tkn_arr[i], "help") == 0) {
			print ("microrl library v 1.0\n");
			print_help ();
			return 1;
		} else {
			DBG ("[%s] ", tkn_arr[i]);
			print ("ERROR: Command '");
			print (tkn_arr[i]);
			print ("' Not found.\n");
//			return 1; //TODO:remove it, need to return 1 if command recognize
		}
		i++;
	}
	return 0;
}

//*****************************************************************************
char ** complit (int argc, const char * const * tkn_arr)
{
	DBG ("\ncomplite for:\n");
	for (int i = 0; i < argc; i++) {
		DBG (" [%s]", tkn_arr[i]);
	}
	DBG ("\n");
	compl_world [0] = world0;
	compl_world [1] = world1;
	compl_world [2] = world2;
	compl_world [3] = world3;
	compl_world [4] = NULL;
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
	// microrl object and pointer on it
	microrl_t rl;
	microrl_t * prl = &rl;
	microrl_init (prl, print);
	microrl_set_execute_callback (prl, execute);
	microrl_set_complite_callback (prl, complit);
	
	while (1) {
		microrl_insert_char (prl, get_char());
	}
	return 0;
}

