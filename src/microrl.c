/*
Author: Samoylov Eugene aka Helius (ghelius@gmail.com)
BUGS and TODO:
-- add echo_off feature
-- rewrite history for use more than 256 byte buffer
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "microrl.h"

//#define DBG(...) fprintf(stderr, "\033[33m");fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\033[0m");

char * prompt_default = _PROMPT_DEFAUTL;

#ifdef _USE_HISTORY

#ifdef _HISTORY_DEBUG
//*****************************************************************************
// print buffer content on screen
static void print_hist (ring_history_t * this)
{
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == this->begin)
			printf ("b");
		else 
			printf (" ");
	}
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (isalpha(this->ring_buf[i]))
			printf ("%c", this->ring_buf[i]);
		else 
			printf ("%d", this->ring_buf[i]);
	}
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == this->end)
			printf ("e");
		else 
			printf (" ");
	}
	printf ("\n");
}
#endif

//*****************************************************************************
// remove older message from ring buffer
static void hist_erase_older (ring_history_t * this)
{
	int new_pos = this->begin + this->ring_buf [this->begin] + 1;
	if (new_pos >= _RING_HISTORY_LEN)
		new_pos = new_pos - _RING_HISTORY_LEN;
	
	this->begin = new_pos;
}

//*****************************************************************************
// check space for new line, remove older while not space
static int hist_is_space_for_new (ring_history_t * this, int len)
{
	if (this->ring_buf [this->begin] == 0)
		return true;
	if (this->end >= this->begin) {
		if (_RING_HISTORY_LEN - this->end + this->begin - 1 > len)
			return true;
	}	else {
		if (this->begin - this->end - 1> len)
			return true;
	}
	return false;
}

//*****************************************************************************
// put line to ring buffer
static void hist_save_line (ring_history_t * this, char * line, int len)
{
	if (len > _RING_HISTORY_LEN - 2)
		return;
	while (!hist_is_space_for_new (this, len)) {
		hist_erase_older (this);
	}
	// if it's first line
	if (this->ring_buf [this->begin] == 0) 
		this->ring_buf [this->begin] = len;
	
	// store line
	if (len < _RING_HISTORY_LEN-this->end-1)
		memcpy (this->ring_buf + this->end + 1, line, len);
	else {
		int part_len = _RING_HISTORY_LEN-this->end-1;
		memcpy (this->ring_buf + this->end + 1, line, part_len);
		memcpy (this->ring_buf, line + part_len, len - part_len);
	}
	this->ring_buf [this->end] = len;
	this->end = this->end + len + 1;
	if (this->end >= _RING_HISTORY_LEN)
		this->end -= _RING_HISTORY_LEN;
	this->ring_buf [this->end] = 0;
	this->cur = 0;
#ifdef _HISTORY_DEBUG
	print_hist (this);
#endif
}

//*****************************************************************************
// copy saved line to 'line' and return size of line
static int hist_restore_line (ring_history_t * this, char * line, int dir)
{
	int cnt = 0;
	// count history record	
	int header = this->begin;
	while (this->ring_buf [header] != 0) {
		header += this->ring_buf [header] + 1;
		if (header >= _RING_HISTORY_LEN)
			header -= _RING_HISTORY_LEN; 
		cnt++;
	}

	if (dir == _HIST_UP) {
		if (cnt >= this->cur) {
			int header = this->begin;
			int j = 0;
			// found record for 'this->cur' index
			while ((this->ring_buf [header] != 0) && (cnt - j -1 != this->cur)) {
				header += this->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			if (this->ring_buf[header]) {
					this->cur++;
				// obtain saved line
				if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
					memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
				} else {
					int part0 = _RING_HISTORY_LEN - header - 1;
					memcpy (line, this->ring_buf + header + 1, part0);
					memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
				}
				return this->ring_buf[header];
			}
		}
	} else {
		if (this->cur > 0) {
				this->cur--;
			int header = this->begin;
			int j = 0;

			while ((this->ring_buf [header] != 0) && (cnt - j != this->cur)) {
				header += this->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
				memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
			} else {
				int part0 = _RING_HISTORY_LEN - header - 1;
				memcpy (line, this->ring_buf + header + 1, part0);
				memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
			}
			return this->ring_buf[header];
		}
	}
	return 0;
}
#endif








//*****************************************************************************
// split cmdline to tkn array and return nmb of token
static int split (microrl_t * this, int limit)
{
	int i = 0;
	int ind = 0;
	while (1) {
		// go to the first whitespace (zerro for us)
		while ((this->cmdline [ind] == '\0') && (ind < limit)) {
			ind++;
		}
		if (!(ind < limit)) return i;
		this->tkn_arr[i++] = this->cmdline + ind;
		if (i >= _COMMAND_TOKEN_NMB) {
			return -1;
		}
		// go to the first NOT whitespace (not zerro for us)
		while ((this->cmdline [ind] != '\0') && (ind < limit)) {
			ind++;
		}
		if (!(ind < limit)) return i;
	}
	return i;
}


//*****************************************************************************
inline static void print_prompt (microrl_t * this)
{
	this->print (this->prompt_str);
}

//*****************************************************************************
inline static void terminal_backspace (microrl_t * this)
{
		this->print ("\033[D \033[D");
}

//*****************************************************************************
inline static void terminal_newline (microrl_t * this)
{
	this->print ("\n\r");
}

//*****************************************************************************
// set cursor at position from begin cmdline (after prompt) + offset
static void terminal_move_cursor (microrl_t * this, int offset)
{
	char str[16] = {0,};
	if (offset > 0) {
		snprintf (str, 16, "\033[%dC", offset);
	} else if (offset < 0) {
		snprintf (str, 16, "\033[%dD", -(offset));
	}
	this->print (str);
}

//*****************************************************************************
static void terminal_reset_cursor (microrl_t * this)
{
	char str[16];
	snprintf (str, 16, "\033[%dD\033[%dC", _COMMAND_LINE_LEN + _PROMPT_LEN + 2,
																					_PROMPT_LEN);
	this->print (str);
}

//*****************************************************************************
// print cmdline to screen, replace '\0' to wihitespace 
static void terminal_print_line (microrl_t * this, int pos, int cursor)
{
	this->print ("\033[K");    // delete all from cursor to end

	char nch [] = {0,0};
	for (int i = pos; i < this->cmdlen; i++) {
		nch [0] = this->cmdline [i];
		if (nch[0] == '\0')
			nch[0] = ' ';
		this->print (nch);
	}
	
	terminal_reset_cursor (this);
	terminal_move_cursor (this, cursor);
}

//*****************************************************************************
void microrl_init (microrl_t * this, void (*print) (const char *)) 
{
	memset(this->cmdline, 0, _COMMAND_LINE_LEN);
#ifdef _USE_HISTORY
	memset(this->ring_hist.ring_buf, 0, _RING_HISTORY_LEN);
	this->ring_hist.begin = 0;
	this->ring_hist.end = 0;
	this->ring_hist.cur = 0;
#endif
	this->cmdlen =0;
	this->cursor = 0;
	this->execute = NULL;
	this->get_completion = NULL;
#ifdef _USE_CTLR_C
	this->sigint = NULL;
#endif
	this->prompt_str = prompt_default;
	this->print = print;
#ifdef _ENABLE_INIT_PROMPT
	print_prompt (this);
#endif
}

//*****************************************************************************
void microrl_set_complite_callback (microrl_t * this, char ** (*get_completion)(int, const char* const*))
{
	this->get_completion = get_completion;
}

//*****************************************************************************
void microrl_set_execute_callback (microrl_t * this, int (*execute)(int, const char* const*))
{
	this->execute = execute;
}
#ifdef _USE_CTLR_C
//*****************************************************************************
void microrl_set_sigint_callback (microrl_t * this, void (*sigintf)(void))
{
	this->sigint = sigintf;
}
#endif

#ifdef _USE_ESC_SEQ
static void hist_search (microrl_t * this, int dir)
{
int len = hist_restore_line (&this->ring_hist, this->cmdline, dir);
if (len) {
	this->cursor = this->cmdlen = len;
	terminal_reset_cursor (this);
	terminal_print_line (this, 0, this->cursor);
}
}

//*****************************************************************************
// handling escape sequences
static int escape_process (microrl_t * this, char ch)
{
	static int seq = 0;

	if (ch == '[') {
		seq = _ESC_BRACKET;	
	} else if (seq == _ESC_BRACKET) {
		if (ch == 'A') {
#ifdef _USE_HISTORY
			hist_search (this, _HIST_UP);
#endif
			return 1;
		} else if (ch == 'B') {
#ifdef _USE_HISTORY
			hist_search (this, _HIST_DOWN);
#endif
			return 1;
		} else if (ch == 'C') {
			if (this->cursor < this->cmdlen) {
				terminal_move_cursor (this, 1);
				this->cursor++;
			}
			return 1;
		} else if (ch == 'D') {
			if (this->cursor > 0) {
				terminal_move_cursor (this, -1);
				this->cursor--;
			}
			return 1;
		} else if (ch == '7') {
			seq = _ESC_HOME;
			return 0;
		} else if (ch == '8') {
			seq = _ESC_END;
			return 0;
		} 
	} else if (ch == '~') {
			if (seq == _ESC_HOME) {
				terminal_reset_cursor (this);
				this->cursor = 0;
				return 1;
			} else if (seq == _ESC_END) {
				terminal_move_cursor (this, this->cmdlen-this->cursor);
				this->cursor = this->cmdlen;
				return 1;
			}
		
	}
	return 0;
}
#endif

//*****************************************************************************
// insert len char of text at cursor position
static int microrl_insert_text (microrl_t * this, char * text, int len)
{
	if (this->cmdlen + len < _COMMAND_LINE_LEN) {
		memmove (this->cmdline + this->cursor + len,
						 this->cmdline + this->cursor,
						 this->cmdlen - this->cursor);
		for (int i = 0; i < len; i++) {
			this->cmdline [this->cursor + i] = text [i];
			if (this->cmdline [this->cursor + i] == ' ') {
				this->cmdline [this->cursor + i] = 0;
			}
		}
		this->cursor += len;
		this->cmdlen += len;
		this->cmdline [this->cmdlen] = '\0';
		return true;
	}
	return false;
}

//*****************************************************************************
// remove one char at cursor
static void microrl_backspace (microrl_t * this)
{
	if (this->cursor > 0) {
		terminal_backspace (this);
		memmove (this->cmdline + this->cursor-1,
						 this->cmdline + this->cursor,
						 this->cmdlen-this->cursor+1);
		this->cursor--;
		this->cmdline [this->cmdlen] = '\0';
		this->cmdlen--;
	}
}


#ifdef _USE_COMPLETE

//*****************************************************************************
static int common_len (char ** arr)
{
	int len = 0;
	int i = 1;
	while (1) {
		while (arr[i]!=NULL) {
			if ((arr[i][len] != arr[i-1][len]) || 
					(arr[i][len] == '\0') || 
					(arr[i-1][len]=='\0')) 
				return len;
			len++;
		}
		i++;
	}
	return 0;
}

//*****************************************************************************
static void microrl_get_complite (microrl_t * this) 
{
	char ** compl_token; 
	
	if (this->get_completion == NULL) // callback was not set
		return;
	
	int status = split (this, this->cursor);
	if (this->cmdline[this->cursor-1] == '\0')
		this->tkn_arr[status++] = "";
	compl_token = this->get_completion (status, this->tkn_arr);
	if (compl_token[0] != NULL) {
		int i = 0;
		int len;

		if (compl_token[1] == NULL) {
			len = strlen (compl_token[0]);
		} else {
			len = common_len (compl_token);
			terminal_newline (this);
			while (compl_token [i] != NULL) {
				this->print (compl_token[i]);
				this->print (" ");
				i++;
			}
			terminal_newline (this);
			print_prompt (this);
		}
		
		if (len) {
			microrl_insert_text (this, compl_token[0] + strlen(this->tkn_arr[status-1]), 
																	len - strlen(this->tkn_arr[status-1]));
			if (compl_token[1] == NULL) 
				microrl_insert_text (this, " ", 1);
		}
		terminal_reset_cursor (this);
		terminal_print_line (this, 0, this->cursor);
	} 
}
#endif

//*****************************************************************************
void microrl_insert_char (microrl_t * this, int ch)
{
	int status;
	
#ifdef _USE_ESC_SEQ
	static int escape = false;
	
	if (escape) {
		if (escape_process(this, ch))
			escape = 0;
	} else {
#endif
		switch (ch) {
			//-----------------------------------------------------
			case KEY_CR:
			case KEY_LF:
				terminal_newline (this);
#ifdef _USE_HISTORY
				if (this->cmdlen > 0)
					hist_save_line (&this->ring_hist, this->cmdline, this->cmdlen);
#endif
				status = split (this, this->cmdlen);
				if (status == -1)
//					this->print ("ERROR: Max token amount exseed\n");
					this->print ("ERROR:tokens too much\n\r");
				if ((status > 0) && (this->execute != NULL)) 
					this->execute (status, this->tkn_arr);
				print_prompt (this);
				this->cmdlen = 0;
				this->cursor = 0;
				memset(this->cmdline, 0, _COMMAND_LINE_LEN);
#ifdef _USE_HISTORY
				this->ring_hist.cur = 0;
#endif
			
			break;
			//-----------------------------------------------------
#ifdef _USE_COMPLETE
			case KEY_HT:
				microrl_get_complite (this);
			break;
#endif
			//-----------------------------------------------------
			case KEY_ESC:
#ifdef _USE_ESC_SEQ
				escape = 1;
#endif
			break;
			//-----------------------------------------------------
			case KEY_NAK: // ^U
					while (this->cursor > 0) {
					microrl_backspace (this);
				}
				terminal_print_line (this, 0, this->cursor);
			break;
			//-----------------------------------------------------
			case KEY_VT:  // ^K
				this->print ("\033[K");
				this->cmdlen = this->cursor;
			break;
			//-----------------------------------------------------
			case KEY_ENQ: // ^E
				terminal_move_cursor (this, this->cmdlen-this->cursor);
				this->cursor = this->cmdlen;
			break;
			//-----------------------------------------------------
			case KEY_SOH: // ^A
				terminal_reset_cursor (this);
				this->cursor = 0;
			break;
			//-----------------------------------------------------
			case KEY_ACK: // ^F
			if (this->cursor < this->cmdlen) {
				terminal_move_cursor (this, 1);
				this->cursor++;
			}
			break;
			//-----------------------------------------------------
			case KEY_STX: // ^B
			if (this->cursor) {
				terminal_move_cursor (this, -1);
				this->cursor--;
			}
			break;
			//-----------------------------------------------------
			case KEY_DLE: //^P
#ifdef _USE_HISTORY
			hist_search (this, _HIST_UP);
#endif
			break;
			//-----------------------------------------------------
			case KEY_SO: //^N
#ifdef _USE_HISTORY
			hist_search (this, _HIST_DOWN);
#endif
			break;
			//-----------------------------------------------------
			case KEY_DEL: // Backspace
			case KEY_BS: // ^U
				microrl_backspace (this);
				terminal_print_line (this, this->cursor, this->cursor);
			break;
#ifdef _USE_CTLR_C
			case KEY_ETX:
			if (this->sigint != NULL)
				this->sigint();
			break;
#endif
			//-----------------------------------------------------
			default:
			if ((ch == ' ') && (this->cmdlen == 0)) 
				break;
			if (microrl_insert_text (this, (char*)&ch, 1))
				terminal_print_line (this, this->cursor-1, this->cursor);
			
			break;
		}
#ifdef _USE_ESC_SEQ
	}
#endif
}
