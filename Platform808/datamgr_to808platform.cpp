#include "Log/loggerworker.h"
#include "datamgr_to808platform.h"

DataMgr_To808Platform dataMgr_To808Platform;
DataMgr_To808Platform::DataMgr_To808Platform(QObject *parent) : QObject(parent)
{

}

int DataMgr_To808Platform::addPlatformDataItem(dataItem_808Platform *item)
{
    QMutexLocker locker(&mutex_dataitem);
    if(item==nullptr)return 0;
    if(platformDataList.size()>100){
        logworker.addLogger("DataMgr_To808Platform::addPlatformDataItem over 100", LOGTYPE_PRINT_RECORD);
        clearAllData();
    }
    platformDataList.prepend(item);
    emit totalDataChanged(platformDataList.size());
    return 1;
}

dataItem_808Platform *DataMgr_To808Platform::getLatestDataItem()
{
    QMutexLocker locker(&mutex_dataitem);
    if(platformDataList.size()==0)return nullptr;
    emit sendDataTrigger(1);
    return platformDataList.takeLast();
}

void DataMgr_To808Platform::checkAllDataValid()
{

}

void DataMgr_To808Platform::clearAllData()
{
    QList<dataItem_808Platform*>::iterator it;
    for(it=platformDataList.begin();it!=platformDataList.end();){
        delete *it;
        it = platformDataList.erase(it);
    }
    platformDataList.clear();
}
