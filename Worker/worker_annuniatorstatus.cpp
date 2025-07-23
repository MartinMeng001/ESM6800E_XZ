#include <sys/prctl.h>
#include "Log/loggerworker.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "worker_annuniatorstatus.h"
Controller_AnnuniatorStatus annuniatorControllerStatus;
Worker_AnnuniatorStatus::Worker_AnnuniatorStatus(QObject *parent) : QObject(parent)
{

}

void Worker_AnnuniatorStatus::work4AnnuniatorStatus()
{
    prctl(PR_SET_NAME,"Worker_AnnuniatorStatus");
    connect(this, &Worker_AnnuniatorStatus::sigStartGetAnnuniatorTimeWorker, &gAnnuniatorStatus, &AnnuniatorStatus::Worker_GetAnnuniatorTime);
    connect(this, &Worker_AnnuniatorStatus::sigStartAnnuniatorStatusSlotsCommon, &gAnnuniatorStatus, &AnnuniatorStatus::Worker_AnnuniatorStatusSlots);
    emit sigStartGetAnnuniatorTimeWorker();
    emit sigStartAnnuniatorStatusSlotsCommon();
    while (true) {
        QThread::msleep(1000);
    }
}

Controller_AnnuniatorStatus::Controller_AnnuniatorStatus(QObject *parent)
{

}

Controller_AnnuniatorStatus::~Controller_AnnuniatorStatus()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_AnnuniatorStatus quit", LOGTYPE_PRINT);
}

void Controller_AnnuniatorStatus::stopWorker()
{
    workerThread.exit();
}

void Controller_AnnuniatorStatus::startStatusWorker()
{
    Worker_AnnuniatorStatus* worker=new Worker_AnnuniatorStatus;

    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4AnnuniatorStatus()));
    workerThread.start();
    emit startWorker();
}
