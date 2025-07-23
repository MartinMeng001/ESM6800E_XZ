#ifndef DATAITEM_PLATFORMCLIENT_H
#define DATAITEM_PLATFORMCLIENT_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QMutex>
struct dataItem_PlatformClient{
    QByteArray dataFromAnnuniator;
    QTime      datatime;
};

class DataMgr_FromPlatformClient : public QObject
{
    Q_OBJECT
public:
    explicit DataMgr_FromPlatformClient(QObject *parent = nullptr);

    int addAnnuniatorDataItem(dataItem_PlatformClient* item);
    dataItem_PlatformClient* getLatestDataItem();
signals:
    void totalDataChanged(int nums);
    void sendDataTrigger(int num);
    void ignoreDataTrigger(int num);
public slots:

protected:
    void checkAllDataValid();
private:
    QList<dataItem_PlatformClient*> annuniatorDataList;
    QMutex  mutex_dataitem;

};
extern DataMgr_FromPlatformClient dataMgrPlatformClient;
#endif // DATAITEM_PLATFORMCLIENT_H
