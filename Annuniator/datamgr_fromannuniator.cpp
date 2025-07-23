#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "datamgr_fromannuniator.h"

DataMgr_FromAnnuniator dataMgrFromAnnuniator;
DataMgr_FromAnnuniator::DataMgr_FromAnnuniator(QObject *parent) : QObject(parent)
{
    annuniatorDataList.clear();
}

int DataMgr_FromAnnuniator::addAnnuniatorDataItem(dataItem_Annuniator *item)
{
    QMutexLocker locker(&mutex_dataitem);
    if(item==nullptr)return 0;
    if(annuniatorDataList.size()>1000) {
        logworker.addLogger("DataMgr_FromAnnuniator::addAnnuniatorDataItem annuniatorDataList over 1000", LOGTYPE_PRINT_RECORD);
        clearAllData();
    }
    annuniatorDataList.prepend(item);
    emit totalDataChanged(annuniatorDataList.size());
    return 1;
}

dataItem_Annuniator *DataMgr_FromAnnuniator::getLatestDataItem()
{
    QMutexLocker locker(&mutex_dataitem);
    if(annuniatorDataList.size()==0)return nullptr;
    emit sendDataTrigger(1);
    return annuniatorDataList.takeLast();
}

void DataMgr_FromAnnuniator::checkAllDataValid()
{

}

void DataMgr_FromAnnuniator::clearAllData()
{
    QList<dataItem_Annuniator*>::iterator it;
    for(it=annuniatorDataList.begin();it!=annuniatorDataList.end();){
        delete *it;
        it = annuniatorDataList.erase(it);
    }
    annuniatorDataList.clear();
}
