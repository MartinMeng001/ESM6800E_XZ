#include <iostream>
#include <QIODevice>
#include <QEventLoop>
#include <QTimer>
#include "Protocol/protocolfor4gserver.h"
#include "AnnuniatorStatus/usage4gnetwork.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "Log/loggerworker.h"
#include "datamgr_to808platform.h"
#include "platform808tcp.h"

Platform808Tcp gPlatform808Tcp;
Platform808Tcp::Platform808Tcp(QObject *parent) : QObject(parent)
{
    status=0;
    reConnectCount = 0;
    serverPort = 6808;
    serverIp = "1.194.232.48";
    beStopNew = false;
    beValidConnection = false;
    inReconnecting = false;
    lastsendTime = QDateTime::currentDateTime();
    emit clientStatusChanged(status);
}

int Platform808Tcp::start(QString ip, int port)
{
    serverIp = ip;
    serverPort = port;
    return reconnectServer();
}

int Platform808Tcp::close()
{
    return disconnectServer();
}

int Platform808Tcp::SendData(QByteArray &data4sending)
{
    int size=0;
    if(beValidConnection==false) return 0;
    if(client.isValid()==false) {
        reconnectServer();
        if(reConnectCount>0)reConnectCount--;
        std::cout << "808 connection is invalid" << std::endl;
        return 0;
    }
    if(client.isWritable()==false) {
        reconnectServer();
        if(reConnectCount>0)reConnectCount--;
        std::cout << "808 connection isnot writable" << std::endl;
        return 0;
    }
    size = client.write(data4sending);
    if(size==-1){
        if(checkClientStatus()==0){
            if(reConnectCount>0)reConnectCount--;
            std::cout << "Platform808Tcp::SendData error" << std::endl;
            client.close();
            reconnectServer();
            return 0;
            //Delay_MSec(10);
        }
    }else{
        if(reConnectCount!=0)reConnectCount=0;
        emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_808PLATFORM, size);
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("808Platform Send(%2):%1\r\n").arg(senddata).arg(size);
        logworker.addLogger(sendData, LOGTYPE_PRINT);
        lastsendTime = QDateTime::currentDateTime();
    }
    return size;
}

int Platform808Tcp::getStatus() const
{
    return status;
}

void Platform808Tcp::processNewConnection()
{
    inReconnecting = false;
    if(beValidConnection) return;
    if(checkClientStatus()==1) return;// allow to connect non-4G server
    std::cout << "************************808 reconnectServer" << std::endl;
    reconnectServer();
}

void Platform808Tcp::processConnected()
{
    std::cout << "************************808 platform connected" << std::endl;
    beValidConnection=true;
    checkClientStatus();
}

void Platform808Tcp::processDisconnected()
{
    //inReconnecting = false;
    qDebug() << "========================808 platform disconnected";
    beValidConnection=false;
    //Delay_MSec(60*1000);
}

void Platform808Tcp::processError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        qDebug() << "QAbstractSocket::RemoteHostClosedError";
        status = 0;
        break;
    case QAbstractSocket::HostNotFoundError:
        status = 0;
        logworker.addLogger("The 808 host was not found. Please check the host name and port settings.", LOGTYPE_PRINT);
        break;
    case QAbstractSocket::ConnectionRefusedError:
        status = 0;
        logworker.addLogger("The 808 connection was refused by the peer. Make sure the fortune server is running.", LOGTYPE_PRINT);
        break;
    default:
        status = 0;
        logworker.addLogger(client.errorString(), LOGTYPE_PRINT);
    }
    emit clientStatusChanged(status);
}

void Platform808Tcp::processReadyRead()
{
    if(client.isValid()==false) return;
    QByteArray data=client.readAll();   
    if(data.size()==0) return;
    emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_808PLATFORM, data.size());
    if(beStopNew) return; // 如果退出，就不再处理数据
    findItemPackets(data);
    //原始处理方式
    QString recdata = data.toHex().toUpper();
    QString recData = QString("808 Platform Receive:%1").arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
    if(status==0){
        status=1;
        emit clientStatusChanged(status);
    }
}

void Platform808Tcp::processReadyWrite(QByteArray data)
{
    SendData(data);
}


int Platform808Tcp::checkClientStatus()
{
    switch (client.state()) {
    case QAbstractSocket::ConnectedState:
        std::cout << "connected 808 platform" << std::endl;
        status =1;
        break;
    case QAbstractSocket::UnconnectedState:
        status = 0;
        break;
    default:
        break;
    }
    emit clientStatusChanged(status);
    return status;
}

int Platform808Tcp::disconnectServer()
{
    if(beValidConnection){
        std::cout << "disconnected from 808 platform" << std::endl;
        beValidConnection = false;
    //    if(checkClientStatus()==1){
    //        client.close();
    //    }
        client.close();
    }
    return checkClientStatus();
}

int Platform808Tcp::connectServer()
{
    if(checkClientStatus()==1) {
        qDebug() << "Platform808Tcp::checkClientStatus()==1 don't reconnect";
        return 1;
    }
    clientConfig();
    client.abort();
    qDebug() << "Platform808Tcp::connectServer() before client.connectToHost";
    if(reConnectCount==0)client.connectToHost(serverIp, serverPort, QIODevice::ReadWrite);
    reConnectCount+=5;
    qDebug() << "Platform808Tcp::connectServer() after client.connectToHost, before Delay_MSec 1min";
    checkClientStatus();
    //Delay_MSec(60*1000);    // if don't connect in 1 minute, reconnect it
    //qDebug() << "Platform808Tcp::connectServer() after Delay_MSec 1min";
    QString info = QString("Connecting to 808 Server, %1:%2").arg(serverIp).arg(serverPort);
    logworker.addLogger(info, LOGTYPE_PRINT);
    return 1;
}

int Platform808Tcp::reconnectServer()
{
    disconnectServer();
    return connectServer();
}

int Platform808Tcp::clientConfig()
{
    connect(&client, &QTcpSocket::connected, this, &Platform808Tcp::processConnected);
    connect(&client, &QTcpSocket::readyRead, this, &Platform808Tcp::processReadyRead);
    connect(&client, &QTcpSocket::disconnected, this, &Platform808Tcp::processDisconnected);
    connect(this, &Platform808Tcp::sig4GNetworkDataChanging, &gAnnuniatorStatus, &AnnuniatorStatus::Status_4GNetworkUsage);
    connect(this, &Platform808Tcp::clientStatusChanged, &gAnnuniatorStatus, &AnnuniatorStatus::Status_4G808);
    return 1;
}

bool Platform808Tcp::parse_basicInfo(QByteArray &data)
{
//    QByteArray retdata;
//    if(gProtocolFor4GServer.checkProtocolData(data, retdata)==1){
//        SendData(retdata);
//        return true;
//    }
    return false;
}

void Platform808Tcp::findItemPackets(QByteArray &data)
{
    if(beValidConnection == false) beValidConnection = true;
//    if(parse_basicInfo(data)){
//        if(beValidConnection == false) beValidConnection = true;
//    }else{
//        // add data to send buffer
//        dataItem_808Platform *dataitem=new dataItem_808Platform;
//        dataitem->dstIPAddress = "192.168.0.99";
//        dataitem->dataFromPlatform = data;
//        dataitem->datatime=QTime::currentTime();
//        dataMgr_To808Platform.addPlatformDataItem(dataitem);
//    }
}

void Platform808Tcp::Delay_MSec(int msec)
{
    if(inReconnecting) return;
    inReconnecting = true;
    //QEventLoop loop;
    QTimer::singleShot(msec, this, SLOT(processNewConnection()));
    //loop.exec();
}
