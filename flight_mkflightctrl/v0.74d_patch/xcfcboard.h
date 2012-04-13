#ifndef _XCFCBOARD_H_
#define _XCFCBOARD_H_


#include "tinycomm.h"
#include "tinymsgdefs_fcxpc.h"

Msg_RefValsAbs msg_RefValsAbs;
Msg_XCallstates msg_xcallstates;
SerPortDLL_t ser_mc;



void uart1sendframe();
void Uart1Init(void);
void SpektrumBinding(void);

//void (*uart1RxframeCallback)(void);
extern void uart1RxframeHandler();
void pickMsgAllstates(Msg_XCallstates *msg_xcallstates);
void push2Control(Msg_RefValsAbs *msg_RefValsAbs);

#endif
