#ifndef _SERIALTHREAD_H_
#define _SERIALTHREAD_H_

//extern "C" {
  #include "tinycomm.h"
  #include "tinymsgdefs_fcxpc.h"
//}
#include "FlightHID.h"

#if defined __cplusplus
#include <QString>
#include <QTextStream>
#include <QThread>
#include <QBasicTimer>
#include <QEvent>
#include <QKeyEvent>
#include <qextserialport.h>
#include "fstream_tinymsgdefs_fcxpc.h"
#endif

typedef struct
{
  double t_sample;		// Sampletime [ms]
  double k_p, k_id, k_dd;	// Controller Gains P,I,D
  double ui;			// Integrator
  double upperlim, lowerlim;	// Upper and lower limits
  double u;			// Controller output value
  unsigned long k;		// Step #
} PID_t;

void pid_init(PID_t *ctrl, double ts, double p, double i, double d, double umax, double umin);
void pid_setint(PID_t *ctrl, double ui_);
double pid_ctrl(PID_t *ctrl, double e, unsigned long k_);



// SerialThread kann RealTimeDatabase sein
class SerialThread: public QThread
{
	Q_OBJECT	// Try to implement with events and without signals & slots
	QextSerialPort * port;
	
	protected:
		//virtual void run();
		//virtual bool event(QEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void timerEvent(QTimerEvent *event);
	
	public:
		QByteArray buffer_tx;
		QByteArray buffer_rx;
		Msg_RefValsAbs pc_msg_refvals;
		Msg_XCallstates msg_xcallstates;
		SerPortDLL_t ser_pc;
		PID_t pi_gas;
		QFile logfile;
		unsigned char ac_state;
		
		void assignFlightHID(FlightHID * flightHID);
		int getNextRxFrame();
		void writelog();

                SerialThread(QextSerialPort * port, QObject * parent = 0);//:QThread(parent)

		~SerialThread()
		{
			logfile.close();
		}
	private:
		//QTimer *timer;
		FlightHID *fHID;
		QBasicTimer timer;
		unsigned long k;
	//private slots:
	//	void send_buffer_tx();
	public slots:
		void ac_onoff(int on);
		void stream_onoff(int on);
}; 

/*QDataStream &operator<<(QDataStream &out, const Msg_RefValsAbs &msg_refvalsabs);
QDataStream &operator>>(QDataStream &in, Msg_RefValsAbs &msg_refvalsabs);*/

#endif
