#include <iostream>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "itemannuniatorconnection.h"
#include "manageannuniatorconnections.h"
#include "tcpserver4annuniator.h"

TcpServer4Annuniator *annuniatorTcpServer = nullptr;

TcpServer4Annuniator::TcpServer4Annuniator(QObject *parent) : QObject(parent)
{
    status = 0;
    listenPort = 5801;
    beStopNew = false;
    serverConfigured = false;
    initialized = false;

    // 不在构造函数中创建定时器，避免信号连接问题
    validationTimer = nullptr;

    emit serverStatusChanged(status);
    logworker.addLogger("TcpServer4Annuniator created", LOGTYPE_PRINT_RECORD);
}

TcpServer4Annuniator::~TcpServer4Annuniator()
{
    close();
    if (validationTimer) {
        validationTimer->stop();
        delete validationTimer;
        validationTimer = nullptr;
    }
    logworker.addLogger("TcpServer4Annuniator destroyed", LOGTYPE_PRINT_RECORD);
}

int TcpServer4Annuniator::start(int port)
{
    QMutexLocker locker(&serverMutex);

    if (port <= 0 || port > 65535) {
        logworker.addLogger(QString("Invalid port number: %1").arg(port), LOGTYPE_PRINT);
        return 0;
    }

    listenPort = port;
    beStopNew = false;  // 确保允许新连接

    int result = rebuildServer();
    if (result == 1) {
        validationTimer->start();  // 启动连接验证定时器
        logworker.addLogger(QString("TcpServer4Annuniator started on port %1").arg(port), LOGTYPE_PRINT);
    } else {
        logworker.addLogger(QString("Failed to start TcpServer4Annuniator on port %1").arg(port), LOGTYPE_PRINT);
    }

    return result;
}

int TcpServer4Annuniator::close()
{
    QMutexLocker locker(&serverMutex);

    beStopNew = true;
    validationTimer->stop();
    if(manager_AnnuniatorConnections==nullptr) return 0;
    // 先关闭所有现有连接
    manager_AnnuniatorConnections->closeAllConnections();

    int result = closeServer();
    logworker.addLogger("TcpServer4Annuniator closed", LOGTYPE_PRINT);
    return result;
}

int TcpServer4Annuniator::stopNewConnection()
{
    QMutexLocker locker(&serverMutex);
    beStopNew = true;
    logworker.addLogger("TcpServer4Annuniator stopped accepting new connections", LOGTYPE_PRINT_RECORD);
    return 1;
}

int TcpServer4Annuniator::resumeNewConnection()
{
    QMutexLocker locker(&serverMutex);
    beStopNew = false;
    logworker.addLogger("TcpServer4Annuniator resumed accepting new connections", LOGTYPE_PRINT_RECORD);
    return 1;
}

int TcpServer4Annuniator::getStatus() const
{
    return status;
}

void TcpServer4Annuniator::initialize()
{
    if (initialized) {
        return;  // 避免重复初始化
    }

    initialized = true;

    // 现在安全地创建定时器和连接信号
    validationTimer = new QTimer(this);
    validationTimer->setSingleShot(false);
    validationTimer->setInterval(5000);  // 每5秒检查一次

    // 此时this指针完全有效，可以安全连接
    connect(validationTimer, &QTimer::timeout, this, &TcpServer4Annuniator::onConnectionValidationTimeout);

    logworker.addLogger("TcpServer4Annuniator initialized with timer", LOGTYPE_PRINT_RECORD);
}

int TcpServer4Annuniator::getActiveConnectionCount() const
{
    return manager_AnnuniatorConnections->getConnectionCount();
}

void TcpServer4Annuniator::processNewConnection()
{
    QMutexLocker locker(&serverMutex);

    logworker.addLogger("Processing new Annuniator connection request", LOGTYPE_PRINT_RECORD);

    int processedCount = 0;
    while (server.hasPendingConnections() && processedCount < MAX_PENDING_CONNECTIONS) {
        QTcpSocket* psocket = server.nextPendingConnection();
        processedCount++;

        if (!psocket) {
            logworker.addLogger("Received null socket from server", LOGTYPE_PRINT);
            continue;
        }

        // 获取客户端IP
        QString clientIP = psocket->peerAddress().toString();
        if (clientIP.startsWith("::ffff:")) {
            clientIP = clientIP.mid(QString("::ffff:").size());
        }

        logworker.addLogger(QString("New connection attempt from IP: %1").arg(clientIP), LOGTYPE_PRINT_RECORD);

        // 检查是否停止接收新连接
        if (beStopNew) {
            logworker.addLogger(QString("Rejecting connection from %1 - server not accepting new connections").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            continue;
        }

        // 检查socket状态
        if (psocket->state() != QAbstractSocket::ConnectedState) {
            logworker.addLogger(QString("Socket from %1 is not in connected state: %2").arg(clientIP).arg(psocket->state()), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            continue;
        }

        // 创建连接项
        ItemAnnuniatorConnection *item = new ItemAnnuniatorConnection();
        if (!item) {
            logworker.addLogger(QString("Failed to create ItemAnnuniatorConnection for %1").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            psocket->deleteLater();
            continue;
        }

        // 初始化连接项
        item->initItem(psocket);

        // 连接验证过程（恢复被注释的代码，但改进实现）
        bool validationSuccess = false;
        int validationAttempts = 0;
        const int maxValidationAttempts = 100;  // 10秒超时 (100 * 100ms)

        logworker.addLogger(QString("Starting validation for connection from %1").arg(clientIP), LOGTYPE_PRINT_RECORD);

        while (validationAttempts < maxValidationAttempts && !beStopNew) {
            Delay_MSec(100);  // 100ms间隔
            validationAttempts++;

            // 检查socket是否仍然有效
            if (!psocket->isValid() || psocket->state() != QAbstractSocket::ConnectedState) {
                logworker.addLogger(QString("Socket from %1 became invalid during validation").arg(clientIP), LOGTYPE_PRINT);
                break;
            }

            // 检查连接是否通过验证
            if (item->getBeValidConnection()) {
                validationSuccess = true;
                logworker.addLogger(QString("Connection from %1 validated successfully after %2 attempts").arg(clientIP).arg(validationAttempts), LOGTYPE_PRINT_RECORD);
                break;
            }
        }

        // 再次检查是否停止接收新连接
        if (beStopNew) {
            logworker.addLogger(QString("Server stopped accepting connections during validation of %1").arg(clientIP), LOGTYPE_PRINT);
            psocket->close();
            delete item;
            continue;
        }

        // 处理验证结果
        if (validationSuccess && item->getBeValidConnection()) {
            // 尝试添加到连接管理器
            int addResult = manager_AnnuniatorConnections->addNewConnection(item);
            if (addResult == 1) {
                logworker.addLogger(QString("Successfully added connection from %1 to manager").arg(clientIP), LOGTYPE_PRINT_RECORD);
                emit newConnectionEstablished(clientIP);
            } else {
                logworker.addLogger(QString("Failed to add connection from %1 to manager (possibly duplicate)").arg(clientIP), LOGTYPE_PRINT);
                psocket->close();
                delete item;
            }
        } else {
            // 验证失败
            logworker.addLogger(QString("Connection validation failed for %1 after %2 attempts").arg(clientIP).arg(validationAttempts), LOGTYPE_PRINT);
            emit connectionValidationFailed(clientIP);
            psocket->close();
            delete item;
        }
    }

    if (processedCount > 0) {
        logworker.addLogger(QString("Processed %1 connection attempts").arg(processedCount), LOGTYPE_PRINT_RECORD);
    }
}

void TcpServer4Annuniator::onConnectionValidationTimeout()
{
    // 定期清理无效连接
    cleanupInvalidConnections();

    // 更新连接统计
    int activeConnections = getActiveConnectionCount();
    if (activeConnections > 0) {
        logworker.addLogger(QString("Active Annuniator connections: %1").arg(activeConnections), LOGTYPE_PRINT_RECORD);
    }
}

void TcpServer4Annuniator::cleanupInvalidConnections()
{
    manager_AnnuniatorConnections->refreshClients();
}

int TcpServer4Annuniator::checkServerStatus()
{
    if (server.isListening()) {
        if (status != 1) {
            status = 1;
            emit serverStatusChanged(status);
        }
        return 1;
    }

    if (status != 0) {
        status = 0;
        emit serverStatusChanged(status);
    }
    return 0;
}

int TcpServer4Annuniator::closeServer()
{
    if (checkServerStatus() == 1) {
        server.pauseAccepting();
        server.close();
        logworker.addLogger("Server socket closed", LOGTYPE_PRINT_RECORD);
    }
    serverConfigured = false;
    return checkServerStatus();
}

int TcpServer4Annuniator::startServer()
{
    if (checkServerStatus() == 1) {
        logworker.addLogger("Server is already running", LOGTYPE_PRINT_RECORD);
        return 1;
    }

    if (!serverConfig()) {
        logworker.addLogger("Server configuration failed", LOGTYPE_PRINT);
        return 0;
    }

    bool listenResult = server.listen(QHostAddress::Any, listenPort);
    if (!listenResult) {
        QString errorMsg = QString("Failed to listen on port %1: %2").arg(listenPort).arg(server.errorString());
        logworker.addLogger(errorMsg, LOGTYPE_PRINT);
        return 0;
    }

    QString info = QString("Annuniator Server listening on port: %1").arg(listenPort);
    logworker.addLogger(info, LOGTYPE_PRINT);

    return checkServerStatus();
}

int TcpServer4Annuniator::rebuildServer()
{
    logworker.addLogger("Rebuilding Annuniator server", LOGTYPE_PRINT_RECORD);
    closeServer();
    return startServer();
}

int TcpServer4Annuniator::serverConfig()
{
    if (serverConfigured) {
        return 1;  // 避免重复配置
    }

    try {
        // 设置服务器参数
        server.setMaxPendingConnections(MAX_PENDING_CONNECTIONS);

        // 断开之前可能存在的连接（避免重复连接）
        disconnect(&server, &QTcpServer::newConnection, this, &TcpServer4Annuniator::processNewConnection);

        // 连接新连接信号
        connect(&server, &QTcpServer::newConnection, this, &TcpServer4Annuniator::processNewConnection);

        serverConfigured = true;
        logworker.addLogger("Server configuration completed successfully", LOGTYPE_PRINT_RECORD);
        return 1;

    } catch (const std::exception& e) {
        logworker.addLogger(QString("Server configuration failed with exception: %1").arg(e.what()), LOGTYPE_PRINT);
        return 0;
    } catch (...) {
        logworker.addLogger("Server configuration failed with unknown exception", LOGTYPE_PRINT);
        return 0;
    }
}

void TcpServer4Annuniator::Delay_MSec(int msec)
{
    if (msec <= 0) return;

    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}
