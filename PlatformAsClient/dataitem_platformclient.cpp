#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "dataitem_platformclient.h"

DataMgr_FromPlatformClient dataMgrPlatformClient;
DataMgr_FromPlatformClient::DataMgr_FromPlatformClient(QObject *parent) : QObject(parent)
{
    annuniatorDataList.clear();
}

int DataMgr_FromPlatformClient::addAnnuniatorDataItem(dataItem_PlatformClient *item)
{
    QMutexLocker locker(&mutex_dataitem);
    if(item==nullptr)return 0;
    if(annuniatorDataList.size()>1000){
        logworker.addLogger("DataMgr_FromPlatformClient::addAnnuniatorDataItem annuniatorDataList over 1000", LOGTYPE_PRINT_RECORD);
        return 1;
    }
    annuniatorDataList.prepend(item);
    emit totalDataChanged(annuniatorDataList.size());
    return 1;
}

dataItem_PlatformClient *DataMgr_FromPlatformClient::getLatestDataItem()
{
    QMutexLocker locker(&mutex_dataitem);
    if(annuniatorDataList.size()==0)return nullptr;
    emit sendDataTrigger(1);
    return annuniatorDataList.takeLast();
}

void DataMgr_FromPlatformClient::checkAllDataValid()
{

}
