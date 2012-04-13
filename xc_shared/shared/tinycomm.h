#ifndef _TINYCOMM_H_
#define _TINYCOMM_H_

 #ifdef __cplusplus
 extern "C" {
 #endif

/*
Written by Joerg Wangemann
Licence: GPLv3
- for RS232 communication
- using HDLC in theory
- implements a Data Link Layer (OSI-Layer 2)
- may be used in simulation
*/


  // ein Teilnehmer ist entweder nur Sender oder Empfänger
  // Callback-function zum senden/empfangen - kann durch modell getauscht werden
  // Send/Recv - function : nur eine nötig
  //unsigned char *buf;
  //unsigned char buf[TINYBUFSIZE]; unsigned char buf_read_i; char buf_write_id;
  //bool READ_FROM_BUF;



// void writeTinyMsg(struct TinyMsg)
// void readTinyMsg(struct TinyMsg)

#define DLL_SIZE_MSG 18
#define DLL_SIZE_FIFO 48 // = DLL_SIZE_MSG*2+2

typedef struct
{
	unsigned char *buf;		// Puffer
	unsigned char size;		// Puffer-Größe
	unsigned char volatile count;	// # Zeichen im Puffer
	unsigned char read_i;		// aktueller Leseindex
	unsigned char write_i;		// aktueller Schreibindex
} Fifo_t;

#define SEFLAG 0x7e
#define ESCCHAR 0x7d
typedef struct
{
	unsigned char len;  // noch nicht benutzt
	unsigned char crc;  // noch nicht benutzt
	// Callback: wenn FIFO leer
} HDLC_t;

// Stores all data of the serial port Data Link Layer
typedef struct
{
	unsigned char frame_rx[DLL_SIZE_MSG];
	unsigned char buf_rx[DLL_SIZE_FIFO];
	Fifo_t fifo_rx;
	unsigned char frame_tx[DLL_SIZE_MSG];
	unsigned char buf_tx[DLL_SIZE_FIFO];
	Fifo_t fifo_tx;
} SerPortDLL_t;

unsigned char crc8( unsigned char inCrc, unsigned char inData );

void frame_init(unsigned char *destframe, unsigned char *sourceframe, unsigned char len);
/*extern*/ void fifo_init (Fifo_t*, unsigned char* buf, const unsigned char size);
unsigned char fifo_reset(Fifo_t *f, unsigned char *dataframe);
unsigned char fifo_putframe(Fifo_t *f, unsigned char *dataframe);
unsigned char fifo_put (Fifo_t *f, const unsigned char data);
unsigned char fifo_get(Fifo_t *f, unsigned char *dest);
unsigned char fifo_crc8(Fifo_t *f);
unsigned char data_crc8(unsigned char *dataframe);

unsigned char tinyHDLC_putframe(Fifo_t *f, unsigned char *dataframe);
unsigned char tinyHDLC_putbyte(Fifo_t *f, unsigned char byte);
unsigned char tinyHDLC_getframe(unsigned char *dataframe, Fifo_t *f);

void serPortDLL_init(SerPortDLL_t *sp);
void serPortDLL_Tx_setframe(SerPortDLL_t *sp);
unsigned char serPortDLL_Rx_getframe(SerPortDLL_t *sp);
unsigned char serPortDLL_Tx_getbyte(SerPortDLL_t *sp, unsigned char *dest);
unsigned char serPortDLL_Rx_putbyte(SerPortDLL_t *sp, unsigned char byte);

static inline unsigned char
_inline_fifo_put (Fifo_t *f, const unsigned char data)
{
	if (f->count >= f->size) return 0;
	*(f->buf + f->write_i) = data;
	if (++f->write_i == f->size) f->write_i = 0;
	f->count++;
	return 1;
}

static inline unsigned char 
_inline_fifo_get (Fifo_t *f)
{
	unsigned char data = *(f->buf + f->read_i);
	if (++f->read_i == f->size) f->read_i = 0;
	f->count--;
	return data;
}

 #ifdef __cplusplus
 }
 #endif

#endif
