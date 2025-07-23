#include <iostream>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include "Log/loggerworker.h"
#include "itemplatformconnection.h"
#include "manageplatformconnections.h"
#include "tcpserver4platform.h"

TcpServer4Platform gtcpServer4Platfrom;
TcpServer4Platform::TcpServer4Platform(QObject *parent) : QObject(parent)
{
    status=0;
    listenPort = 5800;
    beStopNew = false;
    emit serverStatusChanged(status);
}

int TcpServer4Platform::start(int port)
{
    listenPort=port;
    return rebuildServer();
}

int TcpServer4Platform::close()
{
    return closeServer();
}

int TcpServer4Platform::stopNewConnection()
{
    beStopNew = true;
    return 1;
}

int TcpServer4Platform::getStatus() const
{
    return status;
}

void TcpServer4Platform::processNewConnection()
{
    logworker.addLogger("New PlatformClient Connected", LOGTYPE_PRINT_RECORD);
    while(server.hasPendingConnections()){
        QTcpSocket* psocket=server.nextPendingConnection();
        if(beStopNew) {
            psocket->close();
            return;
        }
        ItemPlatformConnection *item=new ItemPlatformConnection();

        item->initItem(psocket);
        manager_PlatformClientConnections.addNewConnection(item);
//        int delay=1000;
//        while(delay--){
//            Delay_MSec(10);
//            if(item->getBeValidConnection()){
//                break;
//            }
//        }
//        if(beStopNew) {
//            psocket->close();
//            delete item;
//            return;
//        }
//        if(item->getBeValidConnection()){
//            manager_PlatformClientConnections.addNewConnection(item);
//        }else{
//            psocket->close();
//        }
    }
}

int TcpServer4Platform::checkServerStatus()
{
    if(server.isListening()) {
        status=1;
        emit serverStatusChanged(status);
        return 1;
    }
    status=0;
    emit serverStatusChanged(status);
    return 0;
}

int TcpServer4Platform::closeServer()
{
    if(checkServerStatus()==1){
        server.pauseAccepting();
        server.close();
    }
    return checkServerStatus();
}

int TcpServer4Platform::startServer()
{
    if(checkServerStatus()==1) return 1;
    serverConfig();
    server.listen(QHostAddress::Any, listenPort);
    QString info = QString("PlatformClient Server Listen Port:%1").arg(listenPort);
    logworker.addLogger(info, LOGTYPE_PRINT);
    return checkServerStatus();
}

int TcpServer4Platform::rebuildServer()
{
    closeServer();
    return startServer();
}

int TcpServer4Platform::serverConfig()
{
    server.setMaxPendingConnections(10);    // default 30
    connect(&server, &QTcpServer::newConnection, this, &TcpServer4Platform::processNewConnection);
    return 1;
}

void TcpServer4Platform::Delay_MSec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
