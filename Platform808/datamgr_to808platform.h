#ifndef DATAMGR_TO808PLATFORM_H
#define DATAMGR_TO808PLATFORM_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QList>
#include <QMutex>
struct dataItem_808Platform{
    QString     dstIPAddress;
    QByteArray  dataFromPlatform;
    QTime       datatime;
};

class DataMgr_To808Platform : public QObject
{
    Q_OBJECT
public:
    explicit DataMgr_To808Platform(QObject *parent = nullptr);

    int addPlatformDataItem(dataItem_808Platform* item);
    dataItem_808Platform *getLatestDataItem();
signals:
    void totalDataChanged(int nums);
    void sendDataTrigger(int num);
    void ignoreDataTrigger(int num);
public slots:

protected:
    void checkAllDataValid();
    void clearAllData();

private:
    QList<dataItem_808Platform*> platformDataList;
    QMutex  mutex_dataitem;
};
extern DataMgr_To808Platform dataMgr_To808Platform;
#endif // DATAMGR_TO808PLATFORM_H
