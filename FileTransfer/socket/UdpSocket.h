#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "SocketBase.h"
#include <windows.h>

class CUdpSocket : public CSocketBase
{
    friend class UdpThread;
    Q_OBJECT
public:
    CUdpSocket();
    virtual ~CUdpSocket() override;
    virtual void StartClient(const QString& serverAddr, uint16_t serverPort) override;
    virtual void StartServer(uint16_t serverPort) override;
    virtual void Stop() override;
protected:
    Q_INVOKABLE virtual void SendData(const QByteArray& data) override;
    Q_INVOKABLE void InternalStartClient(const QString &serverAddr, uint16_t serverPort);
    Q_INVOKABLE void InternalStartServer(uint16_t serverPort);
    Q_INVOKABLE void InternalStop();
    void ParseData(const uint8_t* buffer, const QString &peerAddr, uint16_t peerPort);
    bool CreateSocket(uint16_t port, uint64_t& errCode);
    void startSelect();
private:
    QString m_peerAddr;
    uint16_t m_peerPort;
    SOCKET m_socket;
};

#endif // UDPSOCKET_H
