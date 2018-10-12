#include "logListViewModel.h"
#include "global/defines.h"
#include <QDateTime>
#include <QColor>

CLogListViewModel::CLogListViewModel()
{

}

int CLogListViewModel::rowCount(const QModelIndex &parent) const
{
    return m_logData.size();
}

int CLogListViewModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant CLogListViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
        case 0:
            return "时间";
        case 1:
            return "等级";
        case 2:
            return "日志信息";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant CLogListViewModel::data(const QModelIndex &index, int role) const
{
    const LogData logData = m_logData.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        switch(index.column())
        {
        case 0:
            return logData.logTime.toString("yyyy-MM-dd HH:mm:ss");
        case 1:
            switch(logData.logLevel)
            {
            case LOG_LEVEL::LOG_LEVEL_INFO:
                return "信息";
            case LOG_LEVEL::LOG_LEVEL_ERR:
                return "错误";
            default:
                return QVariant();
            }
        case 2:
            return logData.logMsg;
        default:
            return QVariant();
        }
    case Qt::BackgroundRole:
        switch (logData.logLevel)
        {
        case LOG_LEVEL::LOG_LEVEL_INFO:
            return QColor(Qt::white);
        case LOG_LEVEL::LOG_LEVEL_ERR:
            return QColor(Qt::red);
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

void CLogListViewModel::add(LOG_LEVEL logLevel, const QString &logMsg)
{
    if(m_logData.size() >= MAX_LOG_DISPLAY)
    {
        m_logData.pop_front();
    }
    LogData logData;
    logData.logTime = QDateTime::currentDateTime();
    logData.logLevel = logLevel;
    logData.logMsg = logMsg;
    m_logData.append(logData);
    emit layoutChanged();
}
