#include "Log/loggerworker.h"
#include "status4gnetwork.h"


TimerStatus::TimerStatus()
{
    currentStatus = -1;
    timeCount.setHMS(0,0,0);
}

void TimerStatus::init(int status, int maxtime)
{
    initStatus = status;
    currentStatus = -1;
    overTime = maxtime;
    timeCount.start();
}

void TimerStatus::setStatus(int status)
{
    if(status!=initStatus){
        timeCount.restart();
    }
    currentStatus = status;
}

void TimerStatus::resetOverTime()
{
    timeCount.restart();
}

bool TimerStatus::beOverTime()
{
    if(currentStatus==-1) return false;
    if(currentStatus==initStatus){
        if(timeCount.elapsed()>overTime*1000){
            timeCount.restart();
            return true;
        }
    }
    if(timeCount.elapsed()>(overTime+10)*1000){
        timeCount.restart();
    }
    return false;
}

QString TimerStatus::logStatus()
{
    QString status = QString("init=%1,cur=%2,eclapsed=%3,MAX=%4").arg(initStatus).arg(currentStatus).arg(timeCount.elapsed()/1000).arg(overTime);
    return status;
}

Status4GNetwork::Status4GNetwork(QObject *parent)
{
    allow4GStatus = false;
    initStatusItem();
}

void Status4GNetwork::setPPPidStatus(int status)
{
    openSubStatus(status);
    statusPPPid.setStatus(status);
}

void Status4GNetwork::set808Status(int status)
{
    status808_4G.setStatus(status);
}

void Status4GNetwork::setPlatformStatus(int status)
{
    statusPlatform_4G.setStatus(status);
}

bool Status4GNetwork::Allow2RebootingFromPPPID()
{
    return statusPPPid.beOverTime();
}

bool Status4GNetwork::Allow2RebootingFrom8084G()
{
    if(allow4GStatus==false) return false;
    return status808_4G.beOverTime();
}

bool Status4GNetwork::Allow2RebootinngFromPlatform4G()
{
    if(allow4GStatus==false) return false;
    return statusPlatform_4G.beOverTime();
}

void Status4GNetwork::tick()
{
    logCurrentStatus();
}

void Status4GNetwork::initStatusItem()
{
    allow4GStatus = false;
    statusPPPid.init(0, 10*60);
    status808_4G.init(0, 10*60);
    statusPlatform_4G.init(0, 10*60);
}

void Status4GNetwork::openSubStatus(int status)
{
    if(status==1){
        if(allow4GStatus==false){
            allow4GStatus = true;
            status808_4G.resetOverTime();
            statusPlatform_4G.resetOverTime();
        }
    }else{
        allow4GStatus = false;
    }
}

void Status4GNetwork::logCurrentStatus()
{
    QString pppidlog = "PPPID Status "+statusPPPid.logStatus();
    QString _808log = "808 4G Status "+status808_4G.logStatus();
    QString platformlog = "platform 4G Status "+statusPlatform_4G.logStatus();
    logworker.addLogger(pppidlog, LOGTYPE_PRINT);
    logworker.addLogger(_808log, LOGTYPE_PRINT);
    logworker.addLogger(platformlog, LOGTYPE_PRINT);
}
