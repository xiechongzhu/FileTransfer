#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "global/defines.h"
#include "logListViewModel.h"
#include "socket/SocketBase.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void InitWidgets();
    void addLog(LOG_LEVEL logLevel, const QString& logMsg);
    void OpenSocket(int socketType);
private slots:
    void on_btnAddFiles_clicked();

    void on_btnDelFiles_clicked();

    void on_btnClearFiles_clicked();

    void on_btnConnect_clicked();

    void on_btnDisconnect_clicked();

    void on_btnSendFiles_clicked();

    void on_btnStopSend_clicked();

    void on_cbProtocol_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    CLogListViewModel m_logModel;
    CSocketBase *m_pSocket;
protected slots:
    void slotClose();
    void slotError(const QString& errString);
    void slotMessage(const QString& message);
    void slotHandShankReq();
    void slotHandShankResp();
    void slotSendFileFinish();
    void slotFileSendProgressChange(int progress);
};

#endif // MAINWINDOW_H
