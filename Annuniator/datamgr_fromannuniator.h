#ifndef DATAMGR_FROMANNUNIATOR_H
#define DATAMGR_FROMANNUNIATOR_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QMutex>
struct dataItem_Annuniator{
    QByteArray dataFromAnnuniator;
    QTime      datatime;
};

class DataMgr_FromAnnuniator : public QObject
{
    Q_OBJECT
public:
    explicit DataMgr_FromAnnuniator(QObject *parent = nullptr);

    int addAnnuniatorDataItem(dataItem_Annuniator* item);
    dataItem_Annuniator* getLatestDataItem();
signals:
    void totalDataChanged(int nums);
    void sendDataTrigger(int num);
    void ignoreDataTrigger(int num);
public slots:

protected:
    void checkAllDataValid();
    void clearAllData();
private:
    QList<dataItem_Annuniator*> annuniatorDataList;
    QMutex  mutex_dataitem;
};
extern DataMgr_FromAnnuniator dataMgrFromAnnuniator;

#endif // DATAMGR_FROMANNUNIATOR_H
