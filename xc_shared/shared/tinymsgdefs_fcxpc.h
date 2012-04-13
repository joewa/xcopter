#ifndef _TINYMSGDEFS_FCxPC_H_
#define _TINYMSGDEFS_FCxPC_H_

#include "xctypes.h"

  #ifdef COMMDEBUG
  #include <stdio.h>
  #endif

#define XC_ON 0x1


/*
CAUTION: Mind, that types can be different on some targets.
E.g. avoid "int"
*/

//#include <avr/pgmspace.h>

/*
Sollwerte können auch differentiell übertragen werden.
Um Fehler zu verhindern sollte dann aber ein aktueller
Wert zurückgegeben werden.
*/
typedef struct
{
	//const unsigned char msgid = 0x00;
	signed char dnick; //int8_t
	signed char droll;
	signed char dyaw;
	signed char gas;
	unsigned char ac_state;
} Msg_RefVals;

static inline unsigned char
frame2Msg_RefVals(unsigned char *frame, Msg_RefVals *msg)
{
	if(frame[0] != 5 ) return 0;
	msg->dnick	= frame[1];
	msg->droll	= frame[2];
	msg->dyaw	= frame[3];
	msg->gas	= frame[4];
	msg->ac_state	= frame[5];
	return 1;
}

static inline unsigned char
msg_RefVals2frame(Msg_RefVals *msg, unsigned char *frame)
{
	frame[0] = 5;
	frame[1] = msg->dnick;
	frame[2] = msg->droll;
	frame[3] = msg->dyaw;
	frame[4] = msg->gas;
	frame[5] = msg->ac_state;
	return 1;
}


typedef struct
{
	//const unsigned char msgid = 0x00;
	signed char gyro_nick;
	signed char gyro_roll;
	signed char gyro_yaw;
	unsigned char pressure;
	unsigned char ac_state;
} Msg_ActVals;

#define ID_Msg_RefValsAbs 1
#define SZ_Msg_RefValsAbs 6

typedef struct
{
	//const unsigned char msgid = 0x00;
        uint8_t id;
        int8_t nick;
        int8_t roll;
        int8_t yaw;
        int8_t gas;
        uint8_t ac_state;
} Msg_RefValsAbs;


static inline unsigned char
frame2Msg_RefValsAbs(unsigned char *frame, Msg_RefValsAbs *msg)
{
	if(frame[0] != SZ_Msg_RefValsAbs ) return 0;
	msg->id		= frame[1];
	msg->nick	= frame[2];
	msg->roll	= frame[3];
	msg->yaw	= frame[4];
	msg->gas	= frame[5];
	msg->ac_state	= frame[6];
	return 1;
}

static inline unsigned char
msg_RefValsAbs2frame(Msg_RefValsAbs *msg, unsigned char *frame)
{
	frame[0] = SZ_Msg_RefValsAbs;
	frame[1] = ID_Msg_RefValsAbs;
	frame[2] = msg->nick;
	frame[3] = msg->roll;
	frame[4] = msg->yaw;
	frame[5] = msg->gas;
	frame[6] = msg->ac_state;
	return 1;
}

  #ifdef COMMDEBUG
void msg_RefvalsAbs_print(Msg_RefValsAbs *msg)
{
	int i;
	printf("\nMsg_RefValsAbs:\n");
		printf("Nick = %i\n", msg->nick);
		printf("Roll = %i\n", msg->roll);
		printf("Yaw = %i\n", msg->yaw);
		printf("Gas = %i\n", msg->gas);
		printf("ac_state = %i\n", msg->ac_state);
}
  #endif

#define ID_Msg_XCallstates 3

typedef struct
{
	//const unsigned char msgid = 0x00;
        uint8_t id;
        int16_t gyro_nick;
        int16_t gyro_roll;
        int16_t gyro_yaw;
        int8_t acc_nick;
        int8_t acc_roll;
        int8_t acc_yaw;
        int16_t pressure;
        int8_t compass_nick;
        int8_t compass_roll;
        int16_t compass_yaw;
        uint8_t motor1;
        uint8_t motor2;
        uint8_t motor3;
        uint8_t motor4;
        uint8_t voltage;
        uint8_t ac_state;
} Msg_XCallstates;

typedef unsigned char* JStream;

#define frame2msg(msg,frame,i) {memcpy(&(msg),(frame)+(i),sizeof((msg)));(i)+=sizeof((msg));}
#define msg2frame(frame,msg,i) {memcpy((frame)+(i),&(msg),sizeof((msg)));(i)+=sizeof((msg));}

static inline unsigned char
frame2Msg_XCallstates(unsigned char *frame, Msg_XCallstates *msg)
{
        //if(frame[0] != SZ_Msg_XCallstates ) return 0;
    unsigned char i=2;
    msg->id		= frame[1];
    frame2msg( msg->gyro_nick, frame, i );
    frame2msg( msg->gyro_roll, frame, i );
    frame2msg( msg->gyro_yaw, frame, i );
    frame2msg( msg->acc_nick, frame, i );
    frame2msg( msg->acc_roll, frame, i );
    frame2msg( msg->acc_yaw, frame, i );
    frame2msg( msg->pressure, frame, i );
    frame2msg( msg->compass_nick, frame, i );
    frame2msg( msg->compass_roll, frame, i );
    frame2msg( msg->compass_yaw, frame, i );
    frame2msg( msg->motor1, frame, i );
    frame2msg( msg->motor2, frame, i );
    frame2msg( msg->motor3, frame, i );
    frame2msg( msg->motor4, frame, i );
    frame2msg( msg->voltage, frame, i );
    frame2msg( msg->ac_state, frame, i );
    return 1;
}

static inline unsigned char
msg_XCallstates2frame(Msg_XCallstates *msg, unsigned char *frame)
{
    unsigned char i=2;
    frame[1] = ID_Msg_XCallstates;
    msg2frame( frame, msg->gyro_nick, i );
    msg2frame( frame, msg->gyro_roll, i );
    msg2frame( frame, msg->gyro_yaw, i );
    msg2frame( frame, msg->acc_nick, i );
    msg2frame( frame, msg->acc_roll, i );
    msg2frame( frame, msg->acc_yaw, i );
    msg2frame( frame, msg->pressure, i );
    msg2frame( frame, msg->compass_nick, i );
    msg2frame( frame, msg->compass_roll, i );
    msg2frame( frame, msg->compass_yaw, i );
    msg2frame( frame, msg->motor1, i );
    msg2frame( frame, msg->motor2, i );
    msg2frame( frame, msg->motor3, i );
    msg2frame( frame, msg->motor4, i );
    msg2frame( frame, msg->voltage, i );
    msg2frame( frame, msg->ac_state, i );
    frame[0] = i-1;
    return 1;
}

#endif
