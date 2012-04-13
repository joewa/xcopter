/*
* Paparazzi ublox gps rx configurator $Id$
*
* Copyright (C) 2005 Martin Mueller
*
* This file is part of paparazzi.
*
* paparazzi is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* paparazzi is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with paparazzi; see the file COPYING. If not, write to
* the Free Software Foundation, 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "tinycomm.h"
#include "tinymsgdefs_fcxpc.h"
//#include "fstream_tinymsgdefs_fcxpc.h"



/* *********************** change this to your needs *********************** */

/* serial port that is used at the ublox gps receiver (1 or 2) */
#define UBLOX_PORT 1
/* serial speed that for the ublox gps receiver at startup (usb: don't care) */
#define DEFAULT_BAUDRATE 9600
/* this is the name of the input configuration text from ublox */
#define IN_FILE_NAME "Tiny_LEA-4P-v6.txt"
/* this is where your gps receiver lives on the PC (ttyS0, ttyACM0, ttyUSB0) */
#define OUT_FILE_NAME "/dev/ttyUSB0"
/* should the config be written to flash/battery backed ram? (1=yes) */
#define SAVE_PERMANENT 1

/* **************** no user servicable part below this line **************** */

#define SYNC_CHAR_1 0xB5
#define SYNC_CHAR_2 0x62
#define BLANK_CHAR ' '
#define MINUS_CHAR '-'

#define NAV 0x01
#define RXM 0x02
#define INF 0x04
#define ACK 0x05
#define ACK_ACK 0x01
#define CFG 0x06
#define CFG_PRT 0x00
#define CFG_MSG 0x01
#define CFG_INF 0x02
#define CFG_DAT 0x06
#define CFG_TP 0x07
#define CFG_RATE 0x08
#define CFG_CFG 0x09
#define CFG_TM 0x10
#define CFG_RXM 0x11
#define CFG_FXN 0x0E
#define CFG_ANT 0x13
#define CFG_SBAS 0x16
#define CFG_NMEA 0x17
#define UPD 0x09
#define MON 0x0A
#define MON_VER 0x04
#define AID 0x0B
#define TIM 0x0D

#define ACK_ACK_LENGTH 0x0A
#define CFG_CFG_LENGTH 0x14

struct speed_map
{
    speed_t speed;
    unsigned int speed_value;
};

static const struct speed_map speeds[] = {
    {B0, 0},
    {B50, 50},
    {B75, 75},
    {B110, 110},
    {B134, 134},
    {B150, 150},
    {B200, 200},
    {B300, 300},
    {B600, 600},
    {B1200, 1200},
    {B1800, 1800},
    {B2400, 2400},
    {B4800, 4800},
    {B9600, 9600},
    {B19200, 19200},
    {B38400, 38400},
    {B57600, 57600},
    {B115200, 115200},
    {B230400, 230400},
};

static const int NUM_SPEEDS = (sizeof(speeds) / sizeof(struct speed_map));


static speed_t int_to_baud(unsigned int value)
{
    int i;

    for (i = 0; i < NUM_SPEEDS; ++i)
    {
        if (value == speeds[i].speed_value)
        {
            return speeds[i].speed;
        }
    }
    return (speed_t) - 1;
}


int wait_for_ack( SerPortDLL_t* serportdll, int serial_fd )
{
unsigned char byte;
byte=0;
        Msg_RefValsAbs pc_msg_refvals;
        pc_msg_refvals.ac_state=0;
        pc_msg_refvals.gas=0;
        pc_msg_refvals.id=0;
        pc_msg_refvals.nick=0;
        pc_msg_refvals.roll=0;
        pc_msg_refvals.yaw=0;
    /* the acknowledge buffer to check if the gps understood it */
    unsigned char ack_ack[ACK_ACK_LENGTH] = {
        SYNC_CHAR_1, SYNC_CHAR_2, ACK, ACK_ACK, 0x02, 0x00 };
    struct timeval select_tv;
    fd_set read_fd_set;
    int max_fd;
    int i;
    int ack_index;
    unsigned char data_temp;

    ack_index = 0;
            
    /* 900 msec timeout for reply */
    select_tv.tv_sec = 1;
    select_tv.tv_usec = 20000;

    while (1)
    {
        /* wait for ACK-ACK */
        FD_ZERO( &read_fd_set );
        FD_SET( 0, &read_fd_set );
        FD_SET( serial_fd, &read_fd_set );
        max_fd = serial_fd;
        //perror("Hallo");
        i = select( 1,
                    NULL,
                    NULL,
                    NULL,
                    &select_tv );
        select_tv.tv_usec += 20000;
        if(select_tv.tv_usec/1000000 > 1) {
           select_tv.tv_usec=select_tv.tv_usec/1000000;
           select_tv.tv_usec = select_tv.tv_usec % 1000000;
        }
        msg_RefValsAbs2frame(&pc_msg_refvals, serportdll->frame_tx);
        serPortDLL_Tx_setframe(serportdll);
     while( serPortDLL_Tx_getbyte(serportdll, &byte) ) write(serial_fd, &byte, 1);
     // Stellwerte senden
     //port2fc->write(serport2fc.buffer_tx.constData(), serport2fc.buffer_tx.length());
        //perror("Hallo");
        /*if ( FD_ISSET(serial_fd, &read_fd_set) )
        {
            read(serial_fd, &data_temp, 1);
            if (data_temp != ack_ack[ack_index++])
            {
                ack_index = 0;
            }
            if (ack_index == ACK_ACK_LENGTH)
            {
                printf("OK\n");
                return(0);
            }
        }
        else
        {
            printf("*** no ACK from gps rx ***\n");
            return(-1);
        }*/
    }
}


int main (int argc, char **argv)
{
    char *in_file_name = NULL;
    char *out_file_name = OUT_FILE_NAME;

    struct termios orig_termios, cur_termios;

    /* the general buffer for config read from file */
    unsigned char data[65536] = { SYNC_CHAR_1, SYNC_CHAR_2 };
    /* this packet saves the just sent CFG commands */
    unsigned char data_temp;

    int serial_fd;
    int count, i;
    int br;
    int baud;
    int ublox_port = UBLOX_PORT;


    if ( (serial_fd = open( "/dev/ttyS0", O_RDWR )) == 0 )
    {
        perror("could not open serial port");
        return( -1 );
    }

    if (tcgetattr(serial_fd, &orig_termios))
    {
        perror("getting modem serial device attr");
        return( -1 );
    }

    cur_termios = orig_termios;
    
    /* input modes */
    cur_termios.c_iflag &= ~(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR
                           |ICRNL |IXON|IXANY|IXOFF|IMAXBEL);
    
    /* IGNCR does not pass 0x0D */
    cur_termios.c_iflag |= BRKINT;
    
    /* output_flags */
    cur_termios.c_oflag &=~(OPOST|ONLCR|OCRNL|ONOCR|ONLRET);
    
    /* control modes */
    cur_termios.c_cflag &= ~(CSIZE|CSTOPB|CREAD|PARENB|PARODD|HUPCL|CLOCAL|CRTSCTS);
    cur_termios.c_cflag |= CREAD|CS8|CLOCAL;
    
    /* local modes */
    cur_termios.c_lflag &= ~(ISIG|ICANON|IEXTEN|ECHO|FLUSHO|PENDIN);
    cur_termios.c_lflag |= NOFLSH;
    
    if (cfsetispeed(&cur_termios, B115200))
    {
        perror("setting input modem serial device speed");
        return( -1 );
    }
    
    if (cfsetospeed(&cur_termios, B115200))
    {
        perror("setting modem serial device speed");
        return( -1 );
    }
    
    if (tcsetattr(serial_fd, TCSADRAIN, &cur_termios))
    {
        perror("setting modem serial device attr");
        return( -1 );
    }
  
//       write(serial_fd, cfg_cfg, count+2);
        
//       wait_for_ack( cfg_cfg, serial_fd );

    SerPortDLL_t serportdll;
    serPortDLL_init(&serportdll);
    wait_for_ack(&serportdll , serial_fd );

    close(serial_fd);

    return(0);
}
