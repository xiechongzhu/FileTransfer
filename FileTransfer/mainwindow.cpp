#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "socket/TcpClientSocket.h"
#include "socket/TcpServerSocket.h"
#include "socket/UdpSocket.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_pSocket(nullptr)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(1, 1), &wsaData);
    ui->setupUi(this);
    InitWidgets();
    addLog(LOG_LEVEL::LOG_LEVEL_INFO, "程序启动成功");
}

MainWindow::~MainWindow()
{
    delete ui;
    WSACleanup();
}

void MainWindow::InitWidgets()
{
    ui->cbProtocol->addItem("TCP文件客户端", PROTOCOL_TCP_CLIENT);
    ui->cbProtocol->addItem("UDP文件客户端", PROTOCOL_UDP_CLIENT);
    ui->cbProtocol->addItem("TCP文件服务器", PROTOCOL_TCP_SERVER);
    ui->cbProtocol->addItem("UDP文件服务器", PROTOCOL_UDP_SERVER);

    ui->btnDisconnect->setEnabled(false);
    ui->btnSendFiles->setEnabled(false);
    ui->btnStopSend->setEnabled(false);

    ui->logTableView->setModel(&m_logModel);
    ui->logTableView->setColumnWidth(0, 140);
    ui->logTableView->setColumnWidth(1, 50);
    ui->logTableView->setColumnWidth(2, 600);
}

void MainWindow::addLog(LOG_LEVEL logLevel, const QString &logMsg)
{
    m_logModel.add(logLevel, logMsg);
    ui->logTableView->scrollToBottom();
}

void MainWindow::OpenSocket(int socketType)
{
    switch(socketType)
    {
    case PROTOCOL_TCP_CLIENT:
        m_pSocket->StartClient(ui->editIpAddr->text(), ui->editServerPort->text().toUShort());
        break;
    case PROTOCOL_UDP_CLIENT:
        m_pSocket->StartClient(ui->editIpAddr->text(), ui->editServerPort->text().toUShort());
        break;
    case PROTOCOL_TCP_SERVER:
        m_pSocket->StartServer(ui->editServerPort->text().toUShort());
        break;
    case PROTOCOL_UDP_SERVER:
        m_pSocket->StartServer(ui->editServerPort->text().toUShort());
        break;
    default:
        break;
    }
}

void MainWindow::on_btnAddFiles_clicked()
{
    QStringList selectedFileList = QFileDialog::getOpenFileNames();
    foreach(auto fileName, selectedFileList)
    {
        new QListWidgetItem(fileName, ui->fileListWidget);
    }
}

void MainWindow::on_btnDelFiles_clicked()
{
    foreach(auto pItem, ui->fileListWidget->selectedItems())
    {
        delete pItem;
    }
}

void MainWindow::on_btnClearFiles_clicked()
{
    ui->fileListWidget->clear();
}

void MainWindow::on_btnConnect_clicked()
{
    ui->btnConnect->setEnabled(false);
    ui->btnDisconnect->setEnabled(true);
    if(m_pSocket)
    {
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    switch(ui->cbProtocol->currentData().toInt())
    {
    case PROTOCOL_TCP_CLIENT:
        m_pSocket = new CTcpClientSocket;
        break;
    case PROTOCOL_UDP_CLIENT:
        m_pSocket = new CUdpSocket;
        break;
    case PROTOCOL_TCP_SERVER:
        m_pSocket = new CTcpServerSocket;
        break;
    case PROTOCOL_UDP_SERVER:
        m_pSocket = new CUdpSocket;
        break;
    default:
        break;
    }
    if(m_pSocket)
    {
        connect(m_pSocket, &CSocketBase::signalClose, this, &MainWindow::slotClose);
        connect(m_pSocket, &CSocketBase::signalError, this, &MainWindow::slotError);
        connect(m_pSocket, &CSocketBase::signalMessage, this, &MainWindow::slotMessage);
        connect(m_pSocket, &CSocketBase::signalHandShankReq, this, &MainWindow::slotHandShankReq);
        connect(m_pSocket, &CSocketBase::signalHandShankResp, this, &MainWindow::slotHandShankResp);
        connect(m_pSocket, &CSocketBase::signalFileSendFinish, this, &MainWindow::slotSendFileFinish);
        connect(m_pSocket, &CSocketBase::signalSendFileProgressChange, this, &MainWindow::slotFileSendProgressChange);
        OpenSocket(ui->cbProtocol->currentData().toInt());
    }
}

void MainWindow::slotClose()
{
    if(m_pSocket)
    {
        m_pSocket->Stop();
    }
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->btnSendFiles->setEnabled(false);
    ui->btnStopSend->setEnabled(false);
}

void MainWindow::slotError(const QString &errString)
{
    addLog(LOG_LEVEL::LOG_LEVEL_ERR, errString);
    slotClose();
}

void MainWindow::slotMessage(const QString &message)
{
    addLog(LOG_LEVEL::LOG_LEVEL_INFO, message);
}

void MainWindow::slotHandShankReq()
{
    ui->btnSendFiles->setEnabled(true);
    ui->btnStopSend->setEnabled(false);
}

void MainWindow::slotHandShankResp()
{
    ui->btnSendFiles->setEnabled(true);
    ui->btnStopSend->setEnabled(false);
}

void MainWindow::slotSendFileFinish()
{
    ui->btnSendFiles->setEnabled(true);
    ui->btnStopSend->setEnabled(false);
    ui->btnDisconnect->setEnabled(true);
}

void MainWindow::slotFileSendProgressChange(int progress)
{
    ui->progressBar->setValue(progress);
}

void MainWindow::on_btnDisconnect_clicked()
{
    slotClose();
}

void MainWindow::on_btnSendFiles_clicked()
{
    QStringList fileList;
    for(int i = 0; i < ui->fileListWidget->count(); ++i)
    {
        QListWidgetItem *pItem = ui->fileListWidget->item(i);
        fileList.append(pItem->text());
    }
    QMetaObject::invokeMethod(m_pSocket, "SendFile", Q_ARG(const QStringList&, fileList));
    ui->btnSendFiles->setEnabled(false);
    ui->btnStopSend->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
}

void MainWindow::on_btnStopSend_clicked()
{
    m_pSocket->StopSend();
}

void MainWindow::on_cbProtocol_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    switch (ui->cbProtocol->currentData().toInt())
    {
    case PROTOCOL_TCP_CLIENT:
    case PROTOCOL_UDP_CLIENT:
        ui->labelAddr->setVisible(true);
        ui->editIpAddr->setVisible(true);
        ui->btnConnect->setText("连接");
        break;
    case PROTOCOL_TCP_SERVER:
    case PROTOCOL_UDP_SERVER:
        ui->labelAddr->setVisible(false);
        ui->editIpAddr->setVisible(false);
        ui->btnConnect->setText("打开");
        break;
    default:
        break;
    }
}
