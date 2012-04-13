#ifndef _QMAGELLAN_H_
#define _QMAGELLAN_H_

#include <QThread>
#include <QString>
#include <QTextStream>

typedef struct
{
	int x,y,z,a,b,c;
	bool b0,b1;
} SpaceMouseData;

class QMagellanThread: public QThread
{
	//Q_OBJECT	// Is that neccessary? Is signal/slot communication possible via threads?
	// QextSerialPort * port;
	
	protected:
		virtual void run();
		//virtual bool event(QEvent *event);
		//void keyPressEvent(QKeyEvent *event);
		//void timerEvent(QTimerEvent *event);
	
	public:

		QMagellanThread(/*QextSerialPort * port,*/ QObject * parent = 0):QThread(parent)
		{
			//this->port = port;
			//timer = new QTimer(this);
			//connect( timer, SIGNAL(timeout()), this, SLOT(send_buffer_tx()) );
			//timer->start(500);
			//timer.start(500, this);
			//serPortDLL_init(&ser_pc);
			//buffer_tx.reserve((int)DLL_SIZE_FIFO);
		}
		int x() {return x_;}
		int y() {return y_;}
		int z() {return z_;}
		int a() {return a_;}
		int b() {return b_;}
		int c() {return c_;}
		SpaceMouseData getData();
	private:
		double x_, y_, z_, a_, b_, c_;
		//QBasicTimer timer;
	//private slots:
	//	void send_buffer_tx();
}; 


#endif
