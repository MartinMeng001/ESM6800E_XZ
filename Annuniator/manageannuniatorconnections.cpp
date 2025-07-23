#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "manageannuniatorconnections.h"

ManageAnnuniatorConnections manager_AnnuniatorConnections;
ManageAnnuniatorConnections::ManageAnnuniatorConnections(QObject *parent) : QObject(parent)
{
    socketmap.clear();
}

int ManageAnnuniatorConnections::addNewConnection(ItemAnnuniatorConnection *newConn)
{
    //QMutexLocker locker(&mutex_Annuniator);
    checkValid();
    if(checkExisted(newConn->getAnnuniatorIP())) return 0;
    socketmap.insert(newConn->getAnnuniatorIP(), newConn);
    return 1;
}

ItemAnnuniatorConnection *ManageAnnuniatorConnections::findSocketByAnnuIP(QString annuip)
{
    QMutexLocker locker(&mutex_Annuniator);
    if(socketmap.isEmpty()) return nullptr;
    if(socketmap.contains(annuip)){
        return socketmap[annuip];
    }
    return nullptr;
}

ItemAnnuniatorConnection *ManageAnnuniatorConnections::findSocketFirst()
{
    QMutexLocker locker(&mutex_Annuniator);
    if(socketmap.isEmpty()) return nullptr;
    return socketmap.first();
}

int ManageAnnuniatorConnections::closeAllConnections()
{
    QMutexLocker locker(&mutex_Annuniator);
    if(socketmap.size()==0) return 0;
    QMap<QString, ItemAnnuniatorConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()==nullptr){
            it = socketmap.erase(it);
        }else{
            it.value()->closeSocket();
            delete it.value();
            it = socketmap.erase(it);
            //++it;
        }
    }
    return 1;
}

void ManageAnnuniatorConnections::refreshClients()
{
    checkValid();
}

void ManageAnnuniatorConnections::ClientsHeartBeat()
{
    if(socketmap.size()==0) return;
    QMap<QString, ItemAnnuniatorConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()!=nullptr){
            it.value()->req_basicInfo();
        }
        ++it;
    }
}

void ManageAnnuniatorConnections::ClientHeartBeat(QString annuip)
{
    ItemAnnuniatorConnection* itemconn = findSocketByAnnuIP(annuip);
    if(itemconn!=nullptr){
        itemconn->req_basicInfo();
    }
}

void ManageAnnuniatorConnections::sendData2Annuniator(QString annuip, QByteArray data)
{
    logworker.addLogger("SLOT - annuniator send data", LOGTYPE_PRINT);
    ItemAnnuniatorConnection* itemconn = findSocketByAnnuIP(annuip);
    if(itemconn==nullptr){
        //QString info = QString("Can't find Annuniator %1").arg(annuip);
        itemconn = findSocketFirst();
//        if(itemconn!=nullptr){
//            info += QString(" Try Annuniator %1").arg(itemconn->getAnnuniatorIP());
//        }
//        logworker.addLogger(info, LOGTYPE_PRINT_RECORD);
    }
    if(itemconn!=nullptr){
        itemconn->SendData(data);
        QString annuinfo = QString("annuniator ip=%1, size=%2").arg(itemconn->getAnnuniatorIP()).arg(socketmap.size());
        logworker.addLogger(annuinfo, LOGTYPE_PRINT);
    }else{
        logworker.addLogger("Data Send ready, but No Valid Annuniator Found", LOGTYPE_PRINT);
    }
}

void ManageAnnuniatorConnections::checkValid()
{
    QMutexLocker locker(&mutex_Annuniator);
    if(socketmap.size()==0) return;
    QMap<QString, ItemAnnuniatorConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()==nullptr){
            //std::cout << "remove one client" << std::endl;
            delete it.value();
            it = socketmap.erase(it);
        }else if(it.value()->getTcpSocket()->state()==QAbstractSocket::UnconnectedState){
            delete it.value();
            it = socketmap.erase(it);
        }else
            ++it;
    }
}

bool ManageAnnuniatorConnections::checkExisted(QString ip)
{
    if(socketmap.isEmpty()) return false;
    if(socketmap.contains(ip))return true;
    return false;
}
