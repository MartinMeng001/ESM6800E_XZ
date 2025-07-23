#include <iostream>
#include <QFile>
#include "loggerworker.h"

LoggerWorker logworker;
LoggerWorker::LoggerWorker(QObject *parent) : QThread(parent)
{
    stopThread = false;
    allowCommonLog = false;
    allPendingLoggers.clear();
}

void LoggerWorker::addLogger(QString info, int type)
{
    if(stopThread) return;
    if(allowCommonLog==false){
        if(type == LOGTYPE_PRINT){
            return;
        }
    }
    QMutexLocker mutexLocker(&mutexObj);
    logInfoItem* item = new logInfoItem();
    item->info=info;
    item->infotime=QDateTime::currentDateTime();
    item->type=type;
    allPendingLoggers.append(item);    // 数据加结尾
}

void LoggerWorker::stopWorker()
{
    stopThread = true;
}

void LoggerWorker::setDisableLogCommon(int flag)
{
    if(flag==1)allowCommonLog = false;
    else allowCommonLog = true;
}


void LoggerWorker::run()
{
    std::cout << "LoggerWorker start" << std::endl;
    clearLogFile();
    while(1){
        if(stopThread){
            clearAllBuffer();
            break;
        }
        if(allPendingLoggers.size()>10)std::cout << "LoggerWorker size=" << allPendingLoggers.size() << std::endl;
        if(allPendingLoggers.size()>0){
            mutexObj.lock();
            logInfoItem* item = takeItem(allPendingLoggers.size());//allPendingLoggers.takeFirst();
            if(item!=nullptr){
                if(item->type == LOGTYPE_PRINT_RECORD || item->type == LOGTYPE_PRINT) {
                    printOneLogger(item);
                }
                if(item->type == LOGTYPE_RECORD || item->type == LOGTYPE_PRINT_RECORD){
                    recordOneLogger(item);
                    //msleep(500);
                    //printOneLogger(item);
                    std::cout << "LoggerWorker recorded" << allPendingLoggers.size() << std::endl;
                }
                delete item;
            }
            mutexObj.unlock();
        }
        msleep(500);
    }
    std::cout << "LoggerWorker quit" << std::endl;
}

void LoggerWorker::clearAllBuffer()
{
    QMutexLocker mutexLocker(&mutexObj);
    for(int i=0;i<allPendingLoggers.size();i++){
        logInfoItem* item = allPendingLoggers.takeFirst();
        delete item;
    }
}

void LoggerWorker::recordOneLogger(logInfoItem *item)
{
    //std::cout << "recordOneLogger enter1" << std::endl;
    QFile file("/mnt/mmc/Log/log.txt");
    //std::cout << "recordOneLogger enter" << std::endl;
    if(file.size()>1000*1000){
        file.remove();
    }
    if(file.open(QIODevice::Text | QIODevice::Append | QIODevice::WriteOnly)){
        QString datatime = item->infotime.toString("yyyy-MM-dd hh:mm:ss");
        QString data = QString("%1\t%2\r\n").arg(datatime,item->info);
        //std::cout << "recordOneLogger write" << std::endl;
        file.write(data.toLatin1());
        file.close();
        //std::cout << "recordOneLogger close" << std::endl;
    }
    //std::cout << "recordOneLogger quit" << std::endl;
}

void LoggerWorker::printOneLogger(logInfoItem *item)
{
    QString datatime = item->infotime.toString("yyyy-MM-dd hh:mm:ss");
    QString data = QString("%1\t%2\r\n").arg(datatime,item->info);
    std::cout << data.toUtf8().toStdString();
}

void LoggerWorker::clearLogFile()
{
    //std::cout << "recordOneLogger enter1" << std::endl;
    QFile file("/mnt/mmc/Log/log.txt");
    //std::cout << "recordOneLogger enter" << std::endl;
    if(file.open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly)){
        QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString data = QString("%1\tSystem booting..\r\n").arg(datatime);
        std::cout << "Logger File rebuild" << std::endl;
        file.write(data.toLatin1());
        file.close();
    }
}

logInfoItem *LoggerWorker::takeItem(int size)
{
    logInfoItem* item = nullptr;//allPendingLoggers.takeFirst();
    //if(size>1000){
    if(size>100){
        foreach (auto item2, allPendingLoggers) {
            if(item2->type!=LOGTYPE_PRINT) {
                allPendingLoggers.removeOne(item2);
                item = item2;
                break;
            }
            allPendingLoggers.removeOne(item2);
        }
    }else{
        if(size>0)
            item = allPendingLoggers.takeFirst();
    }
    return item;
}
