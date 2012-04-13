#include "xcfcboard.h"

#include "main.h"

#include "tinycomm.h"
#include "tinymsgdefs_fcxpc.h"

//--------------------------------------------------------------//

//--------------------------------------------------------------//

//BEGIN MK_COMPATIBILITY
void SpektrumBinding(void)
{
  Uart1Init();    // init Uart again
}

void push2Control(Msg_RefValsAbs *msg_RefValsAbs)
{
  signed int signal, tmp, i;
  unsigned int Channel, index;
  SenderOkay=200;
  if( (msg_RefValsAbs->ac_state&XC_ON) && MotorenEin==0)
  { // Einschalten
    modell_fliegt = 1;
    MotorenEin = 1;
    //sollGier = 0;
    Mess_Integral_Gier = 0;
    Mess_Integral_Gier2 = 0;
    Mess_IntegralNick = EE_Parameter.GyroAccFaktor * (long)Mittelwert_AccNick;
    Mess_IntegralRoll = EE_Parameter.GyroAccFaktor * (long)Mittelwert_AccRoll;
    Mess_IntegralNick2 = IntegralNick;
    Mess_IntegralRoll2 = IntegralRoll;
    //SummeNick = 0;
    //SummeRoll = 0;
    MikroKopterFlags |= FLAG_START;
  }
  if( !(msg_RefValsAbs->ac_state&XC_ON) && MotorenEin==1 )
  { // Ausschalten
    MotorenEin = 0;
    modell_fliegt = 0;
  }

  for(i =1; i <= 4; i++)
  { index=i;
    if(i==1) signal=msg_RefValsAbs->nick; else
    if(i==2) signal=msg_RefValsAbs->roll; else
    if(i==3) signal=msg_RefValsAbs->gas; else
    if(i==4) signal=msg_RefValsAbs->yaw;
                tmp = (3 * (PPM_in[index]) + signal) / 4;   
                if(tmp > signal+1) tmp--; else
                if(tmp < signal-1) tmp++; 
                if(SenderOkay >= 180)  PPM_diff[index] = ((tmp - PPM_in[index]) / 3) * 3; 
                else PPM_diff[index] = 0;
                PPM_in[index] = tmp;
  }
			// Null bedeutet: Neue Daten
			// nur beim ersten Frame (CH 0-7) setzen
  NewPpmData = 0;
} //push2Control

void pickMsgAllstates(Msg_XCallstates *msg_xcallstates)
{
    msg_xcallstates->gyro_nick	= IntegralNick / (EE_Parameter.GyroAccFaktor * 4);
    msg_xcallstates->gyro_roll	= IntegralRoll / (EE_Parameter.GyroAccFaktor * 4);
    msg_xcallstates->gyro_yaw 	= MesswertGier;
    msg_xcallstates->acc_nick	= Mittelwert_AccNick / 4;
    msg_xcallstates->acc_roll	= Mittelwert_AccRoll / 4;
    msg_xcallstates->pressure	= HoehenWert;
    msg_xcallstates->compass_yaw = KompassValue;
    msg_xcallstates->motor1 = Motor[0];
    msg_xcallstates->motor2 = Motor[1];
    msg_xcallstates->motor3 = Motor[3];
    msg_xcallstates->motor4 = Motor[2];
    msg_xcallstates->voltage = UBat;
}
//END MK_COMPATIBILITY

void UART1_SetBaudRate( uint32_t baudRate )
{
    uint32_t    divisor;
    divisor = (( SYSCLK / 8 / baudRate ) - 1 );
    // Set the baud rate
    UBRR1H = ( divisor >> 8 ) & 0xFF;
    UBRR1L = divisor & 0xFF;
} // UART1_SetBaudRate
#define BLD_UART1_DDR       DDRD
#define BLD_UART1_PORT      PORTD
#define BLD_UART1_RX_MASK   ( 1 << 2 )
#define BLD_UART1_TX_MASK   ( 1 << 3 )

void Uart1Init(void)
{
  #define UART1_DATA_BIT_8  (( 1 << UCSZ11 ) | ( 1 << UCSZ10 ))
  #define UART1_PARITY_NONE (( 0 << UPM11 )  | ( 0 << UPM10 ))
  #define UART1_STOP_BIT_1  ( 1 << USBS1 )

    // Configure TxD and RxD pins as inputs and turn off the pullups
    BLD_UART1_DDR  &= ~( BLD_UART1_RX_MASK | BLD_UART1_TX_MASK );
//#if CFG_BLD_UART1_ENABLE_TX_PULLUP
  //  BLD_UART1_PORT &= ~BLD_UART1_RX_MASK;
  //  BLD_UART1_PORT |=  BLD_UART1_TX_MASK;
//#endif
    // Initialize the UART
    UART1_SetBaudRate( 115200 );
    UCSR1A = ( 1 << U2X1 );
    UCSR1B = (( 1 << RXCIE1 ) | ( 1 << RXEN1 ));
    UCSR1C = ( UART1_DATA_BIT_8 | UART1_PARITY_NONE | UART1_STOP_BIT_1 );

    serPortDLL_init(&ser_mc);
}




void uart1sendframe()
{
    // We want to write a character, so we need to enable the transmitter
    // and disable the receiver.
    UCSR1B &= ~(( 1 << RXEN1 ) | ( 1 << RXCIE1 ));//#if !CFG_BLD_UART1_RCV_TX
    UCSR1B |= (( 1 << TXEN1 ) | ( 1 << UDRIE1 ));
}



ISR(USART1_RX_vect)
{
  static unsigned int Sync=0, FrameCnt=0, ByteHigh=0, ReSync=1, Frame2=0, FrameTimer;
  unsigned int Channel, index;
  signed int signal, tmp;
  int bCheckDelay;
  uint8_t c;
	
  c = UDR1; // get data byte
  if( serPortDLL_Rx_putbyte(&ser_mc, c) )	//4. Zeichen empfangen
  {
  }
  else					// Nachricht vollständig
  {
    if( serPortDLL_Rx_getframe(&ser_mc) ) uart1RxframeHandler();  // Data available & CRC ok-> process!
  }	
}   

//***************************************************************************
/**
*   Interrupt handler for when the data has completely left the Tx shift
*	register.
*
*	We use this to disable the transmitter and re-enable the receiver.
*/
ISR( USART1_TX_vect )
{
    // Since we were called, the data register is also empty. This means
    // that we can go ahead and disable the transmitter and re-enable the
    // receiver.
    UCSR1B &= ~(( 1 << TXEN1 ) | ( 1 << TXCIE1 )); 
    UCSR1B |=  (( 1 << RXEN1 ) | ( 1 << RXCIE1 ));
} // USART1_TX_vect

//***************************************************************************
/**
*   Interrupt handler for Uart Data Register Empty (Space available in
*   the uart to the servo).
*/
ISR( USART1_UDRE_vect )
{
  static unsigned char byte;

  if( serPortDLL_Tx_getbyte(&ser_mc, &byte) )
  { // Write the next character from the TX Buffer
    UDR1 = byte;
    // Enable the Tx Complete interrupt. This is used to turn off the
    // transmitter and enable the receiver.
    UCSR1B |= ( 1 << TXCIE1 );
  }
  else
  {
    // Nothing left to transmit, disable the transmit interrupt.
    // The transmitter will be disabled when the Tx Complete interrupt 
    // occurs.
    UCSR1B &= ~( 1 << UDRIE1 ); 
  }

    /*if ( CBUF_IsEmpty( BLD_gUart1TxBuf ))
    {
        // Nothing left to transmit, disable the transmit interrupt.
        // The transmitter will be disabled when the Tx Complete interrupt 
        // occurs.

        UCSR1B &= ~( 1 << UDRIE1 );
    }
    else
    {
        // Write the next character from the TX Buffer

        UDR1 = CBUF_Pop( BLD_gUart1TxBuf );

        // Enable the Tx Complete interrupt. This is used to turn off the
        // transmitter and enable the receiver.

        UCSR1B |= ( 1 << TXCIE1 );
    }*/

}  // USART1_UDRE_vect



//***************************************************************************
/**
*   Write a character to the UART 
*/

/*int  UART1_PutChar( char ch )
{
   while ( CBUF_IsFull( BLD_gUart1TxBuf ))
    {
        ;
    }

    CBUF_Push( BLD_gUart1TxBuf, ch );

    // We want to write a character, so we need to enable the transmitter
    // and disable the receiver.
    UCSR1B &= ~(( 1 << RXEN1 ) | ( 1 << RXCIE1 ));//#if !CFG_BLD_UART1_RCV_TX
    UCSR1B |= (( 1 << TXEN1 ) | ( 1 << UDRIE1 ));

    return 0;

} // UART1_PutChar
*/

