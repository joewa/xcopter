#ifndef XCTYPES_H
#define XCTYPES_H

#include <string.h>
#include <QtGlobal>

#define int8_t      qint8
#define uint8_t     quint8
#define int16_t     qint16
#define uint16_t    quint16
#define int32_t     qint32
#define uint32_t    quint32
#define int64_t     qint64
#define uint64_t    quint64

static inline void serialize( void * destination, const void * source, size_t num )
{
    unsigned char * pdestination = (unsigned char*)destination;
    memcpy( (unsigned char*)destination + 1, source, num);
    *pdestination += num;
} // verbessern durch stream-konzept

static inline void deserialize( void * destination, const void * source, size_t num )
{
    memcpy( destination, source, num);
}


#endif // XCTYPES_H
