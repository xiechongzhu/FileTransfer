#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "SocketBase.h"
#include <QUdpSocket>

class CUdpSocket : public CSocketBase
{
    Q_OBJECT
public:
    CUdpSocket();
    virtual ~CUdpSocket() override;
    virtual void Start(const QString &ipAddr, uint16_t port) override;
    virtual void Stop() override;
    virtual void SetLocalPort(uint16_t port) override;
protected:
    Q_INVOKABLE virtual void SendData(const QByteArray& data) override;
    Q_INVOKABLE void InternalStart(const QString &ipAddr, uint16_t port);
    Q_INVOKABLE void InternalStop();
    void ParseData(const uint8_t* buffer);
private:
    QUdpSocket *m_pSocket;
    QString m_remoteIpAddr;
    uint16_t m_remotePort;
    uint16_t m_localPort;
protected slots:
    void slotSocketError(QAbstractSocket::SocketError socketError);
    void slotReadyRead();
};

#endif // UDPSOCKET_H
