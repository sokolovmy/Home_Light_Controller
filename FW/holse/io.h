#ifndef IO_H_
#define IO_H_

#include "spi.h"
#include "cmsis_os2.h"
#include "config.h"

typedef enum {
	eventInputLongPress = 0,
	eventInputClick = 1,
	eventInputDoubleClick = 2,
//	eventInputTripleClick = 3,
//	eventInput4Click = 4,
	eventInputDown = 0x11,
	eventInputUp = 0x12
} enumProcessInputEvents;


void initIO(void);

void setOutputs(BitPorts outputs);
void setOutputOn(int num);
void setOutputOff(int num);
void setOutputsOff(BitPorts outputs);
void setOutputToggle(int num);
int getOutputState(int num);
BitPorts getOutputsState(void);


#endif /* IO_H_ */
