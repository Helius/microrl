#include <stdio.h>
#include <string.h>
#include "../src/microrl.h"
#include "example_misc.h"

// create microrl object and pointer on it
microrl_t rl;
microrl_t * prl = &rl;
void* usr_handle = NULL;

//*****************************************************************************
int main (void/*int argc, char ** argv*/)
{
	init(usr_handle);
	// call init with ptr to microrl instance and print callback
	microrl_init(prl, usr_handle, print);
	// set callback for execute
	microrl_set_execute_callback (prl, execute);

#ifdef _USE_COMPLETE
	// set callback for completion
	microrl_set_complete_callback (prl, complet);
#endif
	// set callback for Ctrl+C
	microrl_set_sigint_callback (prl, sigint);
	while (1) {
		// put received char from stdin to microrl lib
		microrl_insert_char(prl, get_char(usr_handle));
	}
	return 0;
}

