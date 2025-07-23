#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "usage4gnetwork.h"

Usage4GNetwork::Usage4GNetwork(QObject *parent) : QObject(parent)
{
    resetData();
}

void Usage4GNetwork::set4GUsageData(int type, int datasize)
{
    if(checkNewDay()){
        logCurrentData2();
        resetData();
    }
    setDataOnce(type, datasize);
}

void Usage4GNetwork::print()
{
    logCurrentData();
}

bool Usage4GNetwork::checkNewDay()
{
    QDateTime now = QDateTime::currentDateTime();
    if(now.date().day()!=lastUpdateTime.date().day()){
        return true;
    }
    return false;
}

void Usage4GNetwork::resetData()
{
    lastUpdateTime = QDateTime::currentDateTime();
    QMutexLocker locker(&mutex);
    for(int i=0;i<SUPPORT_4GTYPE_TOTAL;i++){
        resetDataItem(i);
    }
}

void Usage4GNetwork::resetDataItem(int index)
{
    allData4GTypes[index].useddatasize = 0;
}

void Usage4GNetwork::setDataOnce(int type, int datasize)
{
    QMutexLocker locker(&mutex);
    switch (type) {
    case SUPPORT_4GTYPE_PLATFORM4G:
        setDataItem(0, datasize);
        break;
    case SUPPORT_4GTYPE_808PLATFORM:
        setDataItem(1, datasize);
        break;
    case SUPPORT_4GTYPE_PLATFORCLIENT:
        setDataItem(2, datasize);
        break;
    default:
        break;
    }
}

void Usage4GNetwork::setDataItem(int index, int datasize)
{
    allData4GTypes[index].useddatasize+=datasize;
}

void Usage4GNetwork::logCurrentData()
{
    QString log = QString("4G Usage:4G_Platform=%1\t4G_808Platform=%2\t4G_PlatformClient=%3\tTotal=%4").arg(getDataSizeByIndex(0))
            .arg(getDataSizeByIndex(1)).arg(getDataSizeByIndex(2)).arg(getTotalSize());
    logworker.addLogger(log, LOGTYPE_PRINT);
}

void Usage4GNetwork::logCurrentData2()
{
    QString log = QString("4G Usage:4G_Platform=%1\t4G_808Platform=%2\t4G_PlatformClient=%3\tTotal=%4").arg(getDataSizeByIndex(0))
            .arg(getDataSizeByIndex(1)).arg(getDataSizeByIndex(2)).arg(getTotalSize());
    logworker.addLogger(log, LOGTYPE_RECORD);
}

int Usage4GNetwork::getTotalSize()
{
    int sum = 0;
    for(int i=0;i<SUPPORT_4GTYPE_TOTAL;i++){
        sum+=getDataSizeByIndex(i);
    }
    return sum;
}

int Usage4GNetwork::getDataSizeByIndex(int index)
{
    return allData4GTypes[index].useddatasize;
}
