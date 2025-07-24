#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "manageannuniatorconnections.h"

ManageAnnuniatorConnections *manager_AnnuniatorConnections=nullptr;

ManageAnnuniatorConnections::ManageAnnuniatorConnections(QObject *parent) : QObject(parent)
{
    socketmap.clear();
    lastConnectionCount = 0;
    cleanupTimer = nullptr;  // 不在构造函数中创建
    initialized = false;

    logworker.addLogger("ManageAnnuniatorConnections created", LOGTYPE_PRINT_RECORD);
}

ManageAnnuniatorConnections::~ManageAnnuniatorConnections()
{
    // 停止清理定时器
    if (cleanupTimer) {
        cleanupTimer->stop();
        delete cleanupTimer;
        cleanupTimer = nullptr;
    }

    // 关闭所有连接
    closeAllConnections();

    logworker.addLogger("ManageAnnuniatorConnections destroyed", LOGTYPE_PRINT_RECORD);
}

int ManageAnnuniatorConnections::addNewConnection(ItemAnnuniatorConnection *newConn)
{
    if (newConn == nullptr) {
        logworker.addLogger("Attempt to add null connection", LOGTYPE_PRINT);
        return 0;
    }

    QMutexLocker locker(&mutex_Annuniator);

    QString ip = newConn->getAnnuniatorIP();

    logworker.addLogger(QString("Attempting to add connection for IP: %1").arg(ip), LOGTYPE_PRINT_RECORD);

    // 首先清理无效连接
    checkValid();

    // 检查是否已存在相同IP的连接
    if (checkExisted(ip)) {
        ItemAnnuniatorConnection* existing = socketmap[ip];

        // 检查现有连接是否仍然有效
        if (isConnectionValid(existing)) {
            logworker.addLogger(QString("Valid connection already exists for IP: %1, rejecting new connection").arg(ip), LOGTYPE_PRINT);
            return 0;  // 已存在有效连接，拒绝新连接
        } else {
            // 现有连接无效，移除它
            logworker.addLogger(QString("Removing invalid existing connection for IP: %1").arg(ip), LOGTYPE_PRINT_RECORD);
            removeConnection(ip);
        }
    }

    // 验证新连接的有效性
    if (!isConnectionValid(newConn)) {
        logworker.addLogger(QString("New connection for IP %1 is invalid").arg(ip), LOGTYPE_PRINT);
        return 0;
    }

    // 添加新连接
    socketmap.insert(ip, newConn);

    logworker.addLogger(QString("Successfully added connection for IP: %1 (Total connections: %2)").arg(ip).arg(socketmap.size()), LOGTYPE_PRINT_RECORD);

    // 连接信号以监听连接状态变化
    connect(newConn, &ItemAnnuniatorConnection::connectionFailed,
            this, [this, ip]() {
                logworker.addLogger(QString("Connection to %1 failed, will be removed").arg(ip), LOGTYPE_PRINT_RECORD);
            });

    emit connectionAdded(ip);
    emitConnectionCountChanged();

    return 1;
}

ItemAnnuniatorConnection *ManageAnnuniatorConnections::findSocketByAnnuIP(QString annuip)
{
    QMutexLocker locker(&mutex_Annuniator);

    if (socketmap.isEmpty()) {
        return nullptr;
    }

    if (socketmap.contains(annuip)) {
        ItemAnnuniatorConnection* conn = socketmap[annuip];

        // 验证连接是否仍然有效
        if (isConnectionValid(conn)) {
            return conn;
        } else {
            // 连接已无效，移除它
            logworker.addLogger(QString("Found invalid connection for IP %1, removing it").arg(annuip), LOGTYPE_PRINT_RECORD);
            removeConnection(annuip);
            return nullptr;
        }
    }

    return nullptr;
}

ItemAnnuniatorConnection *ManageAnnuniatorConnections::findSocketFirst()
{
    QMutexLocker locker(&mutex_Annuniator);

    if (socketmap.isEmpty()) {
        return nullptr;
    }

    // 遍历查找第一个有效连接
    for (auto it = socketmap.begin(); it != socketmap.end(); ++it) {
        if (isConnectionValid(it.value())) {
            return it.value();
        }
    }

    // 如果没有找到有效连接，清理一下
    logworker.addLogger("No valid connections found in findSocketFirst, performing cleanup", LOGTYPE_PRINT_RECORD);
    checkValid();

    return nullptr;
}

int ManageAnnuniatorConnections::closeAllConnections()
{
    QMutexLocker locker(&mutex_Annuniator);

    if (socketmap.size() == 0) {
        return 0;
    }

    logworker.addLogger(QString("Closing all %1 connections").arg(socketmap.size()), LOGTYPE_PRINT_RECORD);

    int closedCount = 0;
    auto it = socketmap.begin();
    while (it != socketmap.end()) {
        ItemAnnuniatorConnection* conn = it.value();
        QString ip = it.key();

        if (conn != nullptr) {
            if (conn->getTcpSocket() != nullptr) {
                conn->closeSocket();
            }
            delete conn;
            closedCount++;
        }

        it = socketmap.erase(it);
        emit connectionRemoved(ip);
    }

    logworker.addLogger(QString("Closed %1 connections").arg(closedCount), LOGTYPE_PRINT_RECORD);
    emitConnectionCountChanged();

    return closedCount;
}

void ManageAnnuniatorConnections::initialize()
{
    if (initialized) {
        return;  // 避免重复初始化
    }

    initialized = true;

    // 现在安全地创建定时器和连接信号
    cleanupTimer = new QTimer(this);
    cleanupTimer->setInterval(CLEANUP_INTERVAL);

    // 此时this指针完全有效，可以安全连接
    connect(cleanupTimer, &QTimer::timeout, this, &ManageAnnuniatorConnections::performPeriodicCleanup);
    cleanupTimer->start();

    logworker.addLogger("ManageAnnuniatorConnections initialized with timer", LOGTYPE_PRINT_RECORD);
}

int ManageAnnuniatorConnections::getConnectionCount() const
{
    QMutexLocker locker(&mutex_Annuniator);
    return socketmap.size();
}

QStringList ManageAnnuniatorConnections::getConnectedIPs() const
{
    QMutexLocker locker(&mutex_Annuniator);

    QStringList ips;
    for (auto it = socketmap.begin(); it != socketmap.end(); ++it) {
        if (isConnectionValid(it.value())) {
            ips.append(it.key());
        }
    }

    return ips;
}

bool ManageAnnuniatorConnections::hasValidConnections() const
{
    QMutexLocker locker(&mutex_Annuniator);

    for (auto it = socketmap.begin(); it != socketmap.end(); ++it) {
        if (isConnectionValid(it.value())) {
            return true;
        }
    }

    return false;
}

void ManageAnnuniatorConnections::performHealthCheck()
{
    QMutexLocker locker(&mutex_Annuniator);

    if (socketmap.isEmpty()) {
        return;
    }

    int invalidCount = 0;
    auto it = socketmap.begin();
    while (it != socketmap.end()) {
        ItemAnnuniatorConnection* conn = it.value();

        if (!isConnectionValid(conn)) {
            QString ip = it.key();
            logworker.addLogger(QString("Health check failed for connection %1").arg(ip), LOGTYPE_PRINT_RECORD);

            if (conn != nullptr) {
                delete conn;
            }

            it = socketmap.erase(it);
            emit connectionRemoved(ip);
            invalidCount++;
        } else {
            ++it;
        }
    }

    if (invalidCount > 0) {
        logworker.addLogger(QString("Health check removed %1 invalid connections").arg(invalidCount), LOGTYPE_PRINT_RECORD);
        emitConnectionCountChanged();
    }
}

int ManageAnnuniatorConnections::removeInvalidConnections()
{
    QMutexLocker locker(&mutex_Annuniator);

    int removedCount = 0;
    auto it = socketmap.begin();
    while (it != socketmap.end()) {
        if (!isConnectionValid(it.value())) {
            QString ip = it.key();

            if (it.value() != nullptr) {
                delete it.value();
            }

            it = socketmap.erase(it);
            emit connectionRemoved(ip);
            removedCount++;
        } else {
            ++it;
        }
    }

    if (removedCount > 0) {
        emitConnectionCountChanged();
    }

    return removedCount;
}

void ManageAnnuniatorConnections::refreshClients()
{
    performHealthCheck();
}

void ManageAnnuniatorConnections::ClientsHeartBeat()
{
    QMutexLocker locker(&mutex_Annuniator);

    if (socketmap.size() == 0) {
        return;
    }

    int heartbeatCount = 0;
    for (auto it = socketmap.begin(); it != socketmap.end(); ++it) {
        ItemAnnuniatorConnection* conn = it.value();
        if (isConnectionValid(conn)) {
            conn->req_basicInfo();
            heartbeatCount++;
        }
    }

    if (heartbeatCount > 0) {
        logworker.addLogger(QString("Sent heartbeat to %1 clients").arg(heartbeatCount), LOGTYPE_PRINT_RECORD);
    }
}

void ManageAnnuniatorConnections::ClientHeartBeat(QString annuip)
{
    ItemAnnuniatorConnection* itemconn = findSocketByAnnuIP(annuip);
    if (itemconn != nullptr) {
        itemconn->req_basicInfo();
        logworker.addLogger(QString("Sent heartbeat to client %1").arg(annuip), LOGTYPE_PRINT_RECORD);
    } else {
        logworker.addLogger(QString("Cannot send heartbeat to %1 - connection not found").arg(annuip), LOGTYPE_PRINT);
    }
}

void ManageAnnuniatorConnections::sendData2Annuniator(QString annuip, QByteArray data)
{
    logworker.addLogger("SLOT - annuniator send data", LOGTYPE_PRINT);

    ItemAnnuniatorConnection* itemconn = findSocketByAnnuIP(annuip);
    if (itemconn == nullptr) {
        // 如果指定IP的连接不存在，尝试使用第一个有效连接
        itemconn = findSocketFirst();
        if (itemconn != nullptr) {
            QString actualIP = itemconn->getAnnuniatorIP();
            logworker.addLogger(QString("Target IP %1 not found, using first available connection %2").arg(annuip).arg(actualIP), LOGTYPE_PRINT_RECORD);
        }
    }

    if (itemconn != nullptr) {
        int sentBytes = itemconn->SendData(data);
        if (sentBytes > 0) {
            QString annuinfo = QString("Data sent to %1, bytes=%2, total_connections=%3")
                              .arg(itemconn->getAnnuniatorIP())
                              .arg(sentBytes)
                              .arg(getConnectionCount());
            logworker.addLogger(annuinfo, LOGTYPE_PRINT);
        } else {
            logworker.addLogger(QString("Failed to send data to %1").arg(itemconn->getAnnuniatorIP()), LOGTYPE_PRINT);
        }
    } else {
        logworker.addLogger("Data Send ready, but No Valid Annuniator Found", LOGTYPE_PRINT);
    }
}

void ManageAnnuniatorConnections::performPeriodicCleanup()
{
    int removedCount = removeInvalidConnections();
    if (removedCount > 0) {
        logworker.addLogger(QString("Periodic cleanup removed %1 invalid connections").arg(removedCount), LOGTYPE_PRINT_RECORD);
    }
}

void ManageAnnuniatorConnections::checkValid()
{
    // 注意：此函数假设调用者已经获得了mutex_Annuniator锁

    if (socketmap.size() == 0) {
        return;
    }

    int removedCount = 0;
    auto it = socketmap.begin();
    while (it != socketmap.end()) {
        ItemAnnuniatorConnection* conn = it.value();
        QString ip = it.key();

        bool shouldRemove = false;

        if (conn == nullptr) {
            logworker.addLogger(QString("Found null connection for IP %1").arg(ip), LOGTYPE_PRINT_RECORD);
            shouldRemove = true;
        } else if (conn->getTcpSocket() == nullptr) {
            logworker.addLogger(QString("Found connection with null socket for IP %1").arg(ip), LOGTYPE_PRINT_RECORD);
            shouldRemove = true;
        } else if (conn->getTcpSocket()->state() == QAbstractSocket::UnconnectedState) {
            logworker.addLogger(QString("Found disconnected socket for IP %1").arg(ip), LOGTYPE_PRINT_RECORD);
            shouldRemove = true;
        } else if (!conn->isConnectionHealthy()) {
            logworker.addLogger(QString("Found unhealthy connection for IP %1").arg(ip), LOGTYPE_PRINT_RECORD);
            shouldRemove = true;
        }

        if (shouldRemove) {
            if (conn != nullptr) {
                delete conn;
            }
            it = socketmap.erase(it);
            emit connectionRemoved(ip);
            removedCount++;
        } else {
            ++it;
        }
    }

    if (removedCount > 0) {
        logworker.addLogger(QString("checkValid removed %1 invalid connections").arg(removedCount), LOGTYPE_PRINT_RECORD);
        emitConnectionCountChanged();
    }
}

bool ManageAnnuniatorConnections::checkExisted(QString ip)
{
    if (socketmap.isEmpty()) {
        return false;
    }

    return socketmap.contains(ip);
}

void ManageAnnuniatorConnections::removeConnection(const QString& ip)
{
    auto it = socketmap.find(ip);
    if (it != socketmap.end()) {
        if (it.value() != nullptr) {
            delete it.value();
        }
        socketmap.erase(it);
        emit connectionRemoved(ip);
        emitConnectionCountChanged();
    }
}

void ManageAnnuniatorConnections::removeConnection(QMap<QString, ItemAnnuniatorConnection*>::iterator& it)
{
    QString ip = it.key();
    if (it.value() != nullptr) {
        delete it.value();
    }
    it = socketmap.erase(it);
    emit connectionRemoved(ip);
    emitConnectionCountChanged();
}

bool ManageAnnuniatorConnections::isConnectionValid(ItemAnnuniatorConnection* conn) const
{
    if (conn == nullptr) {
        return false;
    }

    QTcpSocket* socket = conn->getTcpSocket();
    if (socket == nullptr) {
        return false;
    }

    if (socket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }

    if (!socket->isValid()) {
        return false;
    }

    // 检查连接健康状态
    if (!conn->isConnectionHealthy()) {
        return false;
    }

    return true;
}

void ManageAnnuniatorConnections::emitConnectionCountChanged()
{
    int currentCount = socketmap.size();
    if (currentCount != lastConnectionCount) {
        lastConnectionCount = currentCount;
        emit connectionCountChanged(currentCount);
        logworker.addLogger(QString("Connection count changed to: %1").arg(currentCount), LOGTYPE_PRINT_RECORD);
    }
}
