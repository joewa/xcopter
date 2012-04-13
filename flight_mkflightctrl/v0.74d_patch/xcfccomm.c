/**
Dieses Modul ist "Mikrokopter-frei"
*/

#include "xcfccomm.h"

void uart1RxframeHandler()
{
/*
Die der Nachrichten-id entsprechende Dekodierfunktion aus tinymsgdefs.h
aufrufen und entsprechende Aktionen ausführen
*/
  frame2Msg_RefValsAbs(ser_mc.frame_rx, &msg_RefValsAbs);

  push2Control(&msg_RefValsAbs);

//-------------- ANTWORTEN-------------
    pickMsgAllstates(&msg_xcallstates);
    msg_XCallstates2frame(&msg_xcallstates, ser_mc.frame_tx);
    serPortDLL_Tx_setframe(&ser_mc);
    uart1sendframe();
}

