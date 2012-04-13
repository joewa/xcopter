/**
 * @file main.cpp
 * @brief Main file.
 * @author Joerg Wangemann
 */

//#include <QCoreApplication> //event-loop for shell-apps
#include <QApplication> // or this
#include <QString>
#include <QTextStream>
#include <QThread>
#include <qextserialport.h>
//#include "PortListener.h"

#include "serialthread.h"
#include "FlightHID.h"

#include "tinycomm.h"
#include "tinymsgdefs_fcxpc.h"
#include "fstream_tinymsgdefs_fcxpc.h"

#include "Forms/dlg_flightinput.h"


int main(int argc, char *argv[])
{
	QTextStream out(stdout);
	//QCoreApplication app(argc, argv);
	QApplication app(argc, argv);
        if(argc<1) argv[1]="/dev/ttyUSB0";
	out<<"Opening devive "<< argv[1] <<endl;
	QextSerialPort * port = new QextSerialPort(argv[1], QextSerialPort::Polling);
	port->setBaudRate(BAUD115200);
	port->setFlowControl(FLOW_OFF);
	port->setParity(PAR_NONE);
	port->setDataBits(DATA_8);
	port->setStopBits(STOP_1);
	/*port->open(QIODevice::ReadWrite);
	if (!(port->lineStatus() & LS_DSR)) {
		out << "warning: device is not turned on" << endl;
	}*/

/*	PortListener * listener = new PortListener(port);
	//Because port and listener are working in different threads 
	//Qt will choose queued connection. This may produce clusters of 
	//signals waiting in the queue.
	listener->connect(port, SIGNAL(readyRead()), listener, SLOT(receive()));
	listener->connect(port, SIGNAL(bytesWritten(qint64)), listener, SLOT(reportWritten(qint64)));
	listener->connect(port, SIGNAL(aboutToClose()), listener, SLOT(reportClose()));
        listener->connect(port, SIGNAL(dsrChanged(bool)), listener, SLOT(reportDsr(bool)));*/
	
	SerialThread * thread = new SerialThread(port);
	app.connect(thread, SIGNAL(finished()), & app, SLOT(quit()));
	thread->start();


	/*QMagellanThread * qmagellanthread = new QMagellanThread;
	qmagellanthread->start();
	app.connect(qmagellanthread, SIGNAL(finished()), &app, SLOT(quit()));*/
	FlightHID * flightHID = new FlightHID;
	flightHID->start();
	app.connect(flightHID, SIGNAL(finished()), &app, SLOT(quit()));
	thread->assignFlightHID(flightHID);
	
	DlgFlightInput *dlgFlightInput = new DlgFlightInput;
	dlgFlightInput->show();
	thread->connect( dlgFlightInput->checkBox_Stream,SIGNAL(stateChanged(int)), thread, SLOT(ac_onoff(int)) );
	thread->connect( dlgFlightInput->checkBox_OnOff,SIGNAL(stateChanged(int)), thread, SLOT(stream_onoff(int)) );
	//event loop is required to utilize signals and slots mechanism 
	return app.exec();
}
