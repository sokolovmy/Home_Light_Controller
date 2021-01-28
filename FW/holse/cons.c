#include "cons.h"
#include "emCLI.h"
#include "config.h"
#include "string.h"
#include "stdio.h"
#include "io.h"
#include "cmsis_os.h"


uint8_t *cBuf;
uint32_t *cLen;


/*
typedef bool(*CmdFunc)(const char*);
typedef struct EMCCommand {
	const char* cmdName;
	const char* shortDescr;
	const char* longDescr;
	const CmdFunc cmdFunc;
	const struct EMCCommand* subCmds;
} EMCCommand;
*/
/////////////////////////////////////////////////////////
void print(const char * str) {
		while(CDC_Transmit_FS((uint8_t *) str, strlen(str)) == USBD_BUSY)
			osDelay(10);
};
/////////////////////////////////////////////////////////

bool cmdHelp(char * args);
bool cmdInput(char * args);
bool cmdOutput(char * args);
bool cmdTimer(char * args);
bool cmdShow(char * args);
bool cmdSave(char * args);

EMCCommand cmds[] = {
	{ "help",
		"Show help for commands USAGE: help <command>",
		NULL,
		cmdHelp
	},
	{ "input",
		"Configure input ports",
		"\
USAGE: input (0-15) (down|click|dclick|lpress) (toggle|on|off) NUMMASK [timer TIMERNUM]\r\n\
USAGE: input (0-15) (down|click|dclick|lpress) nop [timer TIMERNUM]\r\n\
\r\n\
NUMMASK - binary mask for outputs\r\n\
		can be 0xHH - hex number\r\n\
		or 0bYYYYYYYYYYYYYYYY - binary number\r\n\
down - button down\r\n\
click - simple button click\r\n\
dclick - double button click\r\n\
lpress - down & hold button more 1 second\r\n\
\r\n\
toggle|on|off  outputs coded in NUMMASK\r\n\
nop - no operation with outputs\r\n\
\r\n\
TIMERNUM - number of timer to start\r\n\
\r\n\
=============================================\r\n\
toggle|on|off NUMMASK output ports (may be 0)\r\n\
& activate timer TIMERNUM\r\n",
		cmdInput,
		//NULL
	},
	
	{ "output",
		"change state output ports",
		"USAGE: output (0-15) (toggle|on|off)\r\n",
		cmdOutput,
		//NULL
	},
	
	{ "show",
		"Show configured input, timers or state of timer, output",
		"\
USAGE: show config\r\n\
USAGE: show (input|timer|output) [0-15]\r\n\
",
		cmdShow
	},
	
	{ "timer",
		"Set timer parameters",
		"\
USAGE: timer (0-15) SECONDS NUMMASK\r\n\
SECOUNDS - Time in Secounds to OFF NUMMASK Output ports\
NUMMASK - binary mask for outputs or timers\r\n\
",
		cmdTimer,
		//NULL
	},
	
	{ "save",
		"Save configuration to flash",
		NULL,
		cmdSave
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};




void printHelpCommands(){
	EMCCommand * cmd = cmds;
	print("Available Commands:\r\n");
	while (cmd->cmdFunc) {
		print(cmd->cmdName);
		print(" - ");
		print(cmd->shortDescr);
		print(ESC_NEWLINE);
		cmd++;
	};
	print("Type \"help <command>\" for more information\r\n");
};
bool cmdHelp(char * args){
	//printFunc("This is help\r\n Args: ");	printFunc(args);	printFunc(ESC_NEWLINE);
	if (!args) {
		printHelpCommands();
		return true;
	};
	EMCCommand * cmd = cmds;
	while(cmd->cmdName) {
		int len = strlen(cmd->cmdName);
		if (strncmp(args, cmd->cmdName, len) == 0) {
			print(ESC_NEWLINE);
			if (cmd->longDescr) print(cmd->longDescr);
			else print(cmd->shortDescr);
			print(ESC_NEWLINE);
			return true;
		};
		cmd++;
	};
	print("Error: help not found\r\n");
	return false;
};
////////////////////////////////////////////////////////////////////
int convertToInt(char * str) {
	char * end;
	int ret;
	if (strlen(str) > 2) {
		if (str[0] == '0' && str[1] == 'x') {//hex number
			ret = strtoul(&str[2], &end, 16);
			if (end <= (str + 2)) return -1;
			else return ret;
		} else if (str[0] == '0' && str[1] == 'b') {// binary number
			ret = strtoul(&str[2], &end, 2);
			if (end <= (str + 2)) return -1;
			else return ret;
		}
	};
	//dec number
	ret = strtoul(str, &end, 10);
	if (end <= str) return -1;
	else return ret;
};
void printOp(ConfigOp op) {
	switch(op) {
		case nop:
		case timerNop:
			print("nop");
			break;
		case toggle:
		case timerToggle:
			print(_TOGGLE_);
			break;
		case on:
		case timerOn:
			print(_ON_);
			break;
		case off:
		case timerOff:
			print(_OFF_);
			break;
	};
};
void printHex(int n, char * buf, size_t buflen) {
	snprintf(buf, buflen, "%x", n);
	print(buf);
};
void printDec(int n) {
	char buf[6];
	snprintf(buf, 5, "%u", n);
	print(buf);
};

void printOutputState(int output) {
	//print("This is output state print\r\n");
	print ("output ");
	printDec(output);
	print(" ");
	if (getOutputState(output)) print(_ON_);
	else print(_OFF_);
	print(ESC_NEWLINE);
};
void printConfigInput1Op(ConfigInput * ci, char * buf, int bufN){
	printOp(ci->op);
	if (ci->op != timerNop) {
		print(" 0x");
		printHex(ci->out, buf, bufN);
	}
	if ((ci->op & timerNop) == timerNop) {
		print(" timer ");
		printDec(ci->timerN);
	}
	print("\r\n");
}
void printConfigInput(int input) {
	print("input ");
	char buf[20];
	snprintf(buf, 20, "%u\r\n",  input);
	print(buf);

	//btn down
	ConfigInput * ci = getInput(input);
	if (ci->op != nop){
		print("\tdown   ");
		printConfigInput1Op(ci, buf, 20);
	}

	//click
	ci = getInputClick(input);
	if (ci->op != nop){
		print("\tclick  ");
		printConfigInput1Op(ci, buf, 20);
	}

	//double click
	ci = getInputDoubleClick(input);
	if (ci->op != nop){
		print("\tdclick  ");
		printConfigInput1Op(ci, buf, 20);
	}
	//long press
	ci = getInputLPress(input);
	if (ci->op != nop){
		print("\tlpress ");
		printConfigInput1Op(ci, buf, 20);
	}
};

void printConfigTimer(int input) {
	print("timer ");
	char buf[20];
	snprintf(buf, 20, "%2u",  input);
	print(buf);
	ConfigTimer * ct = getConfigTimer(input);
	print(" ");
	snprintf(buf, 20, "%lus ", ct->secs);
	print(buf);
	print(" 0x");
	printHex(ct->out, buf, 20);
	print("\r\n");
};
////////////////////////////////////////////////
bool cmdInput(char * args) {
	//tocken number of port
	char * tok = strtok(args, " ");
	if (!tok) return false;
	int inNum = convertToInt(tok); 
	if ((inNum < 0) || (inNum >= MAX_INPUTS)) return false;
	
	//token down|click|dclick|lpress
	tok = strtok(NULL, " ");
	if (!tok) return false;
	char click;
	if (strcmp(tok, "down") == 0) click = 'n';
	else if (strcmp(tok, "click") == 0 || strcmp(tok, "dclick") == 0 || strcmp(tok, "lpress") == 0)
		click = tok[0];
	else return false;
	
	//token toggle|on|off
	tok = strtok(NULL, " ");
	if (!tok) return false;
	ConfigOp op;
	if (strcmp(tok, "toggle") == 0) op = toggle;
	else if (strcmp(tok, "on") == 0) op = on;
	else if (strcmp(tok, "off") == 0) op = off;
	else if (strcmp(tok, "nop") == 0) op = nop;
	else return false;

	int outs = 0;
	//token NUMMASK
	if(op != nop) {
		tok = strtok(NULL, " ");
		if (!tok) return false;
		outs = convertToInt(tok);
		if (outs < 0) return false;
	}

	//token timer
	int timerNum = 0;
	tok = strtok(NULL, " ");
	if (strcmp(tok, "timer") == 0) {
		//token TIMERNUM
		tok = strtok(NULL, " ");
		timerNum = convertToInt(tok);
		if (timerNum < 0) return false;
		op |= timerNop;
	}

	//
	switch (click) {
	case 'n': //<down
		setInput(inNum, op, outs, timerNum);
		break;
	case 'c': //<click
		setInputClick(inNum, op, outs, timerNum);
		break;
	case 'd': //<double click
		setInputDoubleClick(inNum, op, outs, timerNum);
		break;
	case 'l': //<long press
		setInputLPress(inNum, op, outs, timerNum);
		break;
	};
	printConfigInput(inNum);
	return true;
};

bool cmdTimer(char * args){
	//token number of port
	char * tok = strtok(args, " ");
	if (!tok) return false;
	int inNum = convertToInt(tok); 
	if ((inNum < 0) || (inNum >= MAX_TIMERS)) return false;
	
	//token seconds
	tok = strtok(NULL, " ");
	if (!tok) return false;
	int secs = convertToInt(tok);
	if (secs < 0) return false;

	//token NUMMASK
	tok = strtok(NULL, " ");
	if (!tok) return false;
	int outs = convertToInt(tok);
	if (outs < 0) return false;
	
	setConfigTimer(inNum, outs, secs);
	printConfigTimer(inNum);
	return true;
};
bool cmdShow(char * args){
	char * tok = strtok(args, " ");
	if (!tok) return false;
	// config
	if (strcmp(tok, "config") == 0) {
		print("Inputs\r\n");
		for(int i = 0; i < MAX_INPUTS; i++)
			printConfigInput(i);
		print("Timers\r\n");
		for(int i = 0; i< MAX_TIMERS; i++)
			printConfigTimer(i);
		return true;
	};
	//input timer output
	if (strcmp(tok, "input") == 0 || strcmp(tok, "timer") == 0 || strcmp(tok, "output") == 0) {
		char t = tok[0];
		tok = strtok(NULL, " ");
		int inNum = convertToInt(tok);
		if (inNum < 0) {
			//not a number or empty str
			//print("Not a number or empty\r\n");
			switch(t) {
				case 'i':
					for(int i = 0; i < MAX_INPUTS; i++)
						printConfigInput(i);
					break;
				case 't':
					for(int i = 0; i < MAX_TIMERS; i++)
						printConfigTimer(i);
					break;
				case 'o':
					for(int i = 0; i < MAX_OUTPUTS; i++)
						printOutputState(i);
					break;
			}
			return true;
		}
		if (inNum >= MAX_INPUTS && t == 'i') return false;
		if (inNum >= MAX_TIMERS && t == 't') return false;
		if (inNum >= MAX_OUTPUTS && t == 'o') return false;
		switch(t) {
			case 'i':
				printConfigInput(inNum);
				break;
			case 't':
				printConfigTimer(inNum);
				break;
			case 'o':
				printOutputState(inNum);
				break;
		}
		
	} else return false;
	
	return true;
};
bool cmdOutput(char * args){
	//tocken number of out port
	char * tok = strtok(args, " ");
	if (!tok) return false;
	int inNum = convertToInt(tok); 
	if ((inNum < 0) || (inNum >= MAX_OUTPUTS)) return false;
	
	//token seconds
	tok = strtok(NULL, " ");
	if (!tok) return false;
	if (strcmp("toggle", tok) == 0) setOutputToggle(inNum);
	else if (strcmp("on", tok) == 0) setOutputOn(inNum);
	else if (strcmp("off", tok) == 0) setOutputOff(inNum);
	else return false;
	printOutputState(inNum);
	return true;
};

bool cmdSave(char * args) {
	if (saveToFlash() == 0) {
		print("Configuration saved successefuly\r\n");
		return true;
	}
	else return false;
};

////////////////////////////////////////////////
void putBufPtr(uint8_t * Buf, uint32_t * Len){
	cBuf = Buf;
	cLen = Len;
};

/////////////////////////////////////////////



void consThread(void * arg){
	int timeoutCounter = TERM_TIMEOUT * 100;
	while(1){
		if(cBuf) {
			timeoutCounter = TERM_TIMEOUT * 100;
			//CDC_Transmit_FS(cBuf, *cLen);
			cBuf[*cLen] = 0;
			processConsoleStr((char *)cBuf, cLen);
			uint8_t *Buf = cBuf;
			cBuf = NULL;
			CDC_Receive_FS_ProcessNext(Buf);
		};
		if (timeoutCounter > 0)	timeoutCounter--;
		else if (timeoutCounter == 0) {
			closeTerm();
			timeoutCounter--;
		};
		//HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);
		osDelay(10);
	};
};

const osThreadAttr_t consTask_attributes = {
  .name = "cons",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
void consInit(void){
	initCLI(cmds, print);
	/*osThreadId_t t = */osThreadNew(consThread, NULL, &consTask_attributes);

}
