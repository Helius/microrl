/*
Author: Samoylov Eugene aka Helius (ghelius@gmail.com)
BUGS and TODO:
-- rewrite history for use more than 256 byte buffer
*/
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "microrl.h"
#ifdef _USE_LIBC_STDIO
#include <stdio.h>
#endif

//#define DBG_TRACE
#ifdef DBG_TRACE
#include <stdio.h>
#define DBG(...) fprintf(stderr, "\033[33m");fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\033[0m");
#else
//#define DBG(...) 
#endif

char * prompt_default = _PROMPT_DEFAULT;

#ifdef _USE_HISTORY

#ifdef _HISTORY_DEBUG
//*****************************************************************************
// print buffer content on screen
static void print_hist (ring_history_t * pThis)
{
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == pThis->begin)
			printf ("b");
		else
			printf (" ");
	}
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (isalpha(pThis->ring_buf[i]))
			printf ("%c", pThis->ring_buf[i]);
		else
			printf ("%d", pThis->ring_buf[i]);
	}
	printf ("\n");
	for (int i = 0; i < _RING_HISTORY_LEN; i++) {
		if (i == pThis->end)
			printf ("e");
		else
			printf (" ");
	}
	printf ("\n");
}
#endif

//*****************************************************************************
// remove older message from ring buffer
static void hist_erase_older (ring_history_t * pThis)
{
	int new_pos = pThis->begin + pThis->ring_buf [pThis->begin] + 1;
	if (new_pos >= _RING_HISTORY_LEN)
		new_pos = new_pos - _RING_HISTORY_LEN;

	pThis->begin = new_pos;
}

//*****************************************************************************
// check space for new line, remove older while not space
static int hist_is_space_for_new (ring_history_t * pThis, int len)
{
	if (pThis->ring_buf [pThis->begin] == 0)
		return true;
	if (pThis->end >= pThis->begin) {
		if (_RING_HISTORY_LEN - pThis->end + pThis->begin - 1 > len)
			return true;
	}	else {
		if (pThis->begin - pThis->end - 1> len)
			return true;
	}
	return false;
}

//*****************************************************************************
// put line to ring buffer
static void hist_save_line (ring_history_t * pThis, char * line, int len)
{
	if (len > _RING_HISTORY_LEN - 2)
		return;
	while (!hist_is_space_for_new (pThis, len)) {
		hist_erase_older (pThis);
	}
	// if it's first line
	if (pThis->ring_buf [pThis->begin] == 0)
		pThis->ring_buf [pThis->begin] = len;

	// store line
	if (len < _RING_HISTORY_LEN-pThis->end-1)
		memcpy (pThis->ring_buf + pThis->end + 1, line, len);
	else {
		int part_len = _RING_HISTORY_LEN-pThis->end-1;
		memcpy (pThis->ring_buf + pThis->end + 1, line, part_len);
		memcpy (pThis->ring_buf, line + part_len, len - part_len);
	}
	pThis->ring_buf [pThis->end] = len;
	pThis->end = pThis->end + len + 1;
	if (pThis->end >= _RING_HISTORY_LEN)
		pThis->end -= _RING_HISTORY_LEN;
	pThis->ring_buf [pThis->end] = 0;
	pThis->cur = 0;
#ifdef _HISTORY_DEBUG
	print_hist (pThis);
#endif
}

//*****************************************************************************
// copy history line at index into line
static void hist_copy_line_from_index(ring_history_t * pThis, int index, char* line)
{
	int header = index;
	if (pThis->ring_buf [header] + header < _RING_HISTORY_LEN) {
		memcpy (line, pThis->ring_buf + header + 1, pThis->ring_buf[header]);
	} else {
		int part0 = _RING_HISTORY_LEN - header - 1;
		memcpy (line, pThis->ring_buf + header + 1, part0);
		memcpy (line + part0, pThis->ring_buf, pThis->ring_buf[header] - part0);
	}
}

//*****************************************************************************
// copy saved line to 'line' and return size of line
static int hist_restore_line (ring_history_t * pThis, char * line, int dir)
{
	int cnt = 0;
	// count history record
	int header = pThis->begin;
	while (pThis->ring_buf [header] != 0) {
		header += pThis->ring_buf [header] + 1;
		if (header >= _RING_HISTORY_LEN)
			header -= _RING_HISTORY_LEN;
		cnt++;
	}

	if (dir == _HIST_UP) {
		if (cnt >= pThis->cur) {
			int header = pThis->begin;
			int j = 0;
			// found record for 'pThis->cur' index
			while ((pThis->ring_buf [header] != 0) && (cnt - j -1 != pThis->cur)) {
				header += pThis->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			if (pThis->ring_buf[header]) {
					pThis->cur++;
				// obtain saved line
				if (pThis->ring_buf [header] + header < _RING_HISTORY_LEN) {
					memset (line, 0, _COMMAND_LINE_LEN);
					memcpy (line, pThis->ring_buf + header + 1, pThis->ring_buf[header]);
				} else {
					int part0 = _RING_HISTORY_LEN - header - 1;
					memset (line, 0, _COMMAND_LINE_LEN);
					memcpy (line, pThis->ring_buf + header + 1, part0);
					memcpy (line + part0, pThis->ring_buf, pThis->ring_buf[header] - part0);
				}
				return pThis->ring_buf[header];
			}
		}
	} else {
		if (pThis->cur > 0) {
				pThis->cur--;
			int header = pThis->begin;
			int j = 0;

			while ((pThis->ring_buf [header] != 0) && (cnt - j != pThis->cur)) {
				header += pThis->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			hist_copy_line_from_index(pThis, header, line);
			return pThis->ring_buf[header];
		} else {
			/* empty line */
			return 0;
		}
	}
	return -1;
}

static inline int microrl_get_histo_index(int index)
{
	int r;
	if (index >= _RING_HISTORY_LEN)
		r = index - _RING_HISTORY_LEN;
	else
		r = index;
	return r;
}

static void microrl_print_number(microrl_t * pThis, int nb)
{
	char str[10];
#ifdef _USE_LIBC_STDIO
	snprintf (str, 10, "%d", nb);
	pThis->print(pThis, str);
#else
	char tmp_str [10];
	char* pstr;

	if (nb == 0) {
		pThis->print(pThis, "0");
	} else {
	pstr = (char*) &str;
	int i = 0, j;

	while ((nb > 0) && (i < 9)) {
		tmp_str[i++] = (nb % 10) + '0';
		nb /=10;
	}
	tmp_str[i] = '\0';

	for (j = 0; j < i; ++j)
		*pstr++ = tmp_str [i-j-1];
	*pstr = '\0';
	pThis->print(pThis, str);
	}
#endif
}


static inline void microrl_display_histo_number(microrl_t* pThis, int number)
{
	pThis->print(pThis, " ");
	microrl_print_number(pThis, number);
	pThis->print(pThis, " ");
}

void microrl_print_history (microrl_t * pThis)
{
	ring_history_t* pHist = &pThis->ring_hist;
	int i;
	int j;
	int size;
	int hist_number;
	char temp[2];
	temp[1] = '\0';
	hist_number = 0;

	i = pHist->begin;
	while (i != pHist->end) {
		size = pHist->ring_buf[i];
		i = microrl_get_histo_index(i+1);
		microrl_display_histo_number(pThis, hist_number++);
		for(j = 0; j < size; j++) {
			temp[0] = pHist->ring_buf[microrl_get_histo_index(i+j)];
			if (temp[0] == '\0')
				temp[0] = ' ';
			pThis->print(pThis, temp);
		}

		pThis->print(pThis, ENDL);
		i = microrl_get_histo_index(i+size);
	}
}
#endif /* _USE_HISTORY */

#ifdef _USE_HISTORY_EXEC_PREV
static int hist_get_start_offset(ring_history_t* pHist, int hist_index)
{
	int i;
	int size;
	int hist_number;
	hist_number = 0;
	i = pHist->begin;
	while (i != pHist->end) {
		if (hist_number == hist_index)
			return i;

		size = pHist->ring_buf[i];
		i = microrl_get_histo_index(i+ 1 + size);
		hist_number++;
	}
	return -1;
}
#endif /* _USE_HISTORY_EXEC_PREV */


#ifdef _USE_QUOTING
//*****************************************************************************
// restore end quote marks in cmdline
static void restore (microrl_t * pThis)
{
	int iq;
	for (iq = 0; iq < _QUOTED_TOKEN_NMB; ++iq) {
		if (pThis->quotes[iq].end == 0)
			break;
		*pThis->quotes[iq].end = *pThis->quotes[iq].begin;
		pThis->quotes[iq].begin = 0;
		pThis->quotes[iq].end = 0;
	}
}
#endif

//*****************************************************************************
// split cmdline to tkn array and return nmb of token
static int split (microrl_t * pThis, int limit, char const ** tkn_arr)
{
	int i = 0;
	int ind = 0;
#ifdef _USE_QUOTING
	int iq = 0;
	char quote = 0;
	for (iq = 0; iq < _QUOTED_TOKEN_NMB; ++iq) {
		pThis->quotes[iq].begin = 0;
		pThis->quotes[iq].end = 0;
	}
	iq = 0;
#endif
	while (1) {
		// go to the first NOT whitespace
		while (((pThis->cmdline [ind] == ' ')||(pThis->cmdline [ind] == '\0')) && (ind < limit)) {
			ind++;
		}
		if (!(ind < limit)) return i;
#ifdef _USE_QUOTING
		if (pThis->cmdline [ind] == '\'' || pThis->cmdline [ind] == '"') {
			if (iq >= _QUOTED_TOKEN_NMB) {
				restore (pThis);
				return -1;
			}
			quote = pThis->cmdline [ind];
			pThis->quotes[iq].begin = pThis->cmdline + ind;
			ind++;
		}
#endif
		tkn_arr[i++] = pThis->cmdline + ind;
		if (i >= _COMMAND_TOKEN_NMB) {
#ifdef _USE_QUOTING
			restore (pThis);
#endif
			return -1;
		}
		// go to the first whitespace
		while (ind < limit) {
			if ((pThis->cmdline [ind] == ' ') || (pThis->cmdline [ind] == '\0')) {
#ifdef _USE_QUOTING
				if (!quote)
#endif
				{
					pThis->cmdline [ind] = '\0';
					//ind++;
					break;
        }
#ifdef _USE_QUOTING
				else if (pThis->cmdline [ind] == '\0') {
					restore (pThis);
					return -1;
				}
#endif 
#ifdef _USE_QUOTING
			} else if (pThis->cmdline [ind] == quote) {
				if (pThis->cmdline [ind + 1] != '\0') {
					restore (pThis);
					return -1;
				}
				quote = 0;
				pThis->quotes[iq++].end = pThis->cmdline + ind;
				pThis->cmdline [ind++] = '\0';
				break;
#endif
			}
			ind++;
		}
		if (!(ind < limit)) {
#ifdef _USE_QUOTING
			if (quote) {
				restore (pThis);
				return -1;
			}
#endif
			return i;
		}
	}
	return i;
}


//*****************************************************************************
void microrl_erase_prompt(microrl_t *pThis)
{
	pThis->print(pThis, "\033[999D\033[2K");
}

//*****************************************************************************
inline static void print_prompt (microrl_t * pThis)
{
	pThis->print (pThis, pThis->prompt_str);
}

//*****************************************************************************
inline static void terminal_backspace (microrl_t * pThis)
{
	pThis->print (pThis, "\033[D \033[D");
}

//*****************************************************************************
inline static void terminal_newline (microrl_t * pThis)
{
	pThis->print (pThis, ENDL);
}

//*****************************************************************************
// set cursor at current position + offset (positive or negative).
// the provided buffer must be at least 7 bytes long.
static char* generate_move_cursor (char* str, int offset)
{
	char c = 'C';
	if (offset > 999)
		offset = 999;
	if (offset < -999)
		offset = -999;
	if (offset < 0) {
		offset = -offset;
		c = 'D';
	} else if (offset == 0) {
		*str = '\0';
		return str;
	}

#ifdef _USE_LIBC_STDIO
	str += sprintf (str, "\033[%d%c", offset, c);
#else
	*str++ = '\033';
	*str++ = '[';
	char tmp_str [4] = {0,};
	int i = 0, j;
	while (offset > 0) {
		tmp_str[i++] = (offset % 10) + '0';
		offset /=10;
	}
	for (j = 0; j < i; ++j)
		*str++ = tmp_str [i-j-1];
	*str++ = c;
	*str = '\0';
#endif
	return str;
}

//*****************************************************************************
// set cursor at current position + offset (positive or negative)
static void terminal_move_cursor (microrl_t * pThis, int offset)
{
	char str[16] = {0,};
	if (offset != 0) {
		generate_move_cursor (str, offset);
		pThis->print (pThis, str);
	}
}

//*****************************************************************************
// print cmdline to screen, replace '\0' to whitespace
static void terminal_print_line (microrl_t * pThis, int pos, int reset)
{
	if (!ECHO_IS_OFF()) {
		char str[_PRINT_BUFFER_LEN];
		char *j = str;
		if (reset) {
#ifdef _USE_CARRIAGE_RETURN
			*j++ = '\r';
			j = generate_move_cursor(j, _PROMPT_LEN + pos);
#else
			j = generate_move_cursor(j, -(_COMMAND_LINE_LEN + _PROMPT_LEN + 2));
			j = generate_move_cursor(j, _PROMPT_LEN + pos);
#endif
		}
		for (int i = pos; i < pThis->cmdlen; i++) {
			*j = (pThis->cmdline [i] == '\0') ? ' ' : pThis->cmdline [i];
			if ((i >= pThis->start_password) & (ECHO_IS_ONCE()))
				*j = '*';
			j++;
			if (j-str == sizeof(str)-1) {
				*j = '\0';
				pThis->print (pThis, str);
				j = str;
			}
		}
		if (j - str + 3+6+1 > _PRINT_BUFFER_LEN) {
				*j = '\0';
				pThis->print (pThis, str);
				j = str;
		}
		*j++ = '\033';   // delete all past end of text
		*j++ = '[';
		*j++ = 'K';
		generate_move_cursor (j, pThis->cursor - pThis->cmdlen);
		pThis->print (pThis, str);
	}
}

//*****************************************************************************
void microrl_init (microrl_t * pThis, void (*print) (microrl_t*, const char *))
{
	memset(pThis, 0, sizeof(microrl_t));
	pThis->prompt_str = prompt_default;
	pThis->print = print;
#ifdef _ENABLE_INIT_PROMPT
	print_prompt (pThis);
#endif
	pThis->echo = ON;
	pThis->start_password = -1; //
}

//*****************************************************************************
void microrl_set_complete_callback (microrl_t * pThis, char ** (*get_completion)(microrl_t*, int, const char* const*))
{
	pThis->get_completion = get_completion;
}

//*****************************************************************************
void microrl_set_execute_callback (microrl_t * pThis, int (*execute)(microrl_t*, int, const char* const*))
{
	pThis->execute = execute;
}
#ifdef _USE_CTLR_C
//*****************************************************************************
void microrl_set_sigint_callback (microrl_t * pThis, void (*sigintf)(microrl_t*))
{
	pThis->sigint = sigintf;
}
#endif

#ifdef _USE_HISTORY
static void hist_search (microrl_t * pThis, int dir)
{
	int len = hist_restore_line (&pThis->ring_hist, pThis->cmdline, dir);
	if (len >= 0) {
		pThis->cmdline[len] = '\0';
		pThis->cursor = pThis->cmdlen = len;
		terminal_print_line (pThis, 0, 1);
	}
}
#endif

#ifdef _USE_ESC_SEQ
//*****************************************************************************
// handling escape sequences
static int escape_process (microrl_t * pThis, char ch)
{
	if (ch == '[') {
		pThis->escape_seq = _ESC_BRACKET;
		return 0;
	} else if (ch == 'O') {
		pThis->escape_seq = _ESC_BRACKET2;
		return 0;
	} else if (pThis->escape_seq == _ESC_BRACKET) {
		if (ch == 'A') {
#ifdef _USE_HISTORY
			if (!ECHO_IS_ONCE())
				hist_search (pThis, _HIST_UP);
#endif
			return 1;
		} else if (ch == 'B') {
#ifdef _USE_HISTORY
			if (!ECHO_IS_ONCE())
				hist_search (pThis, _HIST_DOWN);
#endif
			return 1;
		} else if (ch == 'C') {
			if (pThis->cursor < pThis->cmdlen) {
				terminal_move_cursor (pThis, 1);
				pThis->cursor++;
			}
			return 1;
		} else if (ch == 'D') {
			if (pThis->cursor > 0) {
				terminal_move_cursor (pThis, -1);
				pThis->cursor--;
			}
			return 1;
		} else if ((ch == '1') || (ch == '7')) {
			pThis->escape_seq = _ESC_HOME;
			return 0;
		} else if (ch == '8') {
			pThis->escape_seq = _ESC_END;
			return 0;
		} else if (ch == 'H') {
			terminal_move_cursor (pThis, -pThis->cursor);
			pThis->cursor = 0;
			return 1;
		} else if (ch == 'F') {
			terminal_move_cursor (pThis, pThis->cmdlen-pThis->cursor);
			pThis->cursor = pThis->cmdlen;
			return 1;
		}
	} else if (pThis->escape_seq == _ESC_BRACKET2) {
		if (ch == 'F') {
			terminal_move_cursor (pThis, pThis->cmdlen-pThis->cursor);
			pThis->cursor = pThis->cmdlen;
			return 1;
		}
	} else if (ch == '~') {
		if (pThis->escape_seq == _ESC_HOME) {
			terminal_move_cursor (pThis, -pThis->cursor);
			pThis->cursor = 0;
			return 1;
		} else if (pThis->escape_seq == _ESC_END) {
			terminal_move_cursor (pThis, pThis->cmdlen-pThis->cursor);
			pThis->cursor = pThis->cmdlen;
			return 1;
		}
	}

	/* unknown escape sequence, stop */
	return 1;
}
#endif

//*****************************************************************************
// insert len char of text at cursor position
static int microrl_insert_text (microrl_t * pThis, char * text, int len)
{
	int i;
	if (pThis->cmdlen + len < _COMMAND_LINE_LEN) {
		if (ECHO_IS_ONCE() & (pThis->start_password == -1))
			pThis->start_password = pThis->cmdlen;
		memmove (pThis->cmdline + pThis->cursor + len,
						 pThis->cmdline + pThis->cursor,
						 pThis->cmdlen - pThis->cursor);
		for (i = 0; i < len; i++) {
			pThis->cmdline [pThis->cursor + i] = text [i];
		}
		pThis->cursor += len;
		pThis->cmdlen += len;
		pThis->cmdline [pThis->cmdlen] = '\0';
		return true;
	}
	return false;
}

//*****************************************************************************
// remove len chars backwards at cursor
static void microrl_backspace (microrl_t * pThis, int len)
{
	if (pThis->cursor >= len) {
		memmove (pThis->cmdline + pThis->cursor-len,
						 pThis->cmdline + pThis->cursor,
						 pThis->cmdlen-pThis->cursor+len);
		pThis->cursor -= len;
		pThis->cmdline [pThis->cmdlen] = '\0';
		pThis->cmdlen -= len;
	}
}

//*****************************************************************************
// remove one char forward at cursor
static void microrl_delete (microrl_t * pThis)
{
	if ((pThis->cmdlen > 0) && (pThis->cursor < pThis->cmdlen)) {
		memmove (pThis->cmdline + pThis->cursor,
						 pThis->cmdline + pThis->cursor+1,
						 pThis->cmdlen-pThis->cursor+1);
		pThis->cmdline [pThis->cmdlen] = '\0';
		pThis->cmdlen--;
	}
}

#ifdef _USE_COMPLETE

//*****************************************************************************
static size_t common_len (char ** arr)
{
	size_t i;
	int j;
	char *shortest = arr[0];
	size_t shortlen = strlen(shortest);

	for (i = 0; arr[i] != NULL; ++i)
		if (strlen(arr[i]) < shortlen) {
			shortest = arr[i];
			shortlen = strlen(shortest);
		}

	for (i = 0; i < shortlen; ++i)
		for (j = 0; arr[j] != 0; ++j)
			if (shortest[i] != arr[j][i])
				return i;

	return i;
}

//*****************************************************************************
static void microrl_get_complite (microrl_t * pThis)
{
	char const * tkn_arr[_COMMAND_TOKEN_NMB];
	char ** compl_token;

	if (pThis->get_completion == NULL) // callback was not set
		return;

	int status = split (pThis, pThis->cursor, tkn_arr);
	if (status < 0)
		return;
	if (pThis->cmdline[pThis->cursor-1] == '\0')
		tkn_arr[status++] = "";
	compl_token = pThis->get_completion (pThis, status, tkn_arr);
#ifdef _USE_QUOTING
	restore (pThis);
#endif
	if (compl_token[0] != NULL) {
		int i = 0;
		size_t len;
		int pos = pThis->cursor;

		if (compl_token[1] == NULL) {
			len = strlen (compl_token[0]);
		} else {
			len = common_len (compl_token);
			terminal_newline (pThis);
			while (compl_token [i] != NULL) {
				pThis->print (pThis, compl_token[i]);
				pThis->print (pThis, " ");
				i++;
			}
			terminal_newline (pThis);
			print_prompt (pThis);
			pos = 0;
		}

		if (len) {
			microrl_insert_text (pThis, compl_token[0] + strlen(tkn_arr[status-1]),
																	len - strlen(tkn_arr[status-1]));
			if (compl_token[1] == NULL)
				microrl_insert_text (pThis, " ", 1);
		}
		terminal_print_line (pThis, pos, 0);
	}
}
#endif

#ifdef _USE_HISTORY_EXEC_PREV

int microrl_cp_histo_line(microrl_t* pThis, int indexHisto)
{
	int r;
	int begin;

	r = false;
	begin = hist_get_start_offset(&pThis->ring_hist, indexHisto);
	if (begin != -1) {
		hist_copy_line_from_index(&pThis->ring_hist, begin, pThis->cmdline);
		pThis->cmdlen = pThis->ring_hist.ring_buf[begin];
		r = true;
	}

	return r;
}

static int microrl_histo_check_and_cp_previous(microrl_t* pThis)
{
	char digit[5];
	int i;
	int j;
	int is_exec_histo;

	memset(digit, '\0', 5);
	j = 0;
	i = 0;
	is_exec_histo = false;

	while (i < pThis->cmdlen) {
		if (!is_exec_histo) {
			if (pThis->cmdline[i] == '!') {
				is_exec_histo = true;
			} else {
				break;
			}
		}
		else
		{
			if ((pThis->cmdline[i] == ' ') || (pThis->cmdline[i] == '\0'))
				break;
			else if (j < 4) {
				digit[j++] = pThis->cmdline[i];
			} else {
				//no more space...
				is_exec_histo = false;
				break;
			}
		}
		i++;
	}

	if ((is_exec_histo == true) && (j > 0)) {
		char* endStr;
		int indexHisto = strtoul(digit, &endStr, 0);
		if (*endStr == '\0') {
			// can execute now...
			is_exec_histo = microrl_cp_histo_line(pThis, indexHisto);
		} else {
			is_exec_histo = false;
		}
	} else
		is_exec_histo = false;

	return is_exec_histo;
}

#endif /* _USE_HISTORY_EXEC_PREV */


//*****************************************************************************
void new_line_handler(microrl_t * pThis)
{
	char const * tkn_arr [_COMMAND_TOKEN_NMB];
	int status;
	terminal_newline (pThis);

#ifdef _USE_HISTORY
	if ((pThis->cmdlen > 0) && (!ECHO_IS_ONCE())) {
#ifdef _USE_HISTORY_EXEC_PREV
		//check if request a previous command
		if (microrl_histo_check_and_cp_previous(pThis) == false)
#endif /* _USE_HISTORY_EXEC_PREV */
			hist_save_line (&pThis->ring_hist, pThis->cmdline, pThis->cmdlen);
	}
#endif
	if (ECHO_IS_ONCE()){
		microrl_set_echo(pThis, ON);
		pThis->start_password = -1;
		//DBG("v ==> %s\n", pThis->cmdline);
		if (pThis->execute != NULL) {
			tkn_arr[0] = pThis->cmdline;
			pThis->execute(pThis, 1, tkn_arr);
		}
	}  else {
		status = split (pThis, pThis->cmdlen, tkn_arr);
		if (status == -1){
			pThis->print (pThis, "ERROR:too many tokens");
#ifdef _USE_QUOTING
			pThis->print (pThis, " or invalid quoting");
#endif
			pThis->print (pThis, ENDL);
		}
	
		if ((status > 0) && (pThis->execute != NULL))
			pThis->execute (pThis, status, tkn_arr);
  }
	print_prompt (pThis);
	pThis->cmdlen = 0;
	pThis->cursor = 0;
	memset(pThis->cmdline, 0, _COMMAND_LINE_LEN);
#ifdef _USE_HISTORY
	pThis->ring_hist.cur = 0;
#endif
}

//*****************************************************************************

void microrl_insert_char (microrl_t * pThis, int ch)
{
  //DBG(">%d (0x%x) \n", ch, ch);
#ifdef _USE_ESC_SEQ
	if (pThis->escape) {
		if (escape_process(pThis, ch))
			pThis->escape = 0;
	} else {
#endif
		if (ch == KEY_CR || ch == KEY_LF) {
			// Only trigger a newline if ch doen't follow its companion's
			// triggering a newline.
			if (pThis->last_endl == (ch == KEY_CR ? KEY_LF : KEY_CR)) {
				pThis->last_endl = 0;      // ignore char, but clear newline state
			} else {
				pThis->last_endl = ch;
				new_line_handler(pThis);
			}
			return;
		}
		pThis->last_endl = 0;
		switch (ch) {
			//-----------------------------------------------------
#ifdef _USE_COMPLETE
			case KEY_HT:
				if (!ECHO_IS_ONCE())
					microrl_get_complite (pThis);
			break;
#endif
			//-----------------------------------------------------
			case KEY_ESC:
#ifdef _USE_ESC_SEQ
				pThis->escape = 1;
#endif
			break;
			//-----------------------------------------------------
			case KEY_NAK: // ^U
				if (pThis->cursor > 0)
					microrl_backspace (pThis, pThis->cursor);
				terminal_print_line (pThis, 0, 1);
			break;
			//-----------------------------------------------------
			case KEY_VT:  // ^K
				pThis->print (pThis,"\033[K");
				pThis->cmdlen = pThis->cursor;
			break;
			//-----------------------------------------------------
			case KEY_ENQ: // ^E
				terminal_move_cursor (pThis, pThis->cmdlen-pThis->cursor);
				pThis->cursor = pThis->cmdlen;
			break;
			//-----------------------------------------------------
			case KEY_SOH: // ^A
				terminal_move_cursor (pThis, -pThis->cursor);
				pThis->cursor = 0;
			break;
			//-----------------------------------------------------
			case KEY_ACK: // ^F
			if (pThis->cursor < pThis->cmdlen) {
				terminal_move_cursor (pThis, 1);
				pThis->cursor++;
			}
			break;
			//-----------------------------------------------------
			case KEY_STX: // ^B
			if (pThis->cursor) {
				terminal_move_cursor (pThis, -1);
				pThis->cursor--;
			}
			break;
			//-----------------------------------------------------
			case KEY_DLE: //^P
#ifdef _USE_HISTORY
			if (!ECHO_IS_ONCE())
				hist_search (pThis, _HIST_UP);
#endif
			break;
			//-----------------------------------------------------
			case KEY_SO: //^N
#ifdef _USE_HISTORY
			if (!ECHO_IS_ONCE())
				hist_search (pThis, _HIST_DOWN);
#endif
			break;
			//-----------------------------------------------------
			case KEY_DEL: // Backspace
			case KEY_BS: // ^H
				if (pThis->cursor > 0) {
					microrl_backspace (pThis, 1);
					if (pThis->cursor == pThis->cmdlen)
						terminal_backspace (pThis);
					else
						terminal_print_line (pThis, pThis->cursor, 1);
				}
			break;
			//-----------------------------------------------------
			case KEY_EOT: // ^D
				microrl_delete (pThis);
				terminal_print_line (pThis, pThis->cursor, 0);
			break;
			//-----------------------------------------------------
			case KEY_DC2: // ^R
				terminal_newline (pThis);
				print_prompt (pThis);
				terminal_print_line (pThis, 0, 0);
			break;
			//-----------------------------------------------------
#ifdef _USE_CTLR_C
			case KEY_ETX:
			if (pThis->sigint != NULL)
				pThis->sigint(pThis);
			break;
#endif
			//-----------------------------------------------------
			default:
			if (((ch == ' ') && (pThis->cmdlen == 0)) || IS_CONTROL_CHAR(ch))
				break;
			if (microrl_insert_text (pThis, (char*)&ch, 1)) {
				if (pThis->cursor == pThis->cmdlen) {
					if (!ECHO_IS_OFF()) {
						char nch [] = {0,0};
						if (ECHO_IS_ONCE())
							nch[0] = '*';
						else
							nch[0] = ch;
						pThis->print (pThis, nch);
					}
				} else {
					terminal_print_line (pThis, pThis->cursor-1, 0);
				}
			}
			break;
		}
#ifdef _USE_ESC_SEQ
	}
#endif
}

//*****************************************************************************
void microrl_set_echo (microrl_t * pThis, echo_t echo) {
	pThis->echo = echo;
}

echo_t microrl_get_echo (microrl_t * pThis) {
	return pThis->echo;
}
