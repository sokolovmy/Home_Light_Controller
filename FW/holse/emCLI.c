#include "emCLI.h"
#include "string.h"
#include "stdio.h"
#include "usbd_cdc_if.h"
#include "cmsis_os.h"
#include "ctype.h"


int termCols = 0, termRows = 0; //terminal size
bool termStarted = false;
PrintFunc printFunc = NULL;
EMCCommand * commands = NULL;
/////////////////////////////////////
char lineCmdStr[CMD_LINE_LEN+1] = {0}; 
uint8_t lineCursor = 0; //line Editor Cursor current position
uint8_t lineLength = 0; //line current size
int lineMaxLength = 0;
////////////////////////////////////
#define HIST_ROUND_IDX(idx)	(histRoundStart + idx) % HIST_BUFFER_SIZE
char histBuffer[HIST_BUFFER_SIZE] = {0};
uint16_t histRoundStart = 0, histSize = 0, histCur = 0;
/////////////////////////////////////////
////////////////////////////////////////
void initCLI(EMCCommand * aCmds, PrintFunc pFunc){
	commands = aCmds;
	printFunc = pFunc;
};

//////////////////////////////////////
void printCh(char ch) {
	if (!termStarted) return;
	char buf[2] = {0};
	buf[0] = ch;
	printFunc(buf);
};
//////////////////////////////////////
void redrawLineAfterCursor(){
		printFunc(ESC_ERASE_FROM_CUR);
		printFunc(ESC_SAVE_CUR_POS);
		printFunc(&lineCmdStr[lineCursor]);
		printFunc(ESC_RESTORE_CUR_POS);
}
void redrawCursor(){
	char s[8] = {0};
	snprintf(s, 7, ESC_CUR_MOV_ABS, strlen(PROMPT_STR) + lineCursor + 1);
	printFunc(s);
};
/// @fn void printLog(char*)
/// @brief print logs in terminal
///
/// @param str
void printLog(char * str){
	printFunc(ESC_NEWLINE);
	printFunc(str);
/*
	if(termStarted) {
		printFunc(ESC_NEWLINE);
		printFunc(PROMPT_STR);
		printFunc(lineCmdStr);
		redrawCursor();
	}
*/
}


void histClean(uint16_t cSize) {
	while(cSize + histSize > HIST_BUFFER_SIZE) {
		for (uint16_t q = 1; q < histSize; q ++) {
			uint16_t idx = HIST_ROUND_IDX(q);
			if(histBuffer[idx] && histBuffer[HIST_ROUND_IDX(q - 1)] == 0) {
				histRoundStart = idx;
				histSize -= q;
				break;
			}
		}
	}
};
void histPutIn(void){
	int slen = strlen(lineCmdStr) + 1;
	if (slen == 1) return; //empty string
	if (slen > HIST_BUFFER_SIZE) return; //lineCmdStr is oversize history buffer	
	if (slen > HIST_BUFFER_SIZE - histSize) //clean old history strings
		histClean(slen);
	for(uint16_t q = 0; q < slen; q ++)
		histBuffer[HIST_ROUND_IDX(histSize + q)] = lineCmdStr[q];
	
	histCur = (histSize += slen);
};
void histGetFrom(void) {
	for(uint16_t q = 0;; q++) {
		lineCmdStr[q] = histBuffer[HIST_ROUND_IDX(histCur + q)];
		if (lineCmdStr[q] == 0) {
			lineLength = q;
			break;
		}
	}
	lineCursor = 0;
	redrawCursor();
	redrawLineAfterCursor();
	lineCursor = lineLength;
	redrawCursor();
};


void histMove(bool fwd) {
	if (fwd) { //move forward
		for(int q = histCur + 1;q < histSize ; q++) {
			if (histBuffer[HIST_ROUND_IDX(q)] && histBuffer[HIST_ROUND_IDX(q - 1)] == 0) {
				histCur = q;
				break;
			}
		}
	} else { // move backward
		for (int q = histCur - 1;q >= 0 ; q--) {
			if (histBuffer[HIST_ROUND_IDX(q)] && histBuffer[HIST_ROUND_IDX(q - 1)] == 0) {
				histCur = q;
				break;
			}
		}
	}
};
/////////////////////////////////////////




void putInLineCh(char ch){
	if (lineLength + 1 > lineMaxLength) // buffer oversize
		return;
	lineCmdStr[lineLength] = 0;
	if (lineCursor < lineLength)
			memmove(&lineCmdStr[lineCursor + 1], &lineCmdStr[lineCursor], lineLength - lineCursor);
	
	lineCmdStr[lineCursor] = ch;
	lineCmdStr[++lineLength] = 0;
	if (lineCursor < lineLength) {
			redrawLineAfterCursor();
			printFunc(ESC_CUR_FORW);
	} else {
		printCh(ch);
	};
	lineCursor++;
};
void delInLine(bool bs) {
	if (!lineLength) return;
	if (bs && lineCursor > 0){ //del previus symbol from cursor
		lineCursor--;
		printFunc(ESC_CUR_BACK);
	}
	if (!bs && lineCursor >= lineLength) return;
	int mm = lineLength - lineCursor;
	//if (!mm) return;
	memmove(&lineCmdStr[lineCursor], &lineCmdStr[lineCursor + 1], mm);
	lineCmdStr[--lineLength] = 0;
	redrawLineAfterCursor();
	//printCh(KEY_BS);
	
};
//////////////////////////////////////
void closeTerm(void){
	termStarted = false;
	printFunc(ESC_NEWLINE"terminal closed to open press <ENTER>"ESC_NEWLINE);
};
//////////////////////////////////////
bool processCmds(char * cmdStr) {
	if(!cmdStr[0]) return true;
	EMCCommand * cmd = commands;
	while(cmd->cmdName) {
		int len = strlen(cmd->cmdName);
		if (strncmp(cmdStr, cmd->cmdName, len) == 0) {
			if (cmdStr[len] == ' ') return cmd->cmdFunc(&(cmdStr[len+1]));
			else if (cmdStr[len] == 0) return cmd->cmdFunc(NULL);
		}
		cmd++;
	};
	return false;
};
//////////////////////////////////////
char * toLower(char * s) {
	for (char *p = s; *p; p++) *p = tolower(*p);
	return s;
};
void processNewLine(bool alt) {
	
	if (!termStarted) {
		printFunc(ESC_STARTTERM);
		printFunc(WELCOME_STR);
		termStarted = true;
		lineCursor = lineLength = lineCmdStr[0] = 0;
		return;
	};
	printFunc(ESC_NEWLINE);
	//printFunc(lineCmdStr);
	//printFunc(ESC_NEWLINE);
	toLower(lineCmdStr);
	histPutIn();
	if (!processCmds(lineCmdStr))
		printFunc("Error parsing command. Type help for list of available commands\r\n");
	printFunc(PROMPT_STR);
	lineCursor = lineLength = lineCmdStr[0] = 0;
};

bool moveLineCursor(bool forward, bool byWord) {
	//int saveCursor = lineCursor;
	//char s[7] = {0};
	if(forward) {//move cursor to forward
		if (lineCursor >= lineLength) return false; //end of lineCmdStr dont move
		lineCursor++;
		if (byWord) {
			while(lineCursor < lineLength && lineCmdStr[lineCursor] != 0x20)
				lineCursor++;
			while(lineCursor < lineLength && lineCmdStr[lineCursor] == 0x20)
				lineCursor++;
		}
		//snprintf(s, 6,  ESC_CUR_FORW_N, lineCursor - saveCursor);
	} else { //move cursor to back
		if (lineCursor <= 0) return false; //cursor at start of line
		lineCursor--;
		if (byWord) {
			while(lineCursor > 0 && lineCmdStr[lineCursor] == 0x20)
				lineCursor--;
			while(lineCursor > 0 && lineCmdStr[lineCursor - 1] != 0x20)
				lineCursor--;
		};
		//snprintf(s, 6,  ESC_CUR_BACK_N, saveCursor - lineCursor);
	};
	//printFunc(s); 
	redrawCursor();
	return true;
};
////////////////////////////////////////////////
void processACMultiple(char *lastCmd){
	EMCCommand * cmd = commands;
	printFunc("\r\nAvailable commands: ");
	int i = 0;
	while(cmd->cmdName) {
		if (strncmp(cmd->cmdName, lineCmdStr, lineLength) == 0) {
			if (lastCmd != cmd->cmdName) 
				for(i = lineLength; lastCmd[i] && cmd->cmdName[i]; i++)
					if (lastCmd[i] != cmd->cmdName[i]) break;
			
			printFunc(cmd->cmdName);
			printFunc(" ");
		}
		cmd++;
	}
	printFunc(ESC_NEWLINE);
	printFunc(PROMPT_STR);
	printFunc(lineCmdStr);
	for(int q = lineLength; q < i; q++) lineCmdStr[q] = lastCmd[q];
	lineCmdStr[i] = 0;
	printFunc(&lineCmdStr[lineLength]);
	lineCursor = lineLength = i;
	//printFunc(&lastCmd[lineLength]
};
void processAutoComplete(void){
	if (!lineLength) return; // empty string
	EMCCommand * cmd = commands;
	char * lastCmdName = NULL;
	int cmdCount = 0;
	while(cmd->cmdName) {
		if (strncmp(cmd->cmdName, lineCmdStr, lineLength) == 0) {
			lastCmdName = cmd->cmdName;
			cmdCount++;
		}
		cmd++;
	};
	if(cmdCount == 1) {// simple command do autocomplete
		
		strncat(lineCmdStr, &lastCmdName[lineLength], CMD_LINE_LEN - 1);
		//strncat(lineCmdStr, " ", CMD_LINE_LEN - 1);
		printFunc(&lastCmdName[lineLength]);
		//printFunc(" ");
		lineCursor = lineLength = strlen(lineCmdStr);
	} else if (cmdCount > 1) processACMultiple(lastCmdName);
};

////////////////////////////////////////////////
// processConsoleStr2
// str - input string
// alt - return true if expected alt sequence
// return remaining string
////////////////////////////////////////////////
char * processConsoleStr2(char * str, bool * alt) {
	int strl = strlen(str);
	bool localAlt = *alt;
	*alt = false;
	if (strl >= 4 && str[0] == 0x1b && str[1] == 0x5b && str[3] == 0x7e) {// INS (0x32) | DEL (0x33) | HOME (0x31) | END (0x34) | PGUP (0x35) | PGDOWN (0x36)
		switch(str[2]) {
			case 0x33: // DEL
				delInLine(false);
				break;
			case 0x31: // HOME
				lineCursor = 0;
				redrawCursor();
				break;
			case 0x34: // END
				lineCursor = lineLength;
				redrawCursor();
				break;
		};
		return &str[4];
  ///////////////////////////////////////////////////////////////////////////////////////
	} else if (strl >= 3 && str[0] == 0x1b && str[1] == 'O') { // CSO codes CTRL + (LEFT (0x44) | RIGHT (0x43) | UP (0x41) | DOWN (0x42) | NUMPAD5 (0x47) )
		switch(str[2]) {
			case 0x44: // CTRL + LEFT
				moveLineCursor(false, true);
				break;
			case 0x43: // CTRL + RIGHT
				moveLineCursor(true, true);
				break;
		};
		return &str[3];
	///////////////////////////////////////////////////////////////////////////////////////
	} else if (strl >= 3 && str[0] == 0x1b && str[1] == '[') { //CSI codes
		if (sscanf(&str[2], "%d;%dR", &termRows, &termCols) == 2) {
			//answer back terminal to CSI 6 n command - current terminal size
			//set current terminal size etc
			lineMaxLength = CMD_LINE_LEN > (termCols - 1) ? (termCols - 1) : CMD_LINE_LEN;
			lineMaxLength -= strlen(PROMPT_STR);
			return strchr(&str[2], 'R') + 1;
		};
		switch(str[2]) {
			case 0x44: // LEFT
				moveLineCursor(false, false);
				break;
			case 0x43: // RIGHT
				moveLineCursor(true, false);
				break;
			case 0x41: // UP
				//histPutIn();
				histMove(false);
				histGetFrom();
				break;
			case 0x42: // DOWN
				//histPutIn();
				histMove(true);
				histGetFrom();
				break;
		};
		return &str[3];
	///////////////////////////////////////////////////////////////////////////////////////
	} else { //single char keys
		switch(str[0]){
			case 0x1b: // ESC
				if (!localAlt) *alt = true;
				else { // DOUBLE ESCAPE PRESSED
					lineLength = lineCmdStr[0] = lineCursor = 0;
					redrawCursor();
					redrawLineAfterCursor();
				}
				break;
			case 0x08: // BACKSPACE
			case 0x7F: // CTRL + BACKSPACE
				delInLine(true);
				break;
			case 0xd: // ENTER
				processNewLine(localAlt);
				break;
			case '\t': //TAB
				processAutoComplete();
				break;
			case 0xc: // CTRL + L
				break;
			default: 
				if (str[0] >= 0x20 && str[0] <= 0x7F) // other keys
					putInLineCh(str[0]);
		};
		return &str[1];
	};
};	
void processConsoleStr(char * str, uint32_t * len){
	static bool alt = false;
	str[*len] = 0;
	while(str[0] != 0) str = processConsoleStr2(str, &alt);
};

