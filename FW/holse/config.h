
#ifndef __CONFIG_HOME_LIGHT
#define __CONFIG_HOME_LIGHT

#include "stdint.h"
#include "stddef.h"

#define SPI_QUEUE_LENGHT 32
#define PROCESS_QUEUE_LENGTH 32
#define LONG_PRESS_COUNT 100 //< count in 10msec = 1sec
#define MAX_OUTPUTS 16 //< define outputs count
#define MAX_INPUTS 16 //< define inputs count
#define MAX_TIMERS 16 //< define timers count
#define MAX_INPUTS_BYTES MAX_INPUTS/8
typedef uint16_t BitPorts; //< change this type if max inputs contains more bits

typedef enum BOOL {
	false = 0,
	true
} bool;

typedef enum {
	nop = 0, //no operation
	on = 1,
	off = 2,
	toggle = 3,
	timerNop = 0x80,
	timerOn = 0x81,
	timerOff = 0x82,
	timerToggle = 0x83
} ConfigOp;

typedef struct __attribute__((__packed__)) {
	ConfigOp op; //operation
	BitPorts out; //output ports | timer (bitwise)
	uint8_t timerN; //N of timer to start if op == timerOn
} ConfigInput;
typedef struct __attribute__((__packed__)) {
	BitPorts out; //output ports | timer (bitwise)
	uint32_t secs;
} ConfigTimer;


void setInput(int input, ConfigOp op, BitPorts out, uint8_t timerN);
void setInputClick(int input, ConfigOp op, BitPorts out, uint8_t timerN);
void setInputDoubleClick(int input, ConfigOp op, BitPorts out, uint8_t timerN);
void setInputLPress(int input, ConfigOp op, BitPorts out, uint8_t timerN);
void setConfigTimer(int timer, BitPorts out,  uint32_t secs);
ConfigInput * getInput(int input);
ConfigInput * getInputClick(int input);
ConfigInput * getInputDoubleClick(int input);
ConfigInput * getInputLPress(int input);
ConfigTimer * getConfigTimer(int timer);
int loadFromFlash(void);
int saveToFlash(void);


#endif

