#include <sys/prctl.h>
#include "Annuniator/manageannuniatorconnections.h"
#include "PlatformAsClient/dataitem_platformclient.h"
#include "Log/loggerworker.h"
#include "worker_fromplatformclient.h"

Controller_PlatformClient platformClientController;
Worker_fromPlatformClient::Worker_fromPlatformClient(QObject *parent) : QObject(parent)
{

}

void Worker_fromPlatformClient::work4PlatformClient()
{
    prctl(PR_SET_NAME,"Worker_fromPlatformClient");
    connect(this, &Worker_fromPlatformClient::sendData2Annuniator, manager_AnnuniatorConnections, &ManageAnnuniatorConnections::sendData2Annuniator);
    while (true) {
        dataItem_PlatformClient* item = dataMgrPlatformClient.getLatestDataItem();
        if(item!=nullptr){
            emit sendData2Annuniator("192.168.0.99", item->dataFromAnnuniator);
            logworker.addLogger("signal - platformclient to signal", LOGTYPE_PRINT);
            delete item;
        }
        QThread::msleep(10);
    }
}

Controller_PlatformClient::Controller_PlatformClient(QObject *parent)
{

}

Controller_PlatformClient::~Controller_PlatformClient()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_PlatformClient quit", LOGTYPE_PRINT);
}

void Controller_PlatformClient::stopWorker()
{
    workerThread.exit();
}

void Controller_PlatformClient::startSendWorker()
{
    Worker_fromPlatformClient* worker=new Worker_fromPlatformClient;

    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4PlatformClient()));
    workerThread.start();
    emit startWorker();
}
