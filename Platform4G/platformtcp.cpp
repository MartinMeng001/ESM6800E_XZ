#include <QIODevice>
#include <QEventLoop>
#include <iostream>
#include <QTimer>
#include "Protocol/protocolfor4gserver.h"
#include "AnnuniatorStatus/usage4gnetwork.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "Log/loggerworker.h"
#include "datamgr_fromplatform.h"
#include "platformtcp.h"

PlatformTcp gPlatformTcp;
PlatformTcp::PlatformTcp(QObject *parent) : QObject(parent)
{
    status=0;
    reConnectCount = 0;
    serverPort = 8126;
    serverIp = "122.5.105.22";
    beStopNew = false;
    beValidConnection = false;
    inReconnecting = false;
    lastsendTime = QDateTime::currentDateTime();
    heartbeat.clear();
    emit clientStatusChanged(status);
}

int PlatformTcp::start(QString ip, int port)
{
    serverIp = ip;
    serverPort = port;
    return connectServer();
    //return reconnectServer();
}

int PlatformTcp::close()
{
    return disconnectServer();
}

int PlatformTcp::SendData2(QByteArray &data4sending)
{
    int size=0;
    if(client.isValid()==false) return 0;
    if(client.isWritable()==false) return 0;
//    if(checkClientStatus()==0){
//        //Delay_MSec(10);
//        reconnectServer();
//        return 0;
//    }
    size = client.write(data4sending);
    if(size==-1){
//        if(checkClientStatus()==0){
//            //Delay_MSec(10);
//            reconnectServer();
//        }
    }else{
        //client.flush();
        emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_PLATFORM4G, size);
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("Platform Send(%2):%1").arg(senddata).arg(size);
        logworker.addLogger(sendData, LOGTYPE_PRINT);
    }
    return size;
}

int PlatformTcp::SendData(QByteArray &data4sending)
{
    int size=0;
    if(beValidConnection==false) {
        if(reConnectCount>0)reConnectCount--;
        reconnectServer();
        return 0;
    }
    if(client.isValid()==false) {
        if(reConnectCount>0)reConnectCount--;
        reconnectServer();
        std::cout << "platform connection is invalid" << std::endl;
        return 0;
    }
    if(client.isWritable()==false) {
        if(reConnectCount>0)reConnectCount--;
        reconnectServer();
        std::cout << "platform connection isnot writable" << std::endl;
        return 0;
    }
    size = client.write(data4sending);

    if(size==-1){
        //if(checkClientStatus()==0){
            if(reConnectCount>0)reConnectCount--;
            std::cout << "Platform808Tcp::SendData error" << std::endl;
            client.close();
            reconnectServer();
            return 0;
            //Delay_MSec(10);
        //}
    }else{
        if(reConnectCount!=0)reConnectCount=0;
        emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_PLATFORM4G, size);
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("Platform Send(%2):%1").arg(senddata).arg(size);
        logworker.addLogger(sendData, LOGTYPE_PRINT);
        lastsendTime = QDateTime::currentDateTime();
    }
    return size;
}

void PlatformTcp::checkNetworkStatus()
{
    // TODO disable the logic
//    if(checkClientStatus()==0){
//        reconnectServer();
//    }
//    QString info = QString("checkNetworkStatus for Platform ok");
//    std::cout << info.toStdString() << std::endl;
    std::cout << "platform: heartbeat checking" << std::endl;
    if(heartbeat.size()>5){
        std::cout << "platform: heartbeat checking - data ok" << std::endl;
        if(lastsendTime.secsTo(QDateTime::currentDateTime())>50){
            std::cout << "platform: heartbeat checking - try to send" << std::endl;
            SendData(heartbeat);
        }
    }
}

int PlatformTcp::getStatus() const
{
    return status;
}

void PlatformTcp::processNewConnection()
{
    inReconnecting = false;
    if(beValidConnection) return;
    //if(checkClientStatus()==1) return;// allow to connect non-4G server
    QString info = QString("reconnectServer by Platform processNewConnection");
    logworker.addLogger(info, LOGTYPE_RECORD);
    reconnectServer();
}

void PlatformTcp::processDisconnected()
{
    //inReconnecting = false;
    disconnectServer();
    QString info = QString("Platform processDisconnected - disconnectServer");
    logworker.addLogger(info, LOGTYPE_RECORD);
    //Delay_MSec(60*1000);
}

void PlatformTcp::processError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        status = 0;
        break;
    case QAbstractSocket::HostNotFoundError:
        status = 0;
        logworker.addLogger("The host was not found. Please check the host name and port settings.", LOGTYPE_PRINT);
        break;
    case QAbstractSocket::ConnectionRefusedError:
        status = 0;
        logworker.addLogger("The connection was refused by the peer. Make sure the fortune server is running.", LOGTYPE_PRINT);
        break;
    default:
        status = 0;
        logworker.addLogger(client.errorString(), LOGTYPE_PRINT);
    }
    emit clientStatusChanged(status);
}

void PlatformTcp::processReadyRead()
{
    if(client.isValid()==false) return;
    QByteArray data=client.readAll();   
    if(data.size()==0) return;
    emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_PLATFORM4G, data.size());
    if(beStopNew) return; // 如果退出，就不再处理数据
    findItemPackets(data);
    //原始处理方式
    QString recdata = data.toHex().toUpper();
    QString recData = QString("Platform Receive:%1").arg(recdata);
    //logworker.addLogger(recData, LOGTYPE_PRINT);
    if(status==0){
        status=1;
        emit clientStatusChanged(status);
    }
}

void PlatformTcp::processReadyWrite(QByteArray data)
{
    SendData(data);
}

void PlatformTcp::processExtraData4Annuniator(QByteArray data)
{
    dataItem_Platform *dataitem=new dataItem_Platform;
    logworker.addLogger("PlatformTcp::processExtraData4Annuniator - new dataItem_Platform;", LOGTYPE_PRINT_RECORD);
    dataitem->dstIPAddress = "192.168.0.99";
    dataitem->dataFromPlatform = data;
    dataitem->datatime=QTime::currentTime();
    dataMgr_FromPlatform.addPlatformDataItem(dataitem);
}

int PlatformTcp::checkClientStatus()
{
    switch (client.state()) {
    case QAbstractSocket::ConnectedState:
        status =1;
        break;
    case QAbstractSocket::UnconnectedState:
        status = 0;
        break;
    case QAbstractSocket::ConnectingState:
        break;
    default:
        std::cout << "PlatformTcp: unkownStatus=" << client.state() << std::endl;
        status =0;
        break;
    }
    emit clientStatusChanged(status);
    return status;
}

int PlatformTcp::disconnectServer()
{
    beValidConnection = false;
    if(checkClientStatus()==1){
        client.close();
    }
    return checkClientStatus();
}

int PlatformTcp::connectServer()
{
    if(checkClientStatus()==1) {
        std::cout << "PlatformTcp: connection normal, don't reconnect" << std::endl;
        return 1;
    }
    clientConfig();
    //client.abort();
    if(reConnectCount==0){
        client.connectToHost(serverIp, serverPort, QIODevice::ReadWrite);
        QString info = QString("Connecting to 4G Server, %1:%2").arg(serverIp).arg(serverPort);
        logworker.addLogger(info, LOGTYPE_RECORD);
    }
    reConnectCount=5;
    //client.connectToHost(serverIp, serverPort);
    checkClientStatus();
    //Delay_MSec(60*1000);    // if don't connect in 1 minute, reconnect it

    return 1;
}

int PlatformTcp::reconnectServer()
{
    disconnectServer();
    return connectServer();
}

int PlatformTcp::clientConfig()
{
    connect(&client, &QTcpSocket::readyRead, this, &PlatformTcp::processReadyRead);
    connect(&client, &QTcpSocket::disconnected, this, &PlatformTcp::processDisconnected);
    connect(this, &PlatformTcp::sig4GNetworkDataChanging, &gAnnuniatorStatus, &AnnuniatorStatus::Status_4GNetworkUsage);
    connect(this, &PlatformTcp::clientStatusChanged, &gAnnuniatorStatus, &AnnuniatorStatus::Status_4GPlatform);
    return 1;
}

bool PlatformTcp::parse_basicInfo(QByteArray &data)
{
    //QByteArray retdata;
    heartbeat.clear();
    if(gProtocolFor4GServer.checkProtocolData(data, heartbeat)==1){
        SendData(heartbeat);
        return true;
    }
    return false;
}

void PlatformTcp::findItemPackets(QByteArray &data)
{
    if(parse_basicInfo(data)){
        if(beValidConnection == false) beValidConnection = true;
    }else{
        // add data to send buffer
        dataItem_Platform *dataitem=new dataItem_Platform;
        logworker.addLogger("PlatformTcp::findItemPackets - new dataItem_Platform", LOGTYPE_PRINT_RECORD);
        dataitem->dstIPAddress = "192.168.0.99";
        dataitem->dataFromPlatform = data;
        dataitem->datatime=QTime::currentTime();
        dataMgr_FromPlatform.addPlatformDataItem(dataitem);
    }
}

void PlatformTcp::Delay_MSec(int msec)
{
    if(inReconnecting) return;
    inReconnecting = true;
    //QEventLoop loop;
    QTimer::singleShot(msec, this, SLOT(processNewConnection()));
    //loop.exec();
}
