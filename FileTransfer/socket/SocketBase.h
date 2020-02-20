#ifndef CSOCKETBASE_H
#define CSOCKETBASE_H

#include <QString>
#include <QObject>
#include <QThread>
#include <QFile>

class CSendFileThread;

class CSocketBase : public QObject
{
    Q_OBJECT
public:
    enum class SOCKET_STATUS
    {
        SOCKET_STATUS_CLOSE,
        SOCKET_STATUS_CONNECTED,
        SOCKET_STATUS_HANDSHANK,
    };

    CSocketBase();
    virtual ~CSocketBase();
    virtual void StartClient(const QString& serverAddr, uint16_t serverPort) = 0;
    virtual void StartServer(uint16_t serverPort) = 0;
    virtual void Stop();
    Q_INVOKABLE void SendFile(const QStringList& fileList);
    void StopSend();
    void SetSendBufferSize(int bufferSize);
    void SendHandShankReq();
protected:
    SOCKET_STATUS m_socketStatus;
    uint64_t m_SendFileDataPacketCnt;
    void ProcessHandShankReq();
    void ProcessHandShankResp();
    void ProcessFileStart(const uint8_t* packet);
    void ProcessFileEnd();
    void ProcessFileData(const uint8_t* packet);
    Q_INVOKABLE virtual void SendData(const QByteArray& data) = 0;
    QThread m_thread;
    bool m_bSend;
private:
    QFile m_file;
    int m_sendBufferSize;
    int m_recvCount;
signals:
    void signalError(const QString& errString);
    void signalMessage(const QString& message);
    void signalClose();
    void signalHandShankReq();
    void signalHandShankResp();
    void signalFileSendFinish();
    void signalSendFileProgressChange(int progress);
};

#endif // CSOCKETBASE_H
