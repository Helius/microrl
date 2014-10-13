#ifndef _MICRORL_H_
#define _MICRORL_H_

#include "microrl_config.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

 /* define the Key codes */
#define KEY_NUL 0 /**< ^@ Null character */
#define KEY_SOH 1 /**< ^A Start of heading, = console interrupt */
#define KEY_STX 2 /**< ^B Start of text, maintenance mode on HP console */
#define KEY_ETX 3 /**< ^C End of text */
#define KEY_EOT 4 /**< ^D End of transmission, not the same as ETB */
#define KEY_ENQ 5 /**< ^E Enquiry, goes with ACK; old HP flow control */
#define KEY_ACK 6 /**< ^F Acknowledge, clears ENQ logon hand */
#define KEY_BEL 7 /**< ^G Bell, rings the bell... */
#define KEY_BS  8 /**< ^H Backspace, works on HP terminals/computers */
#define KEY_HT  9 /**< ^I Horizontal tab, move to next tab stop */
#define KEY_LF  10  /**< ^J Line Feed */
#define KEY_VT  11  /**< ^K Vertical tab */
#define KEY_FF  12  /**< ^L Form Feed, page eject */
#define KEY_CR  13  /**< ^M Carriage Return*/
#define KEY_SO  14  /**< ^N Shift Out, alternate character set */
#define KEY_SI  15  /**< ^O Shift In, resume defaultn character set */
#define KEY_DLE 16  /**< ^P Data link escape */
#define KEY_DC1 17  /**< ^Q XON, with XOFF to pause listings; "okay to send". */
#define KEY_DC2 18  /**< ^R Device control 2, block-mode flow control */
#define KEY_DC3 19  /**< ^S XOFF, with XON is TERM=18 flow control */
#define KEY_DC4 20  /**< ^T Device control 4 */
#define KEY_NAK 21  /**< ^U Negative acknowledge */
#define KEY_SYN 22  /**< ^V Synchronous idle */
#define KEY_ETB 23  /**< ^W End transmission block, not the same as EOT */
#define KEY_CAN 24  /**< ^X Cancel line, MPE echoes !!! */
#define KEY_EM  25  /**< ^Y End of medium, Control-Y interrupt */
#define KEY_SUB 26  /**< ^Z Substitute */
#define KEY_ESC 27  /**< ^[ Escape, next character is not echoed */
#define KEY_FS  28  /**< ^\ File separator */
#define KEY_GS  29  /**< ^] Group separator */
#define KEY_RS  30  /**< ^^ Record separator, block-mode terminator */
#define KEY_US  31  /**< ^_ Unit separator */

#define KEY_DEL 127 /**< Delete (not a real control character...) */

#define IS_CONTROL_CHAR(x) ((x)<=31)

// direction of history navigation
#define _HIST_UP   0
#define _HIST_DOWN 1
// esc seq internal codes
enum {
	_ESC_BRACKET = 1,
	_ESC_HOME,
	_ESC_END,
};

#ifdef _USE_HISTORY
// history struct, contain internal variable
// history store in static ring buffer for memory saving
typedef struct {
	char ring_buf [_RING_HISTORY_LEN];
	int begin;
	int end;
	int cur;
} ring_history_t;
#endif

// Function pointer
typedef unsigned int (*ptExecFunc)(void* user_handle, int argc, const char* const* argv );
typedef char** (*ptGetCompletionFunc)(void* user_handle, int argc, const char* const* argv );
typedef void (*ptPrintFunc)(void* user_handle, const char* str);
typedef void (*ptSigintFunc)(void* user_handle);

// microrl struct, contain internal library data
typedef struct {
	void* user_handle;
#ifdef _USE_ESC_SEQ
	char escape_seq;
	char escape;
#endif
#if (defined(_ENDL_CRLF) || defined(_ENDL_LFCR))
	char tmpch;
#endif
#ifdef _USE_HISTORY
	ring_history_t ring_hist;          // history object
#endif
	char* prompt_str;                  // pointer to prompt string
	char cmdline [_COMMAND_LINE_LEN];  // cmdline buffer
	int cmdlen;                        // last position in command line
	int cursor;                        // input cursor
	ptExecFunc execute; // ptr to 'execute' callback
	ptGetCompletionFunc get_completion; // ptr to 'completion' callback
	ptPrintFunc print; // ptr to 'print' callback
#ifdef _USE_CTRL_C
	ptSigintFunc sigint; // ptr to 'sigint' callback
#endif
} microrl_t;

// init internal data, calls once at start up
void microrl_init(microrl_t* pThis, void* user_handle, ptPrintFunc print);

// set prompt.
void microrl_set_prompt(microrl_t* pThis, const char* prompt);

// set echo mode (true/false), using for disabling echo for password input
// echo mode will enabled after user press Enter.
void microrl_set_echo(int);

// set pointer to callback complition func, that called when user press 'Tab'
// callback func description:
//   param: argc - argument count, argv - pointer array to token string
//   must return NULL-terminated string, contain complite variant splitted by 'Whitespace'
//   If complite token found, it's must contain only one token to be complitted
//   Empty string if complite not found, and multiple string if there are some token
void microrl_set_complete_callback(microrl_t* pThis, ptGetCompletionFunc get_completion);

// pointer to callback func, that called when user press 'Enter'
// execute func param: argc - argument count, argv - pointer array to token string
void microrl_set_execute_callback(microrl_t* pThis, ptExecFunc execute);

// set callback for Ctrl+C terminal signal
#ifdef _USE_CTRL_C
void microrl_set_sigint_callback(microrl_t* pThis, ptSigintFunc sigintf);
#endif

// insert char to cmdline (for example call in usart RX interrupt)
void microrl_insert_char (microrl_t* pThis, int ch);

#endif
