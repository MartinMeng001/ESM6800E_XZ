#include <QDebug>
#include "platform808tcpv2.h"
#include "Log/loggerworker.h"

Platform808TcpV2 gPlatform808Tcpv2;
Platform808TcpV2::Platform808TcpV2(QObject *parent)
    : QObject{parent}
{
    reconnectTimer = new QTimer(this);
    reconnectTimer->setSingleShot(true);
    heartbeat = 0;
}

int Platform808TcpV2::start(QString ip, int port)
{
    this->serverip = ip;
    this->serverport = port;

    socket = new QTcpSocket(this);

    // 连接成功信号
    connect(socket, &QTcpSocket::connected, this, &Platform808TcpV2::onConnected);
    // 断开连接信号
    connect(socket, &QTcpSocket::disconnected, this, &Platform808TcpV2::onDisconnected);
    // read
    connect(socket, &QTcpSocket::readyRead, this, &Platform808TcpV2::processReadyRead);
    // 连接错误信号
    //connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &Platform808TcpV2::onError);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    // 重新连接定时信号
    connect(reconnectTimer, &QTimer::timeout, this, &Platform808TcpV2::reconnect);

    connectToServer();
}

int Platform808TcpV2::SendData(QByteArray &data4sending)
{
    int size = socket->write(data4sending);
    socket->waitForBytesWritten();
    if(size==-1){
        heartbeat = 0;
        return 0;
    }else{
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("808Platform Send(%2):%1\r\n").arg(senddata).arg(size);
        logworker.addLogger(sendData, LOGTYPE_PRINT);
        heartbeat = 1;
    }
    return size;
}

void Platform808TcpV2::connectToServer()
{
    qDebug() << "Connecting to 808 Server...";
    socket->connectToHost(serverip, serverport);
}

void Platform808TcpV2::onConnected()
{
    qDebug() << "Connected to 808 server!";
    // 在连接成功后，重置定时器
    reconnectTimer->stop();
}

void Platform808TcpV2::onDisconnected()
{
    qDebug() << "Disconnected from 808 server!";
    // 断线后启动重连定时器
    if(reconnectTimer->isActive()) return;
    reconnectTimer->start(3000);
}

void Platform808TcpV2::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qDebug() << "808 Socket error:" << socket->errorString();
    // 出错后启动重连定时器
    if(reconnectTimer->isActive()) return;
    reconnectTimer->start(3000);
}

void Platform808TcpV2::reconnect()
{
    qDebug() << "808 Reconnecting...";
    // 关闭当前连接
    socket->abort();
    // 连接到服务器
    connectToServer();
}

void Platform808TcpV2::processReadyRead()
{
    QByteArray data=socket->readAll();
    if(data.size()==0) return;
    findItemPackets(data);
    //原始处理方式
    QString recdata = data.toHex().toUpper();
    QString recData = QString("808 Platform Receive:%1").arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
}

void Platform808TcpV2::processReadyWrite(QByteArray data)
{
    SendData(data);
}

void Platform808TcpV2::findItemPackets(QByteArray &data)
{

}
