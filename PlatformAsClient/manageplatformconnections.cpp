#include "Log/loggerworker.h"
#include "manageplatformconnections.h"

ManagePlatformConnections manager_PlatformClientConnections;
ManagePlatformConnections::ManagePlatformConnections(QObject *parent) : QObject(parent)
{
    socketmap.clear();
}

int ManagePlatformConnections::addNewConnection(ItemPlatformConnection *newConn)
{
    checkValid();
    if(checkExisted(newConn->getAnnuniatorIP())) return 0;
    socketmap.insert(newConn->getAnnuniatorIP(), newConn);
    return 1;
}

ItemPlatformConnection *ManagePlatformConnections::findSocketByAnnuIP(QString annuip)
{
    if(socketmap.isEmpty()) return nullptr;
    if(socketmap.contains(annuip)){
        return socketmap[annuip];
    }
    return nullptr;
}

ItemPlatformConnection *ManagePlatformConnections::findSocketFirst()
{
    if(socketmap.isEmpty()) return nullptr;
    return socketmap.first();
}

int ManagePlatformConnections::closeAllConnections()
{
    if(socketmap.size()==0) return 0;
    QMap<QString, ItemPlatformConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()==nullptr){
            it = socketmap.erase(it);
        }else{
            it.value()->closeSocket();
            it = socketmap.erase(it);
            //++it;
        }
    }
    return 1;
}

void ManagePlatformConnections::refreshClients()
{
    checkValid();
}

void ManagePlatformConnections::ClientsHeartBeat()
{
    if(socketmap.size()==0) return;
    QMap<QString, ItemPlatformConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()!=nullptr){
            it.value()->req_basicInfo();
        }
        ++it;
    }
}

void ManagePlatformConnections::ClientHeartBeat(QString annuip)
{
    ItemPlatformConnection* itemconn = findSocketByAnnuIP(annuip);
    if(itemconn!=nullptr){
        itemconn->req_basicInfo();
    }
}

void ManagePlatformConnections::sendData2Annuniator(QString annuip, QByteArray data)
{    
    ItemPlatformConnection* itemconn = findSocketByAnnuIP(annuip);
    if(itemconn==nullptr)itemconn = findSocketFirst();
    if(itemconn!=nullptr){
        itemconn->SendData(data);
    }else{
        logworker.addLogger("Data Send ready, but No Valid Annuniator Found", LOGTYPE_PRINT);
    }
}

void ManagePlatformConnections::sendData2AllClients(QByteArray data)
{
    if(socketmap.size()==0) return;
    QMap<QString, ItemPlatformConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()==nullptr){
        }else{
            it.value()->SendData(data);
            //logworker.addLogger("Data Send ready, but No Valid Annuniator Found", LOGTYPE_PRINT);
        }
        ++it;
    }
}

void ManagePlatformConnections::checkValid()
{
    if(socketmap.size()==0) return;
    QMap<QString, ItemPlatformConnection*>::iterator it;
    for(it=socketmap.begin();it!=socketmap.end();){
        if(it.value()->getTcpSocket()==nullptr){
            it = socketmap.erase(it);
        }else
            ++it;
    }
}

bool ManagePlatformConnections::checkExisted(QString ip)
{
    if(socketmap.isEmpty()) return false;
    if(socketmap.contains(ip))return true;
    return false;
}
