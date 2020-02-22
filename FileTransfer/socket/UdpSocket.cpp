#include "UdpSocket.h"
#include "protocol/Databuilder.h"
#include <QtEndian>
#include <QtConcurrent/QtConcurrent>
#include "global/defines.h"

CUdpSocket::CUdpSocket() : m_socket(INVALID_SOCKET)
{
    SetSendBufferSize(UDP_SEND_BUFFER_SIZE);
}

CUdpSocket::~CUdpSocket()
{
    InternalStop();
}

void CUdpSocket::StartClient(const QString &serverAddr, uint16_t serverPort)
{
    QMetaObject::invokeMethod(this, "InternalStartClient", Q_ARG(const QString&, serverAddr), Q_ARG(uint16_t, serverPort));
}

void CUdpSocket::StartServer(uint16_t serverPort)
{
    QMetaObject::invokeMethod(this, "InternalStartServer", Q_ARG(uint16_t, serverPort));
}

void CUdpSocket::Stop()
{
    CSocketBase::Stop();
    QMetaObject::invokeMethod(this, "InternalStop");
}

void CUdpSocket::SendData(const QByteArray &data)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(m_peerAddr.toUtf8().data());
    addr.sin_port = htons(m_peerPort);

    int64_t ret = sendto(m_socket, data.data(), data.length(), 0, (sockaddr*)&addr, sizeof (addr));
    if(-1 == ret)
    {
        emit signalError(QString("发送数据错误:%1").arg(GetLastError()));
    }
}

void CUdpSocket::InternalStartClient(const QString &serverAddr, uint16_t serverPort)
{
    m_peerAddr = serverAddr;
    m_peerPort = serverPort;
    uint64_t errCode;
    if(!CreateSocket(0, errCode))
    {
        emit signalError(QString("创建套接字失败,错误:%1").arg(errCode));
        return;
    }
    emit signalMessage("打开套接字成功");
    startSelect();
    SendHandShankReq();
}

void CUdpSocket::InternalStartServer(uint16_t serverPort)
{
    uint64_t errCode;
    if(!CreateSocket(serverPort, errCode))
    {
        emit signalError(QString("创建套接字失败,错误:%1").arg(errCode));
        return;
    }
    emit signalMessage("打开套接字成功");
    startSelect();
}

void CUdpSocket::InternalStop()
{
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_CLOSE;
    m_thread.quit();
}

void CUdpSocket::ParseData(const uint8_t *buffer, const QString& peerAddr, uint16_t peerPort)
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
        m_peerAddr = peerAddr;
        m_peerPort = peerPort;
        ProcessHandShankReq();
        break;
    case CMD_HANDSHANK_RESP:
        ProcessHandShankResp();
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

bool CUdpSocket::CreateSocket(uint16_t port, uint64_t &errCode)
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_socket == INVALID_SOCKET)
    {
        errCode = GetLastError();
        return false;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if(bind(m_socket, (sockaddr*)&addr, sizeof(addr)))
    {
        errCode = GetLastError();
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    int bufferLen = 500 * 1024 * 1024;
    if(setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&bufferLen, sizeof(int)) ||
            setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferLen, sizeof(int)))
    {
        errCode = GetLastError();
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    return true;
}

void CUdpSocket::startSelect()
{
    QtConcurrent::run([this](){
        char* buffer = new char[RECV_BUFFER_SIZE];
        fd_set readSet, exceptionSet;
        while(m_socket != INVALID_SOCKET)
        {
            FD_ZERO(&readSet);
            FD_ZERO(&exceptionSet);
            FD_SET(m_socket, &readSet);
            FD_SET(m_socket, &exceptionSet);
            if(-1 == select(m_socket+1, &readSet, nullptr, &exceptionSet, nullptr))
            {
                emit signalError(QString("套接字错误:%1").arg(GetLastError()));
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
                break;
            }
            if(FD_ISSET(m_socket, &readSet))
            {
                sockaddr_in fromAddr;
                int fromLen = sizeof(fromAddr);
                int64_t readBytes = recvfrom(m_socket, buffer, RECV_BUFFER_SIZE, 0, (sockaddr*)&fromAddr, &fromLen);
                if(readBytes == -1)
                {
                    if(GetLastError() != 10038)
                    {
                        emit signalError(QString("套接字错误:%1").arg(GetLastError()));
                    }
                    closesocket(m_socket);
                    m_socket = INVALID_SOCKET;
                    break;
                }
                if(readBytes >= sizeof (Packet))
                {
                    ParseData(reinterpret_cast<const uint8_t*>(buffer), inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
                }
            }
            if(FD_ISSET(m_socket, &exceptionSet))
            {
                emit signalError(QString("套接字错误:%1").arg(GetLastError()));
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
                break;
            }
        }
        delete[] buffer;
    });
}
