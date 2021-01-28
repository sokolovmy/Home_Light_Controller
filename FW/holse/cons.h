#ifndef __CONS_H__
#define __CONS_H__


#include "emCLI.h"
#include "usbd_cdc_if.h"

//#include "stdint.h"

#define TERM_TIMEOUT 120 //Timeout in seconds about :)
#define MAX_WORD_LENGTH 6 + 1

#define _CSI_		"\033["
#define _RESET_	_CSI_"0m"
#define _TOGGLE_ _CSI_"33mToggle"_RESET_
#define _ON_ _CSI_"32mOn"_RESET_
#define _OFF_ _CSI_"31mOff"_RESET_

//char * consBuf = 0;
void consInit(void);
void putBufPtr(uint8_t * Buf, uint32_t * Len);


#endif

