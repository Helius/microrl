#ifndef _MICRORL_MISC_H_
#define _MICRORL_MISC_H_

/*
Platform independent interface for implementing some specific function
for AVR, linux PC or ARM
*/

// init platform
void init (void);

// print to stream callback
void print (void * pThis, const char * str);

// get_char from stream
char get_char (void);

// execute callback
int execute (void * pThis, int argc, const char * const * argv);

// completion callback
char ** complet (void * pThis, int argc, const char * const * argv);

// ctrl+c callback
void sigint (void * pThis, void);

#endif
