/*
Microrl library config files
Author: Eugene Samoylov aka Helius (ghelius@gmail.com)
Modified configuration for HydraBus/HydraNFC (bvernoux@hydrabus.com)
*/
#ifndef _MICRORL_CONFIG_H_
#define _MICRORL_CONFIG_H_

#define MICRORL_LIB_VER "1.5.2"

/*********** CONFIG SECTION **************/
/*
Command line length, define cmdline buffer size. Set max number of chars + 1,
because last byte of buffer need to contain '\0' - NULL terminator, and 
not use for storing inputed char.
If user input chars more then it parameters-1, chars not added to command line.*/
#define _COMMAND_LINE_LEN (1+100) // for 32 chars

/*
 * Maximum number of command tokens. If the number of tokens on a command line
 * exceeds this value an error message will be printed and the command line
 * will not be executed.
 * A token is a word separate by white space, for example a 3 token line:
 * "> set mode test"
 */
#define _MAX_COMMAND_TOKENS 50

#define _PROMPT_TXT ">"

// Prompt START & END for green color (included automatically)
#define _PROMPT_START "\033[32m"
#define _PROMPT_END "\033[0m"
/*
Define you prompt string here. You can use colors escape code, for highlight you prompt,
for example this prompt will green color (if you terminal supports color)
*/
#define _PROMPT_DEFAULT _PROMPT_TXT	/* shall not include _PROMPT_START & _PROMPT_END */

/*Define it, if you wanna use completion functional, also set completion callback in you code,
now if user press TAB calls 'completion' callback. If you no need it, you can just set
NULL to callback ptr and do not use it, but for memory saving tune, 
if you are not going to use it - disable this define.*/
#define _USE_COMPLETE

/*Define it, if you wanna use history. It s work's like bash history, and
set stored value to cmdline, if UP and DOWN key pressed. Using history add
memory consuming, depends from _RING_HISTORY_LEN parameter */
#define _USE_HISTORY

/*
History ring buffer length, define static buffer size.
For saving memory, each entered cmdline store to history in ring buffer,
so we can not say, how many line we can store, it depends from cmdline len,
but memory using more effective. We not prefer dynamic memory allocation for
small and embedded devices. Overhead is 2 char on each saved line*/
#define _RING_HISTORY_LEN 64

/*
Enable Handling terminal ESC sequence. If disabling, then cursor arrow, HOME, END will not work,
use Ctrl+A(B,F,P,N,A,E,H,K,U,C) see README, but decrease code memory.*/
#define _USE_ESC_SEQ

/*
Use snprintf from you standard complier library, but it gives some overhead.
If not defined, use my own u16int_to_str variant, it's save about 800 byte of code size
on AVR (avr-gcc build).
Try to build with and without, and compare total code size for tune library.
*/
#define _USE_LIBC_STDIO

/*
Enable 'interrupt signal' callback, if user press Ctrl+C */
#define _USE_CTRL_C

/*
Print prompt at 'microrl_init', if enable, prompt will print at startup, 
otherwise first prompt will print after first press Enter in terminal
NOTE!: Enable it, if you call 'microrl_init' after your communication subsystem 
already initialize and ready to print message */
#undef _ENABLE_INIT_PROMPT

/*
New line symbol */
//#define _ENDL_LF
//#define _ENDL_CRLF
#define _ENDL_CR

#if defined(_ENDL_CR)
#define ENDL "\r\n"
#elif defined(_ENDL_CRLF)
#define ENDL "\r\n"
#elif defined(_ENDL_LF)
#define ENDL "\n"
#elif defined(_ENDL_LFCR)
#define ENDL "\n\r"
#else
#error "You must define new line symbol."
#endif

/********** END CONFIG SECTION ************/

#if _RING_HISTORY_LEN > 256
#error "This history implementation (ring buffer with 1 byte iterator) allow 256 byte buffer size maximum"
#endif

#endif
