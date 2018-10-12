#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "global/defines.h"
#include "logListViewModel.h"

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
private slots:
    void on_btnAddFiles_clicked();

    void on_btnDelFiles_clicked();

    void on_btnClearFiles_clicked();

private:
    Ui::MainWindow *ui;
    CLogListViewModel m_logModel;
};

#endif // MAINWINDOW_H
