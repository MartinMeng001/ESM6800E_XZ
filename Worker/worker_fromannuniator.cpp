#include <sys/prctl.h>
#include "Configfile/userinfofile.h"
#include "Annuniator/datamgr_fromannuniator.h"
#include "Platform4G/platformtcp.h"
#include "Protocol/protocolfor4gserver.h"
#include "Platform4G/platformtcpv2.h"
#include "PlatformAsClient/manageplatformconnections.h"
#include "Log/loggerworker.h"
#include "worker_fromannuniator.h"

Controller_Annuniator annuniatorController;
Worker_fromAnnuniator::Worker_fromAnnuniator(QObject *parent) : QObject(parent)
{

}

void Worker_fromAnnuniator::work4Annuniator()
{
    //QString externalip = gUserInfoFile.get4GServerIP();
    prctl(PR_SET_NAME,"Worker_fromAnnuniator");
    connect(this, &Worker_fromAnnuniator::sendData2Platform, &gPlatformTcpv2, &PlatformTcp::processReadyWrite);
    //connect(this, &Worker_fromAnnuniator::reconnect2Platform, &gPlatformTcpv2, &PlatformTcp::onReconnect);
    connect(this, &Worker_fromAnnuniator::sendData2Platform, &manager_PlatformClientConnections, &ManagePlatformConnections::sendData2AllClients);
    //connect(this, &Worker_fromAnnuniator::checkNetworkStatus, &gPlatformTcp, &PlatformTcp::checkNetworkStatus);
    int countTimeout = 1000;
    while (true) {
        dataItem_Annuniator* item = dataMgrFromAnnuniator.getLatestDataItem();
        if(item!=nullptr){
            //gPlatformTcp.SendData(item->dataFromAnnuniator);
            emit sendData2Platform(item->dataFromAnnuniator);
            logworker.addLogger("work4Annuniator delete dataItem_Annuniator", LOGTYPE_PRINT_RECORD);
            delete item;
            //countTimeout=90000;
        }
        QThread::msleep(1);
        if(countTimeout>0)countTimeout--;
        if(countTimeout==0){
            countTimeout=1000;
            emit reconnect2Platform();
        }
//        QEventLoop loop;
//        QTimer::singleShot(10, &loop, SLOT(quit()));
//        loop.exec();
    }
    logworker.addLogger("Controller_Platform Connection Thread End", LOGTYPE_RECORD);
}

Controller_Annuniator::Controller_Annuniator(QObject *parent) : QObject(parent)
{

}

Controller_Annuniator::~Controller_Annuniator()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_Annuniator quit", LOGTYPE_PRINT);
}

void Controller_Annuniator::stopWorker()
{
    workerThread.exit();
    //workerThread.quit();
    //workerThread.wait();
}

void Controller_Annuniator::startSendWorker()
{
    Worker_fromAnnuniator* worker=new Worker_fromAnnuniator;

    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4Annuniator()));
    workerThread.start();
    emit startWorker();
}
