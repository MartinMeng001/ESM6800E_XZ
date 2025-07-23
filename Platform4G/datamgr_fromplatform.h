#ifndef DATAMGR_FROMPLATFORM_H
#define DATAMGR_FROMPLATFORM_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QList>
#include <QMutex>
struct dataItem_Platform{
    QString     dstIPAddress;
    QByteArray  dataFromPlatform;
    QTime       datatime;
};

class DataMgr_FromPlatform : public QObject
{
    Q_OBJECT
public:
    explicit DataMgr_FromPlatform(QObject *parent = nullptr);

    int addPlatformDataItem(dataItem_Platform* item);
    dataItem_Platform *getLatestDataItem();
signals:
    void totalDataChanged(int nums);
    void sendDataTrigger(int num);
    void ignoreDataTrigger(int num);
public slots:

protected:
    void checkAllDataValid();
    void clearAllData();

private:
    QList<dataItem_Platform*> platformDataList;
    QMutex  mutex_dataitem;
};
extern DataMgr_FromPlatform dataMgr_FromPlatform;

#endif // DATAMGR_FROMPLATFORM_H
