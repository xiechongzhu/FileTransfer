#ifndef CTCPCLIENTSOCKET_H
#define CTCPCLIENTSOCKET_H

#include "SocketBase.h"
#include <QTcpSocket>
#include "global/defines.h"

class CTcpClientSocket : public CSocketBase
{
    Q_OBJECT
public:
    CTcpClientSocket();
    virtual ~CTcpClientSocket() override;
    virtual void StartClient(const QString& serverAddr, uint16_t serverPort) override;
    virtual void StartServer(uint16_t serverPort) override;
    virtual void Stop() override;
protected:
    Q_INVOKABLE virtual void SendData(const QByteArray& data) override;
    void ParseData();
    Q_INVOKABLE void InternalStartClient(const QString &ipAddr, uint16_t port);
    Q_INVOKABLE void InternalStop();
private:
    QTcpSocket *m_pSocket;
    uint8_t m_dataBuffer[RECV_BUFFER_SIZE];
    uint32_t m_dataBufferPos;
protected slots:
    void slotConnected();
    void slotSocketError(QAbstractSocket::SocketError socketError);
    void slotDoisconnected();
    void slotReadyRead();
};

#endif // CTCPCLIENTSOCKET_H
