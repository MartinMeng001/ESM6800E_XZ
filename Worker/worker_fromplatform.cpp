#include <iostream>
#include <QEventLoop>
#include <QTimer>
#include <sys/prctl.h>
#include "Annuniator/manageannuniatorconnections.h"
#include "Platform4G/datamgr_fromplatform.h"
#include "Log/loggerworker.h"
#include "worker_fromplatform.h"

Controller_Platform controllerPlatform;
Worker_FromPlatform::Worker_FromPlatform(QObject *parent) : QObject(parent)
{

}

void Worker_FromPlatform::initWorker()
{
    prctl(PR_SET_NAME,"Worker_FromPlatform");
    connect(this, &Worker_FromPlatform::sendData2Annuniator, manager_AnnuniatorConnections, &ManageAnnuniatorConnections::sendData2Annuniator);
    while (true) {
        dataItem_Platform* item = dataMgr_FromPlatform.getLatestDataItem();
        if(item!=nullptr){
            QByteArray datarcv = item->dataFromPlatform;
            logworker.addLogger("Worker_FromPlatform - delete dataItem_Platform;", LOGTYPE_PRINT_RECORD);
            emit sendData2Annuniator(item->dstIPAddress, datarcv);
            delete item;
        }
        QThread::msleep(1);
//        QEventLoop loop;
//        QTimer::singleShot(10, &loop, SLOT(quit()));
//        loop.exec();
    }
}

void Worker_FromPlatform::work4Platform()
{
    initWorker();
}

Controller_Platform::Controller_Platform()
{

}

Controller_Platform::~Controller_Platform()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_Platform quit", LOGTYPE_PRINT);
}

void Controller_Platform::startPlatformWorker()
{
    Worker_FromPlatform* worker=new Worker_FromPlatform();
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4Platform()));

    workerThread.start();

    emit startWorker();
}

void Controller_Platform::stopWorker()
{
    workerThread.exit();
    //workerThread.quit();
    //workerThread.wait();
}
