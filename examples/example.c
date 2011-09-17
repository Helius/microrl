#include <stdio.h>
#include <string.h>
#include "../src/microrl.h"
#include "example_misc.h"



//*****************************************************************************
int main (void/*int argc, char ** argv*/)
{
	init ();
	
	// create microrl object and pointer on it
	microrl_t rl;
	microrl_t * prl = &rl;
	// call init with ptr to microrl instance and print callback
	microrl_init (prl, print);
	// set callback for execute
	microrl_set_execute_callback (prl, execute);
	// set callback for completion
#ifdef _USE_COMPLETE
	microrl_set_complite_callback (prl, complet);
#endif
	
	while (1) {
		// put received char from stdin to microrl lib
		microrl_insert_char (prl, get_char());
	}
	return 0;
}

