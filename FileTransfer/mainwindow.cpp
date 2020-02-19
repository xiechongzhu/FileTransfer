#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "socket/TcpClientSocket.h"
#include "socket/UdpSocket.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_pSocket(nullptr)
{
    ui->setupUi(this);
    InitWidgets();
    addLog(LOG_LEVEL::LOG_LEVEL_INFO, "程序启动成功");
}

MainWindow::~MainWindow()
{
    delete ui;
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
        disconnect(m_pSocket, &CSocketBase::signalClose, this, &MainWindow::slotClose);
        disconnect(m_pSocket, &CSocketBase::signalError, this, &MainWindow::slotError);
        disconnect(m_pSocket, &CSocketBase::signalMessage, this, &MainWindow::slotMessage);
        disconnect(m_pSocket, &CSocketBase::signalFileSendFinish, this, &MainWindow::slotSendFileFinish);
        disconnect(m_pSocket, &CSocketBase::signalSendFileProgressChange, this, &MainWindow::slotFileSendProgressChange);
        m_pSocket->deleteLater();
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
        break;
    case PROTOCOL_UDP_SERVER:
        break;
    default:
        break;
    }
    if(m_pSocket)
    {
        connect(m_pSocket, &CSocketBase::signalClose, this, &MainWindow::slotClose);
        connect(m_pSocket, &CSocketBase::signalError, this, &MainWindow::slotError);
        connect(m_pSocket, &CSocketBase::signalMessage, this, &MainWindow::slotMessage);
        connect(m_pSocket, &CSocketBase::signalHandShank, this, &MainWindow::slotHandShank);
        connect(m_pSocket, &CSocketBase::signalFileSendFinish, this, &MainWindow::slotSendFileFinish);
        connect(m_pSocket, &CSocketBase::signalSendFileProgressChange, this, &MainWindow::slotFileSendProgressChange);
        m_pSocket->Start(ui->editIpAddr->text(), ui->editRemotePort->text().toUShort(), ui->editLocalPort->text().toUShort());
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

void MainWindow::slotHandShank()
{
    ui->btnSendFiles->setEnabled(true);
    ui->btnStopSend->setEnabled(false);
}

void MainWindow::slotSendFileFinish()
{
    ui->btnSendFiles->setEnabled(true);
    ui->btnStopSend->setEnabled(false);
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
        ui->editLocalPort->setEnabled(false);
        ui->btnHandShank->setVisible(false);
        break;
    case PROTOCOL_UDP_CLIENT:
        ui->editLocalPort->setEnabled(true);
        ui->btnHandShank->setVisible(false);
        break;
    case PROTOCOL_TCP_SERVER:
        ui->btnHandShank->setVisible(true);
        break;
    case PROTOCOL_UDP_SERVER:
        ui->btnHandShank->setVisible(true);
        break;
    default:
        break;
    }
}
