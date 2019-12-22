#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "socket/TcpClientSocket.h"

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
    ui->cbProtocol->addItem("TCP", PROTOCOL_TCP);
    ui->cbProtocol->addItem("UDP", PROTOCOL_UDP);

    ui->btnDisconnect->setEnabled(false);
    ui->btnSendFiles->setEnabled(false);
    ui->btnStopSend->setEnabled(false);

    ui->logTableView->setModel(&m_logModel);
    ui->logTableView->setColumnWidth(0, 140);
    ui->logTableView->setColumnWidth(1, 50);
    ui->logTableView->setColumnWidth(2, 600);

    ui->editIpAddr->setText("127.0.0.1");
    ui->editPort->setText("8080");
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
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    switch(ui->cbProtocol->currentData().toInt())
    {
    case PROTOCOL_TCP:
        m_pSocket = new CTcpClientSocket;
        break;
    case PROTOCOL_UDP:
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
        m_pSocket->Start(ui->editIpAddr->text(), ui->editPort->text().toUShort());
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
    //m_pSocket->SendFile(fileList);
    QMetaObject::invokeMethod(m_pSocket, "SendFile", Q_ARG(const QStringList&, fileList));
    ui->btnSendFiles->setEnabled(false);
    ui->btnStopSend->setEnabled(true);
}

void MainWindow::on_btnStopSend_clicked()
{
    m_pSocket->StopSend();
}
