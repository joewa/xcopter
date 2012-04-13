
#include "xccontrol.h"

Diff1::Diff1()
{
  diff1_init(&d1);
}
int Diff1::diff1(long angle_mach)
{  return ::diff1(&d1, angle_mach);
}
long Diff1::angle_mach(double angle_deg)
{  return (long)(angle_deg*Angle360/360*res_fak);
}
int Diff1::diff1(double angle_deg)
{  return diff1( angle_mach(angle_deg) );
}


Axis::Axis()
{
  angle=0; angle_cmd=0; lastangle =0;
  k_axis=0;
}

Axis::Axis(double startangle)
{
  angle=startangle; angle_cmd=startangle; lastangle = startangle;
   k_axis=0;
}

void Axis::InitAngle(double _angle)
{
  angle = _angle; angle_cmd = _angle; lastangle = _angle;
  k_axis=0;
  diff1(_angle); diff1(_angle);
}

void Axis::SetAngle(double _angle, long _k)
{
	if( _k > k_axis )
	{
		k_axis = _k;
		lastangle = angle;
	}
	angle = _angle;
}

double Axis::GetAngle()
{
	return angle;
}

double Axis::GetAngleSpeed()	//ACHTUNG: ist NICHT der richtige Wert
{								//aber ausreichend zum �berschleifen
	return ( angle - lastangle ) / (t_cycle*1000);
}


XCopterCore::XCopterCore()
{
  k=0; kc=0;  //Zeit auf 0 setzen
  t=0; tc=0;
  period=t_cycle;
  nick = new Axis;
  roll = new Axis;
  yaw  = new Axis;
}

XCopterCore::~XCopterCore()
{
  delete nick;
  delete roll;
  delete yaw;
}

Msg_RefValsAbs XCopterCore::calc()
{
  Msg_RefValsAbs msg_RefValsAbs;
}

