#include "TcpClientSocket.h"
#include "protocol/Databuilder.h"
#include <QtEndian>
#include <QDebug>

CTcpClientSocket::CTcpClientSocket() : m_dataBufferPos(0)
{
    qRegisterMetaType<uint16_t>("uint16_t");
}

CTcpClientSocket::~CTcpClientSocket()
{
    delete m_pSocket;
}

void CTcpClientSocket::Start(const QString &ipAddr, uint16_t port)
{
    QMetaObject::invokeMethod(this, "InternalStart", Q_ARG(const QString&, ipAddr), Q_ARG(uint16_t, port));
}

void CTcpClientSocket::Stop()
{
    CSocketBase::Stop();
    QMetaObject::invokeMethod(this, "InternalStop");
}

void CTcpClientSocket::SendData(const QByteArray &data)
{
    int64_t bytesToWrite = data.length();
    uint64_t pos = 0;
    const char* pData = data.data();
    while(bytesToWrite)
    {
        int64_t ret = m_pSocket->write(pData+pos, bytesToWrite);
        m_pSocket->waitForBytesWritten();
        if(-1 == ret)
        {
            emit signalError(QString("发送数据错误:%1").arg(m_pSocket->errorString()));
            return;
        }
        bytesToWrite -= ret;
        pos += ret;
    }
}

void CTcpClientSocket::ParseData()
{
    while(1)
    {
        if(m_dataBufferPos < sizeof(Packet))
        {
            break;
        }
        Packet *pPacket = reinterpret_cast<Packet*>(m_dataBuffer);
        uint32_t magicNumber = qFromBigEndian(pPacket->magic_number);
        uint32_t cmd = qFromBigEndian(pPacket->cmd);
        uint32_t bodyLength = qFromBigEndian(pPacket->body_length);
        if(magicNumber != MAGIC_NUMBER)
        {
            emit signalError(QString::asprintf("数据头部字段错误,MAGIC_NUMBER=%x", magicNumber));
            break;
        }
        uint32_t packetLength = sizeof(Packet)+bodyLength;
        if(m_dataBufferPos >= packetLength)
        {
            switch (cmd)
            {
            case CMD_HANDSHANK_REQ:
                ProcessHandShank();
                break;
            case CMD_FILE_START:
                ProcessFileStart(m_dataBuffer);
                break;
            case CMD_FILE_END:
                ProcessFileEnd();
                break;
            case CMD_FILE_DATA:
                ProcessFileData(m_dataBuffer);
            default:
                break;
            }
            memcpy(m_dataBuffer + packetLength, m_dataBuffer, m_dataBufferPos - packetLength);
            m_dataBufferPos -= packetLength;
        }
        else
        {
            break;
        }
    }
}

void CTcpClientSocket::InternalStart(const QString &ipAddr, uint16_t port)
{
    m_pSocket = new QTcpSocket;
    connect(m_pSocket, &QTcpSocket::connected, this, &CTcpClientSocket::slotConnected);
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CTcpClientSocket::slotSocketError);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &CTcpClientSocket::slotDoisconnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &CTcpClientSocket::slotReadyRead);
    m_pSocket->connectToHost(ipAddr, port);
}

void CTcpClientSocket::InternalStop()
{
    m_pSocket->close();
    m_dataBufferPos = 0;
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    m_thread.quit();
}

void CTcpClientSocket::slotConnected()
{
    emit signalMessage("连接服务器成功");
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CONNECTED;
}

void CTcpClientSocket::slotSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    switch(m_socketStatus)
    {
    case SOCKET_STATUS::SOCKET_STATUS_CLOSE:
        emit signalError(QStringLiteral("连接服务器失败:") + m_pSocket->errorString());
        break;
    default:
        break;
    }
}

void CTcpClientSocket::slotDoisconnected()
{
    emit signalMessage("与服务器断开连接");
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    emit signalClose();
}

void CTcpClientSocket::slotReadyRead()
{
    while(m_pSocket->bytesAvailable())
    {
        int64_t readBytes = m_pSocket->read(reinterpret_cast<char*>(m_dataBuffer+m_dataBufferPos), MAX_BUFFER_LENGTH-m_dataBufferPos);
        if(-1 == readBytes)
        {
            emit signalError(QString("读取套接字数据错误:%1").arg(m_pSocket->errorString()));
            return;
        }
        m_dataBufferPos += readBytes;
        ParseData();
    }
}
