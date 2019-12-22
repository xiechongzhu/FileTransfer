#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QAbstractSocket>

int main(int argc, char *argv[])
{
    QDir dir;
    dir.mkdir(RECV_FOLDER);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
