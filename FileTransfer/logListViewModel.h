#ifndef LOGLISTVIEWMODEL_H
#define LOGLISTVIEWMODEL_H

#include <QAbstractListModel>
#include "global/defines.h"

class CLogListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    CLogListViewModel();
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    void add(LOG_LEVEL logLevel, const QString& logMsg);
private:
    QList<LogData> m_logData;
};

#endif // LOGLISTVIEWMODEL_H
