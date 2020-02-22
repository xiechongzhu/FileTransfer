#ifndef TCPSERVERSOCKET_H
#define TCPSERVERSOCKET_H

#include "SocketBase.h"
#include <QTcpServer>
#include "global/defines.h"

class CTcpServerSocket : public CSocketBase
{
    Q_OBJECT
public:
    CTcpServerSocket();
    virtual ~CTcpServerSocket() override;
    virtual void StartClient(const QString& serverAddr, uint16_t serverPort) override;
    virtual void StartServer(uint16_t serverPort) override;
    virtual void Stop() override;
protected:
    Q_INVOKABLE virtual void SendData(const QByteArray& data) override;
    void ParseData();
    Q_INVOKABLE void InternalStartServer(uint16_t port);
    Q_INVOKABLE void InternalStop();
private:
    QTcpServer *m_pServer;
    QTcpSocket *m_pSocket;
    uint8_t m_dataBuffer[RECV_BUFFER_SIZE];
    uint32_t m_dataBufferPos;
protected slots:
    void slotNewConnection();
    void slotSocketError(QAbstractSocket::SocketError socketError);
    void slotDisconnected();
    void slotReadyRead();
};

#endif // TCPSERVERSOCKET_H
