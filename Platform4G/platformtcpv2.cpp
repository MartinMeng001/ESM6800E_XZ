#include <QDebug>
#include "Protocol/protocolfor4gserver.h"
#include "Log/loggerworker.h"
#include "datamgr_fromplatform.h"
#include "platformtcpv2.h"

PlatformTcpV2 gPlatformTcpv2;
PlatformTcpV2::PlatformTcpV2(QObject *parent)
    : QObject{parent}
{
    reconnectTimer = new QTimer(this);
    reconnectTimer->setSingleShot(true);
}

int PlatformTcpV2::start(QString ip, int port, QString annuip)
{
    this->ip = ip;
    this->port = port;
    this->annuniatorip = annuip;
    socket = new QTcpSocket(this);

    // 连接成功信号
    connect(socket, &QTcpSocket::connected, this, &PlatformTcpV2::onConnected);
    // 断开连接信号
    connect(socket, &QTcpSocket::disconnected, this, &PlatformTcpV2::onDisconnected);
    // connect read
    connect(socket, &QTcpSocket::readyRead, this, &PlatformTcpV2::processReadyRead);
    // 连接错误信号
    //connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &PlatformTcpV2::onError);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    // 重新连接定时信号
    connect(reconnectTimer, &QTimer::timeout, this, &PlatformTcpV2::reconnect);

    connectToServer();
}

int PlatformTcpV2::SendData(QByteArray &data4sending)
{
    int size = socket->write(data4sending);
    socket->waitForBytesWritten();
    if(size==-1){
        return 0;
    }else{
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("PlatformV2 Send(%2):%1").arg(senddata).arg(size);
        logworker.addLogger(sendData, LOGTYPE_PRINT);
    }
    return size;
}

void PlatformTcpV2::connectToServer()
{
    qDebug() << "Connecting to Server...";
    socket->connectToHost(ip, port);
}

void PlatformTcpV2::onConnected()
{
    qDebug() << "Connected to server!";
    // 在连接成功后，重置定时器
    reconnectTimer->stop();
}

void PlatformTcpV2::onDisconnected()
{
    qDebug() << "Disconnected from server!";
    // 断线后启动重连定时器
    if(reconnectTimer->isActive()) return;
    reconnectTimer->start(3000);
}

void PlatformTcpV2::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qDebug() << "Socket error:" << socket->errorString();
    // 出错后启动重连定时器
    if(reconnectTimer->isActive()) return;
    reconnectTimer->start(3000);
}

void PlatformTcpV2::onReconnect()
{
    qDebug() << "Unknown Socket error:lost connection";
    // 出错后启动重连定时器
    if(reconnectTimer->isActive()) return;
    reconnectTimer->start(3000);
}

void PlatformTcpV2::reconnect()
{
    qDebug() << "Reconnecting...";
    // 关闭当前连接
    socket->abort();
    // 连接到服务器
    connectToServer();
}

void PlatformTcpV2::processReadyWrite(QByteArray data)
{
    SendData(data);
}

void PlatformTcpV2::processReadyRead()
{
    QByteArray data=socket->readAll();
    if(data.size()==0) return;
    findItemPackets(data);
    //原始处理方式
    QString recdata = data.toHex().toUpper();
    QString recData = QString("Platform Receive:%1").arg(recdata);
    //logworker.addLogger(recData, LOGTYPE_PRINT);
}

void PlatformTcpV2::processExtraData4Annuniator(QByteArray data)
{
    dataItem_Platform *dataitem=new dataItem_Platform;
    logworker.addLogger("PlatformTcp::processExtraData4Annuniator - new dataItem_Platform;", LOGTYPE_PRINT_RECORD);
    dataitem->dstIPAddress = "192.168.0.99";
    dataitem->dataFromPlatform = data;
    dataitem->datatime=QTime::currentTime();
    dataMgr_FromPlatform.addPlatformDataItem(dataitem);
}

bool PlatformTcpV2::parse_basicInfo(QByteArray &data)
{
    heartbeat.clear();
    if(gProtocolFor4GServer.checkProtocolData(data, heartbeat)==1){
        SendData(heartbeat);
        return true;
    }
    return false;
}

void PlatformTcpV2::findItemPackets(QByteArray &data)
{
    if(parse_basicInfo(data)){
        //if(beValidConnection == false) beValidConnection = true;
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
