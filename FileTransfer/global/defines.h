#ifndef DEFINES_H
#define DEFINES_H

#include <QString>
#include <QDateTime>

static const int PROTOCOL_TCP_CLIENT = 1;
static const int PROTOCOL_UDP_CLIENT = 2;
static const int PROTOCOL_TCP_SERVER = 3;
static const int PROTOCOL_UDP_SERVER = 4;

static const int MAX_LOG_DISPLAY = 100;

static const char* RECV_FOLDER = "./RECV/";

static const int UDP_SEND_BUFFER_SIZE = 5000;
static const int TCP_SEND_BUFFER_SIZE = 100000;
static const int RECV_BUFFER_SIZE = 1000000;

enum class LOG_LEVEL
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_ERR
};

typedef struct
{
    QDateTime logTime;
    LOG_LEVEL logLevel;
    QString logMsg;
}LogData;

#endif // DEFINES_H
