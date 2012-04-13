#ifndef _XCCONTROL_H_
#define _XCCONTROL_H_

using namespace std;

#include "difftool.h"
#include "tinymsgdefs_fcxpc.h"
#include "FlightHID.h"

#define Angle360 900
#define res_fak 128
#define t_cycle 50


class Diff1
{
public:
  Diff1();
  long angle_mach(double angle_deg);
  int get_a_k2();
  int diff1(long angle_mach);
  int diff1(double angle_deg);
private:
  Diff1_s d1;
};

class Axis : public Diff1
{
public:
  Axis();
  Axis(double );
  void InitAngle(double _angle);
  void SetAngle(double _angle, long _k);
  double GetAngle();
  double GetAngleSpeed();
private:
  double angle_cmd;
  double angle, lastangle;
  long k_axis;
};

enum ActivityState {OFF, STREAMING, FLYING};

class XCopterCore
{
public:
  XCopterCore();
  ~XCopterCore();
  Msg_RefValsAbs calc();
  
  ActivityState activityState;
  Axis *nick, *roll, *yaw;
private:
  double period;
  double t, td, tc;
  long k, kd, kc;   //ZEITEN  
};

#endif
