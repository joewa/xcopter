#include "serialthread.h"

//#include "tinycomm.c" //"tinycomm.h" linken klappt nicht :(

/*SerialThread::SerialThread(QextSerialPort * port, QObject * parent = 0):QThread(parent)
{
	this->port = port;
	//serPortDLL_init(&pc);
	//buffer_tx.reserve((int)DLL_SIZE_FIFO);
}*/
SerialThread::SerialThread(QextSerialPort * port, QObject * parent)
{
        this->port = port;
        k = 0;
        //timer = new QTimer(this);
        //connect( timer, SIGNAL(timeout()), this, SLOT(send_buffer_tx()) );
        //timer->start(500);
        //timer.start(500, this);
        serPortDLL_init(&ser_pc);
        buffer_tx.reserve((int)DLL_SIZE_FIFO);
        buffer_rx.reserve((int)DLL_SIZE_FIFO);
        ac_state = 0;
        pid_init(&pi_gas, 20, 1, 0.125, 0, 120, -120);
        logfile.setFileName("serialthread.log");
        logfile.open(QIODevice::WriteOnly);
}

void pid_init(PID_t *ctrl, double ts, double p, double i, double d, double umax, double umin)
{
  ctrl->ui	= 0;
  ctrl->k	= 0;
  ctrl->u	= 0;
  ctrl->t_sample= ts;
  ctrl->k_p	= p;
  ctrl->k_id	= i * ts/1000;
  ctrl->k_dd	= d;
  ctrl->upperlim= umax;
  ctrl->lowerlim= umin;
}

void pid_setint(PID_t *ctrl, double ui_)
{
  ctrl->ui = ui_;
}

double pid_ctrl(PID_t *ctrl, double e, unsigned long k_)
{
  double uc, ec;
  if(k_ > ctrl->k) // Output must be calculated for this timestep
  {
    // Controller has been calculated at this timestep
    ctrl->k = k_;
    // Calculate output value
    ctrl->u = ctrl->k_p * e + ctrl->k_id * e + ctrl->ui;
    // Check limits
    if( ctrl->u > ctrl->upperlim )
    {
      uc = ctrl->u - ctrl->upperlim;
      ec = (ctrl->u + uc - ctrl->ui - (ctrl->k_p + ctrl->k_id) * e) / (ctrl->k_p + ctrl->k_id);
      e = e - ec;
    }
    else if( ctrl->u < ctrl->lowerlim )
    {
      uc = ctrl->u - ctrl->lowerlim;
      ec = (ctrl->u + uc - ctrl->ui - (ctrl->k_p + ctrl->k_id) * e) / (ctrl->k_p + ctrl->k_id);
      e = e - ec;
    }
    // (Re)calculate output value
    ctrl->u = ctrl->k_p * e + ctrl->k_id * e + ctrl->ui;
    // Calculate new integator state
    ctrl->ui += ctrl->k_id * e;
  }
  return ctrl->u;
}


void SerialThread::assignFlightHID(FlightHID * flightHID)
{
	fHID = flightHID;
}

/**BEGIN logging stuff */
void SerialThread::writelog()
{
  QDataStream out(&logfile);
  out << pc_msg_refvals << msg_xcallstates;
}

/**END logging stuff */

int SerialThread::getNextRxFrame() //Anzahl vorhandener Frames
{
  char data[1024];
  char byte;
  int i = 0, bytesRead = 0;  
  bytesRead = port->bytesAvailable();
  bytesRead = port->readLine(data, 1024);
  buffer_rx.append(data, bytesRead);
     //bytesRead = port->readLine( (char*)ser_pc.buf_rx, DLL_SIZE_FIFO); // buffer_rx
  for(i=0; i < buffer_rx.size(); i++)
  {
    byte = buffer_rx.at(0); // get data byte
    buffer_rx.remove(0,1);
    if( serPortDLL_Rx_putbyte(&ser_pc, byte) )	//4. Zeichen empfangen
    {
    }
    else					// Nachricht vollständig
    {      
      if( serPortDLL_Rx_getframe(&ser_pc) ) return 1;	// Data available -> process!
    }
  }
  return 0;
}

void SerialThread::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == timer.timerId())
  { 
     k++;
     /** Pilot-Eingaben lesen */
     QTextStream out(stdout);
     int nick, roll, yaw, gas;
     static unsigned char i;
     static unsigned char byte = 0;
     nick = fHID->nick()/3; roll = -fHID->roll()/3;
     yaw  = fHID->yaw()/3;
     gas  = pid_ctrl(&pi_gas, fHID->gas()/3, k);
     /** Ist-Werte lesen */
     msg_xcallstates.gyro_roll=0;
     if( getNextRxFrame() > 0 )
     {
       // erst MsgID lesen und Dekodierfunktion aufrufen
       frame2Msg_XCallstates(ser_pc.frame_rx, &msg_xcallstates);
     }
     
     if(i > 0) {
      // out << " Nick:"<<nick<<" Roll:"<<roll
      //     << " Yaw: "<<yaw <<" Gas: "<<gas<<" ac_state:"<<ac_state
      //     <<endl;
       out << "XC_States: Voltage: " << msg_xcallstates.voltage
               << " Roll:"<< msg_xcallstates.gyro_roll << endl;
       i = 0;
     } i++;
     /** Regler aufrufen*/
     /** Stellwerte codieren*/
     pc_msg_refvals.ac_state = ac_state;
     pc_msg_refvals.nick = nick;
     pc_msg_refvals.roll = roll;
     pc_msg_refvals.yaw = yaw;
     pc_msg_refvals.gas = gas;
     msg_RefValsAbs2frame(&pc_msg_refvals, ser_pc.frame_tx);
     serPortDLL_Tx_setframe(&ser_pc);
     while( serPortDLL_Tx_getbyte(&ser_pc, &byte) ) buffer_tx.append(byte);
     //QString bytestream(
     // Stellwerte senden
     port->write(buffer_tx.constData(), buffer_tx.length());
	//char *data = buffer_tx.data();
	//for(i=0; i < buffer_tx.length(); i++) out << (int)data[i] << endl;
     buffer_tx.clear();

        //timer.start(500, this);
     writelog();
  } else
  {
         QThread::timerEvent(event);
  }
}

void SerialThread::keyPressEvent(QKeyEvent *event)
{
    QTextStream out(stdout);
    switch (event->key()) {
    case Qt::Key_1:
        pc_msg_refvals.ac_state++;
        out << pc_msg_refvals.ac_state << "HAALLO";
        break;

    default:
        SerialThread::keyPressEvent(event);
    }
}


//SLOTS
void SerialThread::ac_onoff(int on)
{ // Ein/Ausschalten über Flag; bei Empfangsausfall notlanden
	QTextStream out(stdout);
	if(on)
	{
		out << "opening port... ";
		port->open(QIODevice::ReadWrite);
		if (!(port->lineStatus() & LS_DSR)) {
			out << "warning: device is not turned on" << endl;
		}
		timer.start(20, this);
		serPortDLL_init(&ser_pc);
	} else
	{
		timer.stop();
		out << "requesting close of the serial port... " << endl; 
		port->close();
	}
}

void SerialThread::stream_onoff(int on)
{
  pid_setint(&pi_gas, -120);
  if( ac_state&XC_ON ) ac_state = 0;
  else ac_state = ac_state | XC_ON;
}

/*bool SerialThread::event(QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Up)
		{
			pc_msg_refvals.ac_state++;
			return true;
		}
	}
	return QThread::event(event);
}*/


/*void SerialThread::send_buffer_tx()
{
	QTextStream out(stdout);
	out << pc_msg_refvals.ac_state << "x";
	unsigned char byte;
	serPortDLL_Tx_setframe(&ser_pc);
	while( serPortDLL_Tx_getbyte(&ser_pc, &byte) ) buffer_tx.append(byte);
	port->write(buffer_tx.constData(), buffer_tx.length());
}*/

/*void SerialThread::run()
{       // Signals and Slots funktionieren nicht, wenn run() neu implementiert ist
	pc_msg_refvals.dnick = 27;
	pc_msg_refvals.droll = -22;
	pc_msg_refvals.dgier = 65;
	pc_msg_refvals.gas   = 198;
	pc_msg_refvals.ac_state = 0x01;
	QString inStr;
	QTextStream in(stdin);
	QTextStream out(stdout);
	unsigned char byte = 0;
	int i;
	bool ok;
	//QByteArray buffer_tx;
	buffer_tx.reserve(DLL_SIZE_FIFO);
	serPortDLL_init(&ser_pc);
	
	out << "Special commands: open close bye" << endl << endl;
	while(inStr.compare("bye") != 0) {
		in >> inStr;
		if (inStr.compare("close") == 0) {
			out << "requesting close... " << endl; 
			port->close();
		}
		else if (inStr.compare("open") == 0) 
		{
			out << "opening port... ";
			out << port->open(QIODevice::ReadWrite);
			out << endl;
		}
		else if (inStr.compare("start") == 0) 
		{
			out << "start timer... " << endl;
			timer->start(20);
			send_buffer_tx();
		} 
		else if (inStr.compare("stop") == 0) 
		{
			out << "stop timer... " << endl;
			timer->stop();
		}
		else if (inStr.compare("bye") != 0)
			{
				i = inStr.toInt(&ok, 10);
				if(ok)
				{
					pc_msg_refvals.ac_state = i;
					msg_RefVals2frame(&pc_msg_refvals, ser_pc.frame_tx);
					serPortDLL_Tx_setframe(&ser_pc);
					while( serPortDLL_Tx_getbyte(&ser_pc, &byte) ) buffer_tx.append(byte);
					//QString bytestream(
					port->write(buffer_tx.constData(), buffer_tx.length());
					out << "Sending:";
					char *data = buffer_tx.data();
					for(i=0; i < buffer_tx.length(); i++) out << (int)data[i] << endl;
					buffer_tx.clear();
				}
			}
	}
	port->close();
}*/
