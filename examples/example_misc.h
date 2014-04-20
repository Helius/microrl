#ifndef _MICRORL_MISC_H_
#define _MICRORL_MISC_H_

/*
Platform independent interface for implementing some specific function
for AVR, linux PC or ARM
*/

// init platform
void init(void* user_handle);

// print to stream callback
void print(void* user_handle, const char * str);

// get_char from stream
char get_char(void* user_handle) ;

// execute callback
int execute(void* user_handle, int argc, const char * const * argv);

// completion callback
char ** complet(void* user_handle, int argc, const char * const * argv);

// ctrl+c callback
void sigint(void* user_handle);

#endif
