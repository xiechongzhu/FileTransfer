#-------------------------------------------------
#
# Project created by QtCreator 2019-12-21T14:44:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = FileTransfer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        logListViewModel.cpp \
        main.cpp \
        mainwindow.cpp \
        protocol/DataBuilder.cpp \
        socket/SocketBase.cpp \
        socket/TcpClientSocket.cpp \
        socket/TcpServerSocket.cpp \
        socket/UdpSocket.cpp

HEADERS += \
        global/defines.h \
        global/protocol.h \
        logListViewModel.h \
        mainwindow.h \
        protocol/DataBuilder.h \
        socket/SocketBase.h \
        socket/TcpClientSocket.h \
        socket/TcpServerSocket.h \
        socket/UdpSocket.h

FORMS += \
        mainwindow.ui

LIBS += -lws2_32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
