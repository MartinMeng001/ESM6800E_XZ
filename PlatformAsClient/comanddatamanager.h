#ifndef COMANDDATAMANAGER_H
#define COMANDDATAMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QMutex>
#include "commanddataitem.h"

class ComandDataManager : public QObject
{
    Q_OBJECT
public:
    explicit ComandDataManager(QObject *parent = nullptr);
    bool newDataFromAnnuniator(QByteArray &data);
    void newDataFromPlatform(QByteArray &data);
signals:

protected:
    bool checkDataExisted(int command);
    void addNewItem(int command);
    void updateItem(int command);
    bool checkItem(int command);
private:
    QMap<int, CommandDataItem*> commandMap;
    QMutex mutex;
};

#endif // COMANDDATAMANAGER_H
