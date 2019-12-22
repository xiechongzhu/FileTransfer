#include "DataBuilder.h"
#include <QtEndian>

CDataBuilder::CDataBuilder()
{
}

QByteArray CDataBuilder::BuildPacket(uint32_t cmd)
{
    Packet packet;
    packet.magic_number = qToBigEndian(MAGIC_NUMBER);
    packet.cmd = qToBigEndian(cmd);
    packet.body_length = 0;
    return QByteArray(reinterpret_cast<const char*>(&packet), sizeof(Packet));
}

QByteArray CDataBuilder::BuildPacket(uint32_t cmd, const QByteArray &data)
{
    QByteArray array;
    array.resize(sizeof(Packet)+data.length());
    uint8_t *pData = reinterpret_cast<uint8_t*>(array.data());
    Packet *pPacket = reinterpret_cast<Packet*>(pData);
    pPacket->magic_number = qToBigEndian(MAGIC_NUMBER);
    pPacket->cmd = qToBigEndian(cmd);
    pPacket->body_length = qToBigEndian(data.length());
    memcpy(pData+sizeof(Packet), data.data(), data.length());
    return array;
}
