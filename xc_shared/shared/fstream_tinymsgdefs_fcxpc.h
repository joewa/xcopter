#ifndef _FSTREAM_TINYMSGDEFS_FCxPC_H_
#define _FSTREAM_TINYMSGDEFS_FCxPC_H_

#include <QDataStream>
#include <QFile>

#include "tinymsgdefs_fcxpc.h"

//QDataStream &operator<<(QDataStream &out, const Msg_RefAbsVals &msg);
//QDataStream &operator>>(QDataStream &in, Painting &painting);

static inline QDataStream &operator<<(QDataStream &out, Msg_RefValsAbs &msg)
{
  QByteArray qframe;
  unsigned char frame[512];
  msg_RefValsAbs2frame(&msg, frame);
  qframe.append((char*)frame, frame[0]);
  out << qframe;
  return out;
}

static inline QDataStream &operator<<(QDataStream &out, Msg_XCallstates &msg)
{
  QByteArray qframe;
  unsigned char frame[512];
  msg_XCallstates2frame(&msg, frame);
  qframe.append((char*)frame, frame[0]);
  out << qframe;
  return out;
}

static inline QDataStream &operator>>(QDataStream &in, Msg_RefValsAbs &msg)
{
    QByteArray qframe;
    unsigned char *frame;
    in >> qframe;
    frame = (unsigned char*)qframe.constData();
    frame2Msg_RefValsAbs(frame, &msg);
    return in;
}

static inline QDataStream &operator>>(QDataStream &in, Msg_XCallstates &msg)
{
    QByteArray qframe;
    unsigned char *frame;
    in >> qframe;
    frame = (unsigned char*)qframe.constData();
    frame2Msg_XCallstates(frame, &msg);
    return in;
}

#endif
