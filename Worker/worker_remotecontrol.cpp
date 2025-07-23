#include <iostream>
#include <QThread>
#include <sys/prctl.h>
#include "Configfile/userinfofile.h"
#include "Log/loggerworker.h"
#include "RemoteControl/pulse315m.h"
#include "worker_remotecontrol.h"
Controller_RemoteControl controllerRemoteControl;
Worker_RemoteControl::Worker_RemoteControl(QObject *parent) : QObject(parent)
{

}

void Worker_RemoteControl::initWorker()
{
    prctl(PR_SET_NAME,"Worker_RemoteControl");
    QByteArray organization = gUserInfoFile.getOrganization().toLatin1();
    pulse315M.setAllowOrganizationId(organization[0], organization[1], organization[2]);
    pulse315M.setRecordOnly(organization[3]);
    //pulse315M.setRecordOnly(setRecordOnly
    std::cout << "start PulseAnalysis Worker" << std::endl;
    while (true) {
        pulse315M.checkDataOnce();
        QThread::msleep(10);
    }
}

void Worker_RemoteControl::work4RemoteGuard()
{
    initWorker();
}

Controller_RemoteControl::Controller_RemoteControl()
{

}

Controller_RemoteControl::~Controller_RemoteControl()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_RemoteControl quit", LOGTYPE_PRINT);
}

void Controller_RemoteControl::startRemoteControlWorker()
{
    Worker_RemoteControl* worker=new Worker_RemoteControl();
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(work4RemoteGuard()));

    workerThread.start();

    emit startWorker();
}

void Controller_RemoteControl::stopWorker()
{
    workerThread.exit();
}
