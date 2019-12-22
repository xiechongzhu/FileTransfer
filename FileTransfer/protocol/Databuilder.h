#ifndef DATABUILDER_H
#define DATABUILDER_H

#include "global/protocol.h"
#include <QByteArray>

class CDataBuilder
{
private:
    CDataBuilder();
public:
    static QByteArray BuildPacket(uint32_t cmd);
    static QByteArray BuildPacket(uint32_t cmd, const QByteArray& data);
};

#endif // DATABUILDER_H
