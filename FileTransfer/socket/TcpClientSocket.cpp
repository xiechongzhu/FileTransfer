#include "TcpClientSocket.h"
#include "protocol/Databuilder.h"
#include <QtEndian>
#include <QDebug>
#include "global/defines.h"

CTcpClientSocket::CTcpClientSocket() : m_pSocket(nullptr), m_dataBufferPos(0)
{
    SetSendBufferSize(TCP_SEND_BUFFER_SIZE);
}

CTcpClientSocket::~CTcpClientSocket()
{
    InternalStop();
}

void CTcpClientSocket::StartClient(const QString &serverAddr, uint16_t serverPort)
{
    QMetaObject::invokeMethod(this, "InternalStartClient", Q_ARG(const QString&, serverAddr), Q_ARG(uint16_t, serverPort));
}

void CTcpClientSocket::StartServer(uint16_t serverPort)
{
    Q_UNUSED(serverPort)
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
    while(bytesToWrite && m_pSocket)
    {
        int64_t ret = m_pSocket->write(pData+pos, bytesToWrite);
        if(-1 == ret)
        {
            emit signalError(QString("发送数据错误:%1").arg(m_pSocket->errorString()));
            return;
        }
        bytesToWrite -= ret;
        pos += ret;
        m_pSocket->waitForBytesWritten();
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
                ProcessHandShankReq();
                break;
            case CMD_HANDSHANK_RESP:
                ProcessHandShankResp();
                break;
            case CMD_FILE_START:
                ProcessFileStart(m_dataBuffer);
                break;
            case CMD_FILE_END:
                ProcessFileEnd();
                break;
            case CMD_FILE_DATA:
                ProcessFileData(m_dataBuffer);
                break;
            default:
                break;
            }
            memcpy(m_dataBuffer, m_dataBuffer + packetLength, m_dataBufferPos - packetLength);
            m_dataBufferPos -= packetLength;
        }
        else
        {
            break;
        }
    }
}

void CTcpClientSocket::InternalStartClient(const QString &ipAddr, uint16_t port)
{
    m_pSocket = new QTcpSocket;
    m_pSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    connect(m_pSocket, &QTcpSocket::connected, this, &CTcpClientSocket::slotConnected);
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CTcpClientSocket::slotSocketError);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &CTcpClientSocket::slotDisconnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &CTcpClientSocket::slotReadyRead);
    m_pSocket->connectToHost(ipAddr, port);
}

void CTcpClientSocket::InternalStop()
{
    if(m_pSocket)
    {
        m_pSocket->close();
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    m_dataBufferPos = 0;
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    m_thread.quit();
}

void CTcpClientSocket::slotConnected()
{
    emit signalMessage("连接服务器成功");
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CONNECTED;
    SendHandShankReq();
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
        emit signalError(QStringLiteral("套接字错误:%1").arg(m_pSocket->errorString()));
        break;
    }
}

void CTcpClientSocket::slotDisconnected()
{
    emit signalMessage("与服务器断开连接");
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    emit signalClose();
}

void CTcpClientSocket::slotReadyRead()
{
    while(m_pSocket->bytesAvailable())
    {
        int64_t readBytes = m_pSocket->read(reinterpret_cast<char*>(m_dataBuffer+m_dataBufferPos), RECV_BUFFER_SIZE-m_dataBufferPos);
        if(-1 == readBytes)
        {
            emit signalError(QString("读取套接字数据错误:%1").arg(m_pSocket->errorString()));
            return;
        }
        m_dataBufferPos += readBytes;
        ParseData();
    }
}
