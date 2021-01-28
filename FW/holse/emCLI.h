#ifndef EMCLI_H
#define EMCLI_H

#include <stdint.h>
#include "config.h"

//////////////////////////////
#define ESC_NEWLINE		"\n\r"
#define ESC_CSI				"\033["
#define ESC_CSO				"\033O"

#define ESC_CUR_BACK		ESC_CSI"1D"
#define ESC_CUR_BACK_N	ESC_CSI"%dD"
#define ESC_CUR_FORW		ESC_CSI"1C"
#define ESC_CUR_FORW_N	ESC_CSI"%dC"
#define ESC_CUR_MOV_ABS	ESC_CSI"%dG"

//#define ESC_SAVCURPOS	ESC_CSI"s"
//#define	ESC_RESCURPOS	ESC_CSI"u"
#define	ESC_HIDE_CUR		ESC_CSI"?25l"
#define	ESC_SHOW_CUR		ESC_CSI"?25h"
#define ESC_CLEAR_SCREEN	ESC_CSI"2J"
#define ESC_RESET_CUR	ESC_CSI"1;1H"

#define ESC_SET_MAX_CUR_POS	ESC_CSI"255;255H"
#define	ESC_GET_MAX_CUR_POS	ESC_CSI"6n"
#define ESC_STARTTERM ESC_HIDE_CUR""ESC_SET_MAX_CUR_POS""ESC_GET_MAX_CUR_POS""ESC_CLEAR_SCREEN""ESC_SHOW_CUR""ESC_RESET_CUR

#define ESC_SAVE_CUR_POS ESC_CSI"s"ESC_HIDE_CUR
#define ESC_RESTORE_CUR_POS ESC_CSI"u"ESC_SHOW_CUR
#define ESC_ERASE_FROM_CUR ESC_CSI"0K"


#define PROMPT_STR "emCLI# "

#define WELCOME_STR "\
******************************\r\n\
** You are welcome to emCLI **\r\n\
******************************\r\n\r\n\
"PROMPT_STR

#define PROMPT_STR "emCLI# "

#define CMD_LINE_LEN 128
#define HIST_BUFFER_SIZE 256


//#define COMMAND_END { NULL, NULL, NULL, NULL, NULL}
typedef bool(*CmdFunc)(char*);
typedef struct EMCCommand {
	char* cmdName;
	char* shortDescr;
	char* longDescr;
	CmdFunc cmdFunc;
	//const struct EMCCommand* subCmds; // for future use
} EMCCommand;

typedef void (*PrintFunc)(const char * str);


void initCLI(EMCCommand * aCmds, PrintFunc pFunc);
char * toLower(char * s);
void closeTerm(void);
void processConsoleStr(char * str, uint32_t * len);
void printLog(char * str);

#endif
