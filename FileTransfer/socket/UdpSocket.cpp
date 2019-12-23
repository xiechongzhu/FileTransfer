#include "UdpSocket.h"
#include "protocol/Databuilder.h"
#include <QtEndian>

CUdpSocket::CUdpSocket()
{

}

CUdpSocket::~CUdpSocket()
{
    if(m_pSocket)
    {
        disconnect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CUdpSocket::slotSocketError);
        disconnect(m_pSocket, &QUdpSocket::readyRead, this, &CUdpSocket::slotReadyRead);
        delete m_pSocket;
    }
}

void CUdpSocket::Start(const QString &ipAddr, uint16_t port)
{
    QMetaObject::invokeMethod(this, "InternalStart", Q_ARG(const QString&, ipAddr), Q_ARG(uint16_t, port));
}

void CUdpSocket::Stop()
{
    CSocketBase::Stop();
    QMetaObject::invokeMethod(this, "InternalStop");
}

void CUdpSocket::SetLocalPort(uint16_t port)
{
    m_localPort = port;
}

void CUdpSocket::SendData(const QByteArray &data)
{
    int64_t ret = m_pSocket->writeDatagram(data, QHostAddress(m_remoteIpAddr), m_remotePort);
    if(ret != data.length())
    {
        emit signalError(QString("发送数据错误:%1").arg(m_pSocket->errorString()));
    }
    m_pSocket->waitForBytesWritten();
}

void CUdpSocket::InternalStart(const QString &ipAddr, uint16_t port)
{
    m_remoteIpAddr = ipAddr;
    m_remotePort = port;
    m_pSocket = new QUdpSocket;
    if(!m_pSocket->bind(QHostAddress(QHostAddress::Any), m_localPort))
    {
        emit signalError(QStringLiteral("绑定端口失败,错误:%1").arg(m_pSocket->errorString()));
        return;
    }
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CUdpSocket::slotSocketError);
    connect(m_pSocket, &QUdpSocket::readyRead, this, &CUdpSocket::slotReadyRead);
    emit signalMessage("打开套接字成功");
}

void CUdpSocket::InternalStop()
{
    m_pSocket->close();
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    m_thread.quit();
}

void CUdpSocket::ParseData(const uint8_t *buffer)
{
    const Packet *pPacket = reinterpret_cast<const Packet*>(buffer);
    uint32_t magicNumber = qFromBigEndian(pPacket->magic_number);
    uint32_t cmd = qFromBigEndian(pPacket->cmd);
    if(magicNumber != MAGIC_NUMBER)
    {
        emit signalError(QString::asprintf("数据头部字段错误,MAGIC_NUMBER=%x", magicNumber));
        return;
    }
    switch (cmd)
    {
    case CMD_HANDSHANK_REQ:
        ProcessHandShank();
        break;
    case CMD_FILE_START:
        ProcessFileStart(buffer);
        break;
    case CMD_FILE_END:
        ProcessFileEnd();
        break;
    case CMD_FILE_DATA:
        ProcessFileData(buffer);
        break;
    default:
        break;
    }
}

void CUdpSocket::slotSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit signalError(QStringLiteral("套接字错误:%1").arg(m_pSocket->errorString()));
}

void CUdpSocket::slotReadyRead()
{
    char buffer[1024*100] = {0};
    while(m_pSocket->hasPendingDatagrams())
    {
        int64_t readBytes = m_pSocket->readDatagram(buffer, sizeof(buffer));
        if(readBytes >= sizeof (Packet))
        {
            ParseData(reinterpret_cast<const uint8_t*>(buffer));
        }
    }
}
