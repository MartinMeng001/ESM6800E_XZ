#include <iostream>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include "Log/loggerworker.h"
#include "itemannuniatorconnection.h"
#include "manageannuniatorconnections.h"
#include "tcpserver4annuniator.h"

TcpServer4Annuniator annuniatorTcpServer;
TcpServer4Annuniator::TcpServer4Annuniator(QObject *parent) : QObject(parent)
{
    status=0;
    listenPort = 5801;
    beStopNew = false;
    emit serverStatusChanged(status);
}

int TcpServer4Annuniator::start(int port)
{
    listenPort=port;
    return rebuildServer();
}

int TcpServer4Annuniator::close()
{
    return closeServer();
}

int TcpServer4Annuniator::stopNewConnection()
{
    beStopNew = true;
    return 1;
}

int TcpServer4Annuniator::getStatus() const
{
    return status;
}

void TcpServer4Annuniator::processNewConnection()
{
    logworker.addLogger("New Annuniator Connected", LOGTYPE_PRINT_RECORD);
    while(server.hasPendingConnections()){
        QTcpSocket* psocket=server.nextPendingConnection();
        if(beStopNew) {
            psocket->close();
            return;
        }
        ItemAnnuniatorConnection *item=new ItemAnnuniatorConnection();

        item->initItem(psocket);

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
        if(item->getBeValidConnection()){
            manager_AnnuniatorConnections.addNewConnection(item);
        }else{
            psocket->close();
            delete item;
        }
    }
}

int TcpServer4Annuniator::checkServerStatus()
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

int TcpServer4Annuniator::closeServer()
{
    if(checkServerStatus()==1){
        server.pauseAccepting();
        server.close();
    }
    return checkServerStatus();
}

int TcpServer4Annuniator::startServer()
{
    if(checkServerStatus()==1) return 1;
    serverConfig();
    server.listen(QHostAddress::Any, listenPort);
    QString info = QString("Annuniator Server Listen Port:%1").arg(listenPort);
    logworker.addLogger(info, LOGTYPE_PRINT);
    return checkServerStatus();
}

int TcpServer4Annuniator::rebuildServer()
{
    closeServer();
    return startServer();
}

int TcpServer4Annuniator::serverConfig()
{
    server.setMaxPendingConnections(10);    // default 30
    connect(&server, &QTcpServer::newConnection, this, &TcpServer4Annuniator::processNewConnection);
    return 1;
}

void TcpServer4Annuniator::Delay_MSec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
