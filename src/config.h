/*
Microrl library config files
Autor: Eugene Samoylov aka Helius (ghelius@gmail.com)
*/
#ifndef _MICRORL_CONFIG_H_
#define _MICRORL_CONFIG_H_

#define MICRORL_LIB_VER "1.3"

/*********** CONFIG SECTION **************/
/*
Command line length, define cmdline buffer size. Set max number of chars + 1,
because last byte of buffer need to contain '\0' - NULL terminator, and 
not use for storing inputed char.
If user input chars more then it parametrs-1, chars not added to command line.*/
#define _COMMAND_LINE_LEN (1+100)									// for 32 chars

/*
Command token number, define max token it command line, if number of token 
typed in command line exceed this value, then prints message about it and
command line not to be parced and 'execute' callback will not calls.
Token is word separate by white space, for example 3 token line:
"IRin> set mode test" */
#define _COMMAND_TOKEN_NMB 8

/*
Define you prompt string here. You can use colors escape code, for highlight you prompt,
for example this prompt will green color (if you terminal supports color)*/
//#define _PROMPT_DEFAUTL "\033[32mIRin >\033[0m "	// green color
#define _PROMPT_DEFAUTL "\033[32mIRin >\033[0m "	// green color
//#define _PROMPT_DEFAUTL "IRin > "

/*
Define prompt text (without ESC sequence, only text) prompt length, it needs because if you use
ESC sequence, it's not possible detect only text length*/
#define _PROMPT_LEN       7

/*Define it, if you wanna use completion functional, also set completion callback in you code,
now if user press TAB calls 'copmlitetion' callback. If you no need it, you can just set 
NULL to callback ptr and do not use it, but for memory saving tune, 
if you are not going to use it - disable this define.*/
#define _USE_COMPLETE

/*Define it, if you wanna use history. It s work's like bash history, and
set stored value to cmdline, if UP and DOWN key pressed. Using history add
memory consuming, depends from _RING_HISTORY_LEN parametr */
#define _USE_HISTORY

/*
History ring buffer length, define static buffer size.
For saving memory, each entered cmdline store to history in ring buffer,
so we can not say, how many line we can store, it depends from cmdline len,
but memory using more effective. We not prefer dinamic memory allocation for
small and embedded devices. Overhead is 2 char on each saved line*/
#define _RING_HISTORY_LEN 64

/*
Enable Handling terminal ESC sequence. If disabling, then cursor arrow, HOME, END will not work,
use Ctrl+A(B,F,P,N,A,E,H,K,U,C) see README, but decrease code memory.*/
#define _USE_ESC_SEQ

/*
Enable 'interrupt signal' callback, if user press Ctrl+C */
#define _USE_CTLR_C

/*
Print prompt at 'microrl_init', if enable, prompt will print at startup, 
otherwise first prompt will print after first press Enter in terminal
NOTE!: Enable it, if you call 'microrl_init' after your communication subsystem 
already initialize and ready to print message */
//#define _ENABLE_INIT_PROMPT

/********** END CONFIG SECTION ************/


#if _RING_HISTORY_LEN > 256
#error "This history implementation (ring buffer with 1 byte iterator) allow 256 byte buffer size maximum"
#endif

#endif
