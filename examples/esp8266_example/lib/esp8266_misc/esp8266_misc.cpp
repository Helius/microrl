#include "esp8266_misc.h"

/*
ESP8266 platform specific implementation routines.
*/
#define _ESP8266_DEMO_VER "1.0"

#define _LOGIN      "admin"
#define _PASSWORD   "1234"


// definition commands word
#define _CMD_HELP   "help"
#define _CMD_CLEAR  "clear"
#define _CMD_LOGIN    "login"

//*****************************************************************************
void print (microrl* pThis, const char* str)
{
	Serial.print(str);
}

//*****************************************************************************
void print_help (microrl * pThis)
{
	print (pThis, "clean              - clear screen\n\r");
	print (pThis, "login YOUR_LOGIN   - admin in this example\n\r");
	print (pThis, "if login is correct, you will be asked to enter password.\n\r");
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
int execute (microrl * pThis, int argc, const char * const * argv)
{
	static int pass_word = 0;
	int i = 0;
	// just iterate through argv word and compare it with your commands
	while (i < argc) {
		if (strcmp (argv[i], _CMD_HELP) == 0) {
			print (pThis, "microrl v");
			print (pThis, MICRORL_LIB_VER);
			print (pThis, "\r\nESP8266 DEMO v");
			print (pThis, _ESP8266_DEMO_VER);
			print(pThis, "\n\r");
			print_help (pThis);        // print help
		} else if (strcmp (argv[i], _CMD_CLEAR) == 0) {
			print (pThis, "\033[2J");    // ESC seq for clear entire screen
			print (pThis, "\033[H");     // ESC seq for move cursor at left-top corner
		} else if (strcmp (argv[i], _CMD_LOGIN) == 0) {
			if (++i < argc) {
				if (strcmp (argv[i], _LOGIN) == 0) {
					print(pThis, "Enter your password:\r\n");
					microrl_set_echo (prl, ONCE);
					pass_word = 1;
					return 1;
				} else {
					print(pThis, "Wrong login name. try again.\r\n");
					return 1;
				}
			} else {
				print(pThis, "Enter your login after command login.\r\n");
				return 1;
			}
		} else if (pass_word == 1) {
				if (strcmp(argv[i], _PASSWORD) == 0) {
					print(pThis, "Grate You Log In!!!\r\n");
					pass_word = 0;
					return 1;
				} else {
					print(pThis, "Wrong password, try log in again.\r\n");
					pass_word = 0;
					return 1;
				}
		} else {
			print (pThis, "Command: '");
			print (pThis, (char*)argv[i]);
			print (pThis, "' Not found.\n\r");
		}
		i++;
	}
	return 0;
}
