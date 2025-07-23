#include <sys/prctl.h>
#include <iostream>
#include <QString>
#include <QTime>
//#include "Driver/driver_4g.h"
#include "Driver/yiyuan_ec20_4gadapter.h"
#include "Log/loggerworker.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "worker_4g.h"

Controller_4G controller4G;
Worker_4G::Worker_4G(QObject *parent) : QObject(parent)
{
    flag_Rebooting = false;
    counts_MaxRetry = MAXRETRY_COUNTS;
}

void Worker_4G::reconnect()
{
    if(load4GDriver(TYPE4G_EC20)==0){
        logworker.addLogger("Unknown 4G module type, quit workFor4G", LOGTYPE_PRINT);
        return;
    }
    startDiagMgr();
    flag_Rebooting = false;
}

void Worker_4G::workFor4G()
{
    prctl(PR_SET_NAME,"workFor4G");
//    if(load4GDriver(TYPE4G_EC20)==0){
//        logworker.addLogger("Unknown 4G module type, quit workFor4G", LOGTYPE_PRINT);
//        return;
//    }
//    connect(this, &Worker_4G::sigReset4G, &gAnnuniatorStatus, &AnnuniatorStatus::Action_4GPowerReset);
    connect(this, &Worker_4G::sigPPPidStatus, &gAnnuniatorStatus, &AnnuniatorStatus::Status_PPPIDUpdating);
//    QTime count4Error;
//    count4Error.start();
    //startDiagMgr();

//    int ret = gYiYuanEC20_4GAdapter.getConnectStatus();
//    if(ret==EC20_CONN_STATUS_SUCCESS){
//        emit sigPPPidStatus(1);
//    }else{
//        emit sigPPPidStatus(0);
//    }

    while(true){
        QThread::msleep(5000);
        gAnnuniatorStatus.tick();
    }
}

int Worker_4G::load4GDriver(int type4G)
{
    char cmdstr[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    if(TYPE4G_ME909S == type4G)sprintf(cmdstr, MODPROBE_CMD, ME909_VENDORID, ME909_PRODUCTID);
    else if(TYPE4G_EC20 == type4G)sprintf(cmdstr, MODPROBE_CMD, EC20_VERDORID, EC20_PRODUCTID);
    else return 0;
    system(cmdstr);
    //conn_status=EC20_CONN_STATUS_LOADDRV;
    return 1;
}

int Worker_4G::startDiagMgr()
{
    //先调用一次pppconnt_stop，关闭连接
    //pppconnt_stop();
//    int ret = pppconnt_start();
//    QString a = QString("pppconnt_start with ret = %1").arg(ret);
//    logworker.addLogger(a, LOGTYPE_PRINT);
    return 0;
}
//void Worker_ForGPIO::open_4gPower()
//{
//    if(gpio_fd==0)return;
//    GPIO_OutSet(gpio_fd, POWER4GCONTROL_PIN);
//    emit sig_4gReseted();
//    logworker.addLogger("open_4gPower", LOGTYPE_PRINT);
//}
int Worker_4G::reset4GModule()
{
    return 0;
}

Controller_4G::Controller_4G(QObject *parent)
{

}

Controller_4G::~Controller_4G()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_4G quit", LOGTYPE_PRINT);
}

void Controller_4G::stopWorker()
{
    workerThread.exit();
}

void Controller_4G::start4GWorker()
{
    if(worker==nullptr){
        worker=new Worker_4G;
        logworker.addLogger("Controller_4G::- new Worker_4G;", LOGTYPE_PRINT_RECORD);
    }
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(workFor4G()));
    //connect(this, SIGNAL(restart4G()), worker, SLOT(reconnect()));
    workerThread.start();
    emit startWorker();
}

void Controller_4G::restart4G()
{
    if(worker==nullptr){
        worker=new Worker_4G;
        logworker.addLogger("Controller_4G::restart4G- new Worker_4G;", LOGTYPE_PRINT_RECORD);
    }
    if(worker!=nullptr){
        worker->reconnect();
        logworker.addLogger("reconnect 4G module", LOGTYPE_PRINT);
        //emit sigBatteryOff();
    }
}

