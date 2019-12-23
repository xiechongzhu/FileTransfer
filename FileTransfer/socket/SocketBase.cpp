#include "SocketBase.h"
#include "protocol/Databuilder.h"
#include "global/defines.h"
#include <QtEndian>
#include <QAbstractSocket>
#include <QFile>
#include <QFileInfo>
#include <QApplication>

CSocketBase::CSocketBase() : m_socketStatus(SOCKET_STATUS::SOCKET_STATUS_CLOSE), m_bSend(false)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<uint16_t>("uint16_t");
    moveToThread(&m_thread);
    m_thread.start();
}

CSocketBase::~CSocketBase()
{

}

void CSocketBase::Stop()
{
    m_file.close();
}

void CSocketBase::SetLocalPort(uint16_t port)
{
    Q_UNUSED(port)
}

void CSocketBase::SendFile(const QStringList &fileList)
{
    m_SendFileDataPacketCnt = 0;
    uint64_t byteSend = 0;
    uint64_t byteTotal = 0;
    foreach(const QString& fileFullName, fileList)
    {
        QFileInfo fileInfo(fileFullName);
        byteTotal += fileInfo.size();
    }
    m_bSend = true;
    foreach(const QString& fileFullName, fileList)
    {
        QFileInfo fileInfo(fileFullName);
        QString fileName = fileInfo.fileName();
        QFile file(fileFullName);
        if(file.open(QFile::ReadOnly))
        {
            emit signalMessage(QString("开始发送文件:%1").arg(fileName));
            SendData(CDataBuilder::BuildPacket(CMD_FILE_START, fileName.toUtf8()));
            while(!file.atEnd() && m_bSend)
            {
                QApplication::processEvents();
                char buffer[READ_BUFFER_SIZE] = {0};
                int64_t readBytes = file.read(buffer, READ_BUFFER_SIZE);
                if(readBytes > 0)
                {
                    SendData(CDataBuilder::BuildPacket(CMD_FILE_DATA, QByteArray(buffer, readBytes)));
                    byteSend += readBytes;
                    m_SendFileDataPacketCnt++;
                    emit signalSendFileProgressChange((float)byteSend/byteTotal*100);
                    //TODO:: Qt的套接字太垃圾，这里不休眠，发送数据就会丢包
                    QThread::currentThread()->msleep(1);
                }
                else
                {
                    emit signalError(QString("读取文件数据失败:%1,错误:%2").arg(fileName).arg(file.errorString()));
                    break;
                }
            }
            file.close();
            SendData(CDataBuilder::BuildPacket(CMD_FILE_END));
            emit signalSendFileProgressChange(100);
            emit signalMessage(QString("发送文件结束:%1").arg(fileName));
        }
        else
        {
            emit signalError(QString("打开文件失败:%1,错误:%2").arg(fileName).arg(file.errorString()));
        }
    }
    qDebug() << "发送文件数据包数:" <<m_SendFileDataPacketCnt;
    emit signalFileSendFinish();
}

void CSocketBase::StopSend()
{
    m_bSend = false;
}

void CSocketBase::ProcessHandShank()
{
    QByteArray data = CDataBuilder::BuildPacket(CMD_HANDSHANK_RESP);
    SendData(data);
    emit signalMessage("收到服务器握手请求,向服务器发送握手回应");
    m_socketStatus = SOCKET_STATUS::SOCKET_STATUS_HANDSHANK;
    emit signalHandShank();
}

void CSocketBase::ProcessFileStart(const uint8_t *packet)
{
    if(m_socketStatus != SOCKET_STATUS::SOCKET_STATUS_HANDSHANK)
    {
        return;
    }
    const Packet *pPacket = reinterpret_cast<const Packet*>(packet);
    QString strFileName = QString(RECV_FOLDER)+QString::fromUtf8(QByteArray(reinterpret_cast<const char*>(pPacket->data), qFromBigEndian(pPacket->body_length)));
    m_file.setFileName(strFileName);
    if(!m_file.open(QFile::WriteOnly))
    {
        emit signalError(QString("打开文件失败:%1,错误信息:%2").arg(strFileName).arg(m_file.errorString()));
    }
    else
    {
        emit signalMessage(QString("打开文件成功:%1").arg(strFileName));
    }
}

void CSocketBase::ProcessFileEnd()
{
    if(m_socketStatus != SOCKET_STATUS::SOCKET_STATUS_HANDSHANK)
    {
        return;
    }
    m_file.close();
    emit signalMessage(QString("关闭文件:%1").arg(m_file.fileName()));
}

void CSocketBase::ProcessFileData(const uint8_t *packet)
{
    if(m_socketStatus != SOCKET_STATUS::SOCKET_STATUS_HANDSHANK)
    {
        return;
    }
    const Packet *pPacket = reinterpret_cast<const Packet*>(packet);
    uint32_t dataLength = qFromBigEndian(pPacket->body_length);
    uint32_t writePos = 0;
    while(dataLength)
    {
        int64_t writeBytes = m_file.write(reinterpret_cast<const char*>(pPacket->data+writePos), dataLength);
        if(-1 == writeBytes)
        {
            m_file.close();
            emit signalError(QString("写入文件失败,错误原因:%1").arg(m_file.errorString()));
            break;
        }
        writePos += writeBytes;
        dataLength -= writeBytes;
    }
}
