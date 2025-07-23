#include "Log/loggerworker.h"
#include "Platform4G/platformtcp.h"
#include "Protocol/protocolannuniator4u.h"
#include "annuniatortime.h"

AnnuniatorTime::AnnuniatorTime(QObject *parent) : QObject(parent)
{
    timediff = 0;
}

int AnnuniatorTime::setCurrentAnnuniatorTime(QDateTime datetime)
{
    if(checkDeviceTimeValid()==false){
        timediff = getTimeDiff(datetime);
        QString infolog = QString("DeviceTime is Wrong:%1").arg(datetime.toString("yyyy-MM-dd HH:mm:ss"));
        logworker.addLogger(infolog, LOGTYPE_PRINT);
        return 0;
    }
    timediff = getTimeDiff(datetime);
    if(checkTimeDiff()){
        QByteArray timedata = gProtocolAnnuniator4U.setAnnuniatorTime();
        emit updateAnnuniatorTime(timedata);
        QString infolog = QString("AnnunitorTime:%1, try to correct").arg(datetime.toString("yyyy-MM-dd HH:mm:ss"));
        logworker.addLogger(infolog, LOGTYPE_PRINT);
        return 1;
    }
    QString infolog = QString("Valid AnnunitorTime:%1").arg(datetime.toString("yyyy-MM-dd HH:mm:ss"));
    logworker.addLogger(infolog, LOGTYPE_PRINT);
    return 0;
}

QDateTime AnnuniatorTime::getAnnuniatorTime()
{
    QDateTime now = QDateTime::currentDateTime();
    return now.addSecs((long)timediff);
}

bool AnnuniatorTime::checkDeviceTimeValid()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime validtime = QDateTime::fromString("2021-12-27 00:00:00", "yyyy-MM-dd HH:mm:ss");
    //validtime.date().setDate(2021, 12, 27);
    //validtime.time().setHMS(0,0,0);

    QString infolog = QString("Time Now:%1, Time Valid Base:%2").arg(now.toString("yyyy-MM-dd HH:mm:ss")).arg(validtime.toString("yyyy-MM-dd HH:mm:ss"));
    logworker.addLogger(infolog, LOGTYPE_PRINT);

    if(validtime.secsTo(now)<=0) return false;
    return true;
}

int AnnuniatorTime::getTimeDiff(QDateTime datetime)
{
    QDateTime now = QDateTime::currentDateTime();
    return (int)datetime.secsTo(now);
}

bool AnnuniatorTime::checkTimeDiff()
{
    if(timediff>120 || timediff<-120) return true;
    return false;
}

