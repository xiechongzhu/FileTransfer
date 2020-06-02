#include "TcpServerSocket.h"
#include <QTcpSocket>
#include "protocol/Databuilder.h"
#include <QtEndian>
#include <QDebug>
#include "global/defines.h"

CTcpServerSocket::CTcpServerSocket() : m_pServer(nullptr), m_pSocket(nullptr), m_dataBufferPos(0)
{
    SetSendBufferSize(TCP_SEND_BUFFER_SIZE);
}

CTcpServerSocket::~CTcpServerSocket()
{
    InternalStop();
}

void CTcpServerSocket::StartClient(const QString &serverAddr, uint16_t serverPort)
{
    Q_UNUSED(serverAddr)
    Q_UNUSED(serverPort)
}

void CTcpServerSocket::StartServer(uint16_t serverPort)
{
    QMetaObject::invokeMethod(this, "InternalStartServer", Q_ARG(uint16_t, serverPort));
}

void CTcpServerSocket::Stop()
{
    CSocketBase::Stop();
    QMetaObject::invokeMethod(this, "InternalStop");
}

void CTcpServerSocket::SendData(const QByteArray &data)
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

void CTcpServerSocket::ParseData()
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

void CTcpServerSocket::InternalStartServer(uint16_t port)
{
    m_pServer = new QTcpServer;
    if(!m_pServer->listen(QHostAddress::AnyIPv4, port))
    {
        emit signalError(QString("打开套接字失败:%1").arg(m_pServer->errorString()));
        delete m_pServer;
        return;
    }
    emit signalMessage("打开套接字成功");
    connect(m_pServer, &QTcpServer::newConnection, this, &CTcpServerSocket::slotNewConnection);
}

void CTcpServerSocket::InternalStop()
{
    if(m_pSocket)
    {
        m_pSocket->close();
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    if(m_pServer)
    {
        m_pServer->close();
        delete m_pServer;
        m_pServer = nullptr;
    }
    m_dataBufferPos = 0;
    m_thread.quit();
}

void CTcpServerSocket::slotNewConnection()
{
    QTcpSocket *pSock = m_pServer->nextPendingConnection();
    if(m_pSocket)
    {
        pSock->close();
        return;
    }
    m_pSocket = pSock;
    m_pSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CTcpServerSocket::slotSocketError);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &CTcpServerSocket::slotDisconnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &CTcpServerSocket::slotReadyRead);
    emit signalMessage(QStringLiteral("客户端连接成功,IP=%1, PORT=%2").arg(m_pSocket->peerAddress().toString()).arg(m_pSocket->peerPort()));
}

void CTcpServerSocket::slotSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    if(socketError != QAbstractSocket::RemoteHostClosedError)
    {
        emit signalError(QStringLiteral("套接字错误:%1").arg(m_pSocket->errorString()));
    }
}

void CTcpServerSocket::slotDisconnected()
{
    emit signalMessage("客户端断开连接");
    if(m_pSocket)
    {
        m_pSocket->close();
        m_pSocket = nullptr;
    }
    m_dataBufferPos = 0;
}

void CTcpServerSocket::slotReadyRead()
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
