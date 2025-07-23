#include <iostream>
#include <QEventLoop>
#include <QTimer>
#include <sys/prctl.h>
#include "Platform808/datamgr_to808platform.h"
#include "Platform808/platform808tcpv2.h"
#include "Log/loggerworker.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "worker_to808platform.h"

Controller_808Platform controller808Platform;
Worker_To808Platform::Worker_To808Platform(QObject *parent) : QObject(parent)
{

}
void Worker_To808Platform::initWorker()
{
    prctl(PR_SET_NAME,"Worker_To808Platform");
    connect(this, &Worker_To808Platform::sendDataTo808Platform, &gPlatform808Tcpv2, &Platform808TcpV2::processReadyWrite);
    connect(this, &Worker_To808Platform::sendHeartbeatTo808Platform, &gAnnuniatorStatus, &AnnuniatorStatus::Action_808Heartbeat);
    int count10s = 1000;
    int heartbeat = 0;
    while (true) {
        if(heartbeat==1){
            dataItem_808Platform* item = dataMgr_To808Platform.getLatestDataItem();
            if(item!=nullptr){
                QByteArray datarcv = item->dataFromPlatform;
                emit sendDataTo808Platform(datarcv);
                logworker.addLogger("Worker_To808Platform delete dataItem_808Platform", LOGTYPE_PRINT_RECORD);
                delete item;
            }
        }
        count10s--;
        if(count10s<=0){
            count10s = 1000;
            QByteArray data = gProtocol808Platform.getCmd0002();
            if(data.size()!=0){
                emit sendDataTo808Platform(data);
            }
            // heartbeat = 1;
            //if(gPlatform808Tcp.getStatus()==1)
            //    emit sendHeartbeatTo808Platform();
        }
        heartbeat = gPlatform808Tcpv2.getHearbeat();
        QThread::msleep(10);
//        QEventLoop loop;
//        QTimer::singleShot(10, &loop, SLOT(quit()));
//        loop.exec();
    }
}

void Worker_To808Platform::work4Platform()
{
    initWorker();
}

Controller_808Platform::Controller_808Platform()
{

}

Controller_808Platform::~Controller_808Platform()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_808Platform quit", LOGTYPE_PRINT);
}

void Controller_808Platform::startPlatformWorker()
{
    Worker_To808Platform* worker=new Worker_To808Platform();
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4Platform()));

    workerThread.start();

    emit startWorker();
}

void Controller_808Platform::stopWorker()
{
    workerThread.exit();
    //workerThread.quit();
    //workerThread.wait();
}
