#include "tinycomm.h"


//#define COMMDEBUG
  #ifdef COMMDEBUG
  #include <stdio.h>
  #include "tinymsgdefs_fcxpc.h"
  #endif

unsigned char crc8( unsigned char inCrc, unsigned char inData )
{
    unsigned char   i;
    unsigned char   data;
    data = inCrc ^ inData;
    for ( i = 0; i < 8; i++ ) 
    {
        if (( data & 0x80 ) != 0 )
        {
            data <<= 1;
            data ^= 0x07;
        }
        else
        {
            data <<= 1;
        }
    }
    return data;
} // Crc8

//-----------------------------------------------------------------------------------
//BEGIN FIFO
void frame_init(unsigned char *destframe, unsigned char *sourceframe, unsigned char len)
{
	unsigned char* size = destframe;
	int i;
	for(i = 0; i < len; i++) destframe[i+1] = sourceframe[i];
	*size = len;
}

void fifo_init(Fifo_t *f, unsigned char *buffer, const unsigned char size)
{
	f->count = f->read_i = f-> write_i = 0;
	f->buf = buffer;
	f->size = size;
}

unsigned char fifo_reset(Fifo_t *f, unsigned char *dataframe)
{
	unsigned char i;
	unsigned char *len = dataframe; // Byte 0 ist Länge
	if (*len >= f->size) return 0;
	fifo_init(f, f->buf, f->size);
	for(i = 0; i < *len; i++) fifo_put(f, dataframe[i+1]);
	return 1;
}

unsigned char fifo_putframe(Fifo_t *f, unsigned char *dataframe)
{
	unsigned char i;
	unsigned char *len = dataframe; // Byte 0 ist Länge
	if (f->count + *len >= f->size) return 0;  // zu gross fuer FIFO
	for(i = 0; i < *len; i++) fifo_put(f, dataframe[i+1]);
	return 1;
}

unsigned char fifo_put (Fifo_t *f, const unsigned char data)
{
	return _inline_fifo_put (f, data);
}

unsigned char fifo_get(Fifo_t *f, unsigned char *dest)
{
	unsigned char count = f->count;
	if (f->count) *dest = _inline_fifo_get (f);
	return count;
}

unsigned char fifo_crc8(Fifo_t *f)
{
	unsigned char byte, crc = 0;
	Fifo_t f_temp = *f;
	while( fifo_get(&f_temp, &byte) ) 
		if(byte != SEFLAG) crc = crc8(crc, byte);
	return crc;
}

unsigned char data_crc8(unsigned char *dataframe)
{
	unsigned char i, crc=0;
	unsigned char *len = dataframe; // Byte 0 ist Länge
	for(i = 0; i < *len; i++)
	{
		crc = crc8(crc, dataframe[i+1]);
	}
	return crc ^ 0xff;
}

//END FIFO
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//BEGIN HDLC
#define DLL_LEN 2 // length of data link layer protokoll
unsigned char tinyHDLC_putframe(Fifo_t *f, unsigned char *dataframe)
{
	unsigned char i;
	unsigned char *len = dataframe; // Byte 0 ist Länge
	if(f->count > 0 && *len >= f->size + DLL_LEN) return 0;
	dataframe[*len+1] = data_crc8(dataframe);
	fifo_put(f, SEFLAG);
	for(i = 0; i <= *len; i++) // "<=" wegen crc
	{
		if(dataframe[i+1] == SEFLAG || dataframe[i+1] == ESCCHAR)
		{
			fifo_put(f, ESCCHAR);
			fifo_put(f, dataframe[i+1] ^ 0x20); // invert Bit 5
		}
		else
		{
			fifo_put(f, dataframe[i+1]);
		}
	}
	fifo_put(f, SEFLAG);
	return 1;  // alles OK
}

unsigned char tinyHDLC_putbyte(Fifo_t *f, unsigned char byte)
{ // Schreibt empfangene Bytes in Fifo
  // wenn byte == SEFLAG return 0; --> FIFO wird geleert und ausgewertet
	if(byte == SEFLAG) return 0; //aufrufender muss Fifo resetten
	if( !fifo_put(f, byte) ) return 0; // Fifo ist voll!
	return 1;
}

unsigned char tinyHDLC_getframe(unsigned char *dataframe, Fifo_t *f)
{
	unsigned char byte, lastbyte = 0;
	unsigned char *len = dataframe;
	*len = 0; byte = 0;
	while( fifo_get(f, &byte) )
	{
		if(byte != ESCCHAR)
		{
			if(lastbyte != ESCCHAR) dataframe[*len+1] = byte;
			else dataframe[*len+1] = byte ^ 0x20; // invert Bit 5
			*len = *len + 1; // Achtung *len++ geht nicht
		}
		lastbyte = byte;
	}
	fifo_init(f, f->buf, f->size);
	//check crc
	if(*len > 1) *len = *len - 1;
	if( data_crc8(dataframe) != dataframe[*len+1] ) return 0; //crc fehler
	return 1;
}


//END HDLC
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//BEGIN SERPORTDLL
void serPortDLL_init(SerPortDLL_t *sp)
{
	fifo_init(&sp->fifo_rx, sp->buf_rx, DLL_SIZE_FIFO);
	fifo_init(&sp->fifo_tx, sp->buf_tx, DLL_SIZE_FIFO);
}

void serPortDLL_Tx_setframe(SerPortDLL_t *sp)
{
	tinyHDLC_putframe(&sp->fifo_tx, sp->frame_tx);
}

unsigned char serPortDLL_Rx_getframe(SerPortDLL_t *sp)
{
	return tinyHDLC_getframe(sp->frame_rx, &sp->fifo_rx);
}

unsigned char serPortDLL_Tx_getbyte(SerPortDLL_t *sp, unsigned char *dest)
{
	return fifo_get(&sp->fifo_tx, dest);
}

unsigned char serPortDLL_Rx_putbyte(SerPortDLL_t *sp, unsigned char byte)
{
	return tinyHDLC_putbyte(&sp->fifo_rx, byte);
}
//END SERPORTDLL
//-----------------------------------------------------------------------------------

  #ifdef COMMDEBUG
  #define BUFSIZE 20
int main()
{
	int i;
	Fifo_t myfifo;
	unsigned char mybuf[BUFSIZE];
	fifo_init(&myfifo, mybuf, BUFSIZE);
	
	char satz[20];
	char c;
	frame_init(satz, "Hello World\n", 11);
        for(i=0; i<satz[0]; i++) printf("%c",satz[i+1]);
	fifo_reset(&myfifo, satz);
	printf("%i\n", myfifo.count);
	for(i=0; i<9 && fifo_get(&myfifo,&c); i++) printf("%c", c);
	//fifo_putframe(&myfifo, "Kartoffelpuffer mit", 16);
	while(fifo_get(&myfifo,&c)) printf("%c", c);
	printf("\n");

	printf("HDLC:\n");
	//Nachrichten werden als Strukturen definiert
	unsigned char frame[DLL_SIZE_MSG];
        unsigned char temp;

	SerPortDLL_t pc;
	serPortDLL_init(&pc);
	SerPortDLL_t mc;
	serPortDLL_init(&mc);
	//frame_init(frame_tx2, "Meine Welt\n", 10);	//1. Sendeframe schreiben
	pc.frame_tx[1]=0x7e;
	pc.frame_tx[2]=0x7e;
	pc.frame_tx[3]=0x3a;
	pc.frame_tx[4]=0x7d;
	pc.frame_tx[5]=0x7d;
	pc.frame_tx[6]=0x11;
	pc.frame_tx[0]=6;
	for(i=0;i<pc.frame_tx[0];i++) printf("%x ",pc.frame_tx[i+1]);
	serPortDLL_Tx_setframe(&pc);			//2. In Fifo kopieren
	printf("\nAuf Leitung: \n");
	while( serPortDLL_Tx_getbyte(&pc, &temp) )	//3. Übertragen
	{
		printf("%x ",temp);
		if( serPortDLL_Rx_putbyte(&pc, temp) )	//4. Zeichen empfangen
		{
			//fifo_init(&fifo_rx, buf_rx, BUFSIZE);
		}
		else					// Nachricht vollständig
		{
			printf("Border\n");
			serPortDLL_Rx_getframe(&pc);
			for(i=0;i<pc.frame_rx[0];i++) printf("%x ",pc.frame_rx[i+1]);
		}					//5. Nachricht auspacken
	}



        printf("\nNormale Uebertragung von Msg_RefValsAbs\n");
	Msg_RefValsAbs pc_msg_refvals, mc_msg_refvals;
	pc_msg_refvals.nick = 27;
	pc_msg_refvals.roll = -23;
	pc_msg_refvals.yaw = 65;
	pc_msg_refvals.gas   = 198;
	pc_msg_refvals.ac_state = 0x01;
	mc_msg_refvals.nick = 0;
	mc_msg_refvals.roll = 0;
	mc_msg_refvals.yaw = 0;
	mc_msg_refvals.gas   = 0;
	mc_msg_refvals.ac_state = 0;
        printf("\nRefvalsAbs auf PC");
	msg_RefvalsAbs_print(&pc_msg_refvals);
        printf("\nRefvalsAbs auf uC");
	msg_RefvalsAbs_print(&mc_msg_refvals);

	//serPortDLL_init(&pc);
	msg_RefValsAbs2frame(&pc_msg_refvals, pc.frame_tx);
        printf("\nChecksumTX(PC)=%i\n",data_crc8(pc.frame_tx));
	serPortDLL_Tx_setframe(&pc);
	while( serPortDLL_Tx_getbyte(&pc, &temp) )	//3. Übertragen
	{
		printf("%x ",temp);
		if( serPortDLL_Rx_putbyte(&mc, temp) )	//4. Zeichen empfangen
		{
		}
		else					// Nachricht vollständig
                {
                    if(serPortDLL_Rx_getframe(&mc)==1)
                        printf("CRC ok");
                        else
                            printf("CRC Fehler");
                    printf("\nChecksumRX=%i\n",data_crc8(mc.frame_rx));
		}					//5. Nachricht auspacken
	}
	frame2Msg_RefValsAbs(mc.frame_rx, &mc_msg_refvals);
	msg_RefvalsAbs_print(&mc_msg_refvals);

        printf("\n---   INJECT FAULT   ---\n");
        int fault_i = 0;
        serPortDLL_Tx_setframe(&pc);
        while( serPortDLL_Tx_getbyte(&pc, &temp) )	//3. Übertragen
        {
            fault_i++;
            if(fault_i==5) temp=0;  // INJECT FAULT
                printf("%x ",temp);
                if( serPortDLL_Rx_putbyte(&mc, temp) )	//4. Zeichen empfangen
                {
                }
                else					// Nachricht vollständig
                {
                    if(serPortDLL_Rx_getframe(&mc)==1)
                        printf("CRC ok");
                        else
                            printf("CRC Fehler");
                    printf("\nChecksumRX=%i\n",data_crc8(mc.frame_rx));
                }					//5. Nachricht auspacken
        }
        frame2Msg_RefValsAbs(mc.frame_rx, &mc_msg_refvals);
        msg_RefvalsAbs_print(&mc_msg_refvals);


        printf("\nTransmitt the same message again");
        serPortDLL_Tx_setframe(&pc);
        while( serPortDLL_Tx_getbyte(&pc, &temp) )	//3. Übertragen
        {
                printf("%x ",temp);
                if( serPortDLL_Rx_putbyte(&mc, temp) )	//4. Zeichen empfangen
                {
                }
                else					// Nachricht vollständig
                {
                    if(serPortDLL_Rx_getframe(&mc)==1)
                        printf("CRC ok");
                        else
                            printf("CRC Fehler");
                    printf("\nChecksumRX=%i\n",data_crc8(mc.frame_rx));
                }					//5. Nachricht auspacken
        }
        frame2Msg_RefValsAbs(mc.frame_rx, &mc_msg_refvals);
        msg_RefvalsAbs_print(&mc_msg_refvals);
	return 0;
}

  #endif

