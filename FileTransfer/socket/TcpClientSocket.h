#ifndef CTCPCLIENTSOCKET_H
#define CTCPCLIENTSOCKET_H

#include "SocketBase.h"
#include <QTcpSocket>

class CTcpClientSocket : public CSocketBase
{
    Q_OBJECT
public:
    static const uint32_t MAX_BUFFER_LENGTH = 1024 * 1024 *16;
    CTcpClientSocket();
    virtual ~CTcpClientSocket();
    virtual void Start(const QString &ipAddr, uint16_t port) override;
    virtual void Stop() override;
protected:
    Q_INVOKABLE virtual void SendData(const QByteArray& data) override;
    void ParseData();
    Q_INVOKABLE void InternalStart(const QString &ipAddr, uint16_t port);
    Q_INVOKABLE void InternalStop();
private:
    QTcpSocket *m_pSocket;
    uint8_t m_dataBuffer[MAX_BUFFER_LENGTH];
    uint32_t m_dataBufferPos;
protected slots:
    void slotConnected();
    void slotSocketError(QAbstractSocket::SocketError socketError);
    void slotDoisconnected();
    void slotReadyRead();
};

#endif // CTCPCLIENTSOCKET_H
