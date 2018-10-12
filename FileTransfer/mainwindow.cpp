#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
}

void MainWindow::addLog(LOG_LEVEL logLevel, const QString &logMsg)
{
    m_logModel.add(logLevel, logMsg);
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
