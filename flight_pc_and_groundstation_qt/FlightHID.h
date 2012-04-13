/*
Wrapper class for HID.
- interface to SpaceNavigator <- this
- other joystick device
- received via bluetooth/wlan
- or Automatic flight
*/

#ifndef _FLIGHTHID_H_
#define _FLIGHTHID_H_

#include "qmagellan.h"

class FlightHID : public QMagellanThread
{
public:
	//Semaphore not necessary because data integrity unimportant
	int nick() {return z();}
	int roll() {return x();}
	int yaw()  {return b();}
	int gas()  {return y();}

private:
	//int _nick, _roll, _yaw, _gas;
};


#endif
