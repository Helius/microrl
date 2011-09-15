#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> 

//*****************************************************************************
//dummy function, no need on linux-PC
void init (void){};

//*****************************************************************************
// print callback for microrl library
void print (char * str)
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
