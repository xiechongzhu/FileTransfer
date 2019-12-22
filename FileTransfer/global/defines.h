#ifndef DEFINES_H
#define DEFINES_H

#include <QString>
#include <QDateTime>

static const int PROTOCOL_TCP = 1;
static const int PROTOCOL_UDP = 2;

static const int MAX_LOG_DISPLAY = 100;

static const char* RECV_FOLDER = "./RECV/";

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
