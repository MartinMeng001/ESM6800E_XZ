#include <iostream>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include "Log/loggerworker.h"
#include "itemplatformconnection.h"
#include "manageplatformconnections.h"
#include "tcpserver4platform.h"

TcpServer4Platform *gtcpServer4Platfrom=nullptr;

TcpServer4Platform::TcpServer4Platform(QObject *parent) : QObject(parent)
{
    status = 0;
    listenPort = 5800;
    beStopNew = false;  // 默认允许新连接
    initialized = false;
    // 创建状态检查定时器
    statusCheckTimer = nullptr;

    emit serverStatusChanged(status);

    logworker.addLogger("TcpServer4Platform initialized", LOGTYPE_PRINT);
}

int TcpServer4Platform::start(int port)
{
    beStopNew = false;  // 确保允许新连接
    listenPort = port;

    logworker.addLogger(QString("Starting server on port %1").arg(port), LOGTYPE_PRINT);
    return rebuildServer();
}

int TcpServer4Platform::close()
{
    logworker.addLogger("Closing server", LOGTYPE_PRINT);
    return closeServer();
}

int TcpServer4Platform::stopNewConnection()
{
    beStopNew = true;
    logworker.addLogger("New connections disabled", LOGTYPE_PRINT);
    return 1;
}

int TcpServer4Platform::allowNewConnection()
{
    beStopNew = false;
    logworker.addLogger("New connections enabled", LOGTYPE_PRINT);
    return 1;
}

int TcpServer4Platform::getStatus() const
{
    return status;
}

int TcpServer4Platform::getConnectionCount() const
{
    return manager_PlatformClientConnections.getActiveConnectionCount();
}

bool TcpServer4Platform::isStopNewEnabled() const
{
    return beStopNew;
}

void TcpServer4Platform::initialize()
{
    if (initialized) {
        return;  // 避免重复初始化
    }

    initialized = true;

    // 现在安全地创建定时器和连接信号
    statusCheckTimer = new QTimer(this);
    connect(statusCheckTimer, &QTimer::timeout, this, &TcpServer4Platform::logServerStatus);
    statusCheckTimer->start(30000);  // 每30秒检查一次状态

    logworker.addLogger("TcpServer4Platform initialized with timer", LOGTYPE_PRINT_RECORD);
}

void TcpServer4Platform::processNewConnection()
{
    logworker.addLogger("processNewConnection called", LOGTYPE_PRINT_RECORD);

    // 检查服务器状态
    if (!server.isListening()) {
        logworker.addLogger("ERROR: Server not listening when processing new connection", LOGTYPE_PRINT);
        return;
    }

    while(server.hasPendingConnections()) {
        QTcpSocket* psocket = server.nextPendingConnection();

        if (!psocket) {
            logworker.addLogger("ERROR: nextPendingConnection returned null", LOGTYPE_PRINT);
            continue;
        }

        QString clientIP = psocket->peerAddress().toString();
        QString clientInfo = QString("New connection from %1:%2").arg(clientIP).arg(psocket->peerPort());
        logworker.addLogger(clientInfo, LOGTYPE_PRINT_RECORD);

        // 检查是否停止新连接
        if(beStopNew) {
            logworker.addLogger(QString("Connection from %1 rejected: new connections disabled").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            return;
        }

        // 检查客户端IP有效性
        if(clientIP.isEmpty() || clientIP == "::") {
            logworker.addLogger("ERROR: Invalid client IP address", LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            continue;
        }

        // 检查是否已有连接
        if (manager_PlatformClientConnections.checkExisted(clientIP)) {
            logworker.addLogger(QString("Connection from %1 already exists, closing old connection").arg(clientIP), LOGTYPE_PRINT);
            manager_PlatformClientConnections.closeConnectionByIP(clientIP);
        }

        // 创建连接项
        ItemPlatformConnection *item = new ItemPlatformConnection();

        // 连接断开信号
        connect(psocket, &QTcpSocket::disconnected, this, &TcpServer4Platform::onClientDisconnected);

        // 初始化连接
        if (!item->initItem(psocket)) {
            logworker.addLogger(QString("ERROR: Failed to initialize connection from %1").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            delete item;
            continue;
        }

        // 验证连接有效性（添加超时机制）
        bool connectionValid = false;
        int attempts = 0;
        const int maxAttempts = 10;  // 最多等待1秒

        while (attempts < maxAttempts) {
            if (item->getBeValidConnection()) {
                connectionValid = true;
                break;
            }
            Delay_MSec(100);  // 等待100ms
            attempts++;
        }

        if (connectionValid) {
            // 添加到连接管理器
            if (manager_PlatformClientConnections.addNewConnection(item)) {
                logworker.addLogger(QString("Client %1 connected successfully").arg(clientIP), LOGTYPE_PRINT);
                emit clientConnected(clientIP);
            } else {
                logworker.addLogger(QString("ERROR: Failed to add connection %1 to manager").arg(clientIP), LOGTYPE_PRINT);
                psocket->close();
                psocket->deleteLater();
                delete item;
            }
        } else {
            logworker.addLogger(QString("ERROR: Connection %1 validation failed").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            delete item;
        }
    }

    // 记录连接统计
    logConnectionStats();
}

void TcpServer4Platform::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QString clientIP = socket->peerAddress().toString();
        logworker.addLogger(QString("Client %1 disconnected").arg(clientIP), LOGTYPE_PRINT);
        emit clientDisconnected(clientIP);
    }
}

int TcpServer4Platform::checkServerStatus()
{
    if(server.isListening()) {
        status = 1;
        emit serverStatusChanged(status);
        return 1;
    }
    status = 0;
    emit serverStatusChanged(status);
    return 0;
}

int TcpServer4Platform::closeServer()
{
    if(checkServerStatus() == 1) {
        // 先停止接受新连接
        server.pauseAccepting();

        // 关闭所有现有连接
        manager_PlatformClientConnections.closeAllConnections();

        // 关闭服务器
        server.close();

        logworker.addLogger("Server closed", LOGTYPE_PRINT);
    }
    return checkServerStatus();
}

int TcpServer4Platform::startServer()
{
    if(checkServerStatus() == 1) {
        logworker.addLogger("Server already running", LOGTYPE_PRINT);
        return 1;
    }

    serverConfig();

    // 尝试监听
    if (!server.listen(QHostAddress::Any, listenPort)) {
        QString error = QString("ERROR: Failed to start server on port %1: %2")
                       .arg(listenPort).arg(server.errorString());
        logworker.addLogger(error, LOGTYPE_PRINT);
        return 0;
    }

    QString info = QString("PlatformClient Server started on port %1").arg(listenPort);
    logworker.addLogger(info, LOGTYPE_PRINT);

    // 显示监听的网络接口
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress::LocalHost) {
            logworker.addLogger(QString("Server listening on %1:%2").arg(address.toString()).arg(listenPort), LOGTYPE_PRINT);
        }
    }

    return checkServerStatus();
}

int TcpServer4Platform::rebuildServer()
{
    logworker.addLogger("Rebuilding server", LOGTYPE_PRINT);
    closeServer();
    Delay_MSec(1000);  // 等待1秒确保端口释放
    return startServer();
}

int TcpServer4Platform::serverConfig()
{
    // 设置最大待处理连接数（对单设备来说10已足够）
    server.setMaxPendingConnections(10);

    // 连接新连接信号
    disconnect(&server, &QTcpServer::newConnection, this, &TcpServer4Platform::processNewConnection);
    connect(&server, &QTcpServer::newConnection, this, &TcpServer4Platform::processNewConnection);

    logworker.addLogger("Server configuration completed", LOGTYPE_PRINT);
    return 1;
}

void TcpServer4Platform::logServerStatus()
{
    QString statusMsg = QString("Server Status - Listening: %1, Port: %2, StopNew: %3")
                       .arg(server.isListening() ? "YES" : "NO")
                       .arg(listenPort)
                       .arg(beStopNew ? "YES" : "NO");

    logworker.addLogger(statusMsg, LOGTYPE_PRINT);

    if (server.isListening()) {
        logConnectionStats();
    }
}

void TcpServer4Platform::logConnectionStats()
{
    int activeConnections = getConnectionCount();
    int pendingConnections = server.hasPendingConnections() ? 1 : 0;

    QString stats = QString("Connection Stats - Active: %1, Pending: %2, MaxPending: %3")
                   .arg(activeConnections)
                   .arg(pendingConnections)
                   .arg(server.maxPendingConnections());

    logworker.addLogger(stats, LOGTYPE_PRINT);
}

void TcpServer4Platform::Delay_MSec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
