#ifndef _MICRORL_MISC_H_
#define _MICRORL_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
Platform independent interface for implementing some specific function
for AVR, linux PC or ARM
*/

// init platform
void init (void);

// print to stream callback
void print (microrl_t * pThis, const char * str);

// get_char from stream
char get_char (void);

// execute callback
int execute (microrl_t * pThis, int argc, const char * const * argv);

// completion callback
char ** complet (microrl_t * pThis, int argc, const char * const * argv);

// ctrl+c callback
void sigint (microrl_t * pThis);

#ifdef __cplusplus
}
#endif

#endif
