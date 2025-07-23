#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "datamgr_fromplatform.h"

DataMgr_FromPlatform dataMgr_FromPlatform;
DataMgr_FromPlatform::DataMgr_FromPlatform(QObject *parent) : QObject(parent)
{
    platformDataList.clear();
}

int DataMgr_FromPlatform::addPlatformDataItem(dataItem_Platform *item)
{
    QMutexLocker locker(&mutex_dataitem);
    if(item==nullptr)return 0;
    if(platformDataList.size()>1000) {
        clearAllData();
        logworker.addLogger("DataMgr_FromPlatform::addPlatformDataItem platformDataList over 1000", LOGTYPE_PRINT_RECORD);
    }
    platformDataList.prepend(item);
    emit totalDataChanged(platformDataList.size());
    return 1;
}

dataItem_Platform *DataMgr_FromPlatform::getLatestDataItem()
{
    QMutexLocker locker(&mutex_dataitem);
    if(platformDataList.size()==0)return nullptr;
    if(platformDataList.size()>1000){
        clearAllData();
        //logworker.addLogger("DataMgr_FromPlatform::getLatestDataItem annuniatorDataList over 1000", LOGTYPE_PRINT_RECORD);
    }
    emit sendDataTrigger(1);
    return platformDataList.takeLast();
}

void DataMgr_FromPlatform::checkAllDataValid()
{

}

void DataMgr_FromPlatform::clearAllData()
{
    QList<dataItem_Platform*>::iterator it;
    for(it=platformDataList.begin();it!=platformDataList.end();){
        delete *it;
        it = platformDataList.erase(it);
    }
    platformDataList.clear();
}
