#include "platformtcp.h"
#include <iostream>
#include <QDebug>
#include <QAbstractSocket>
#include "Protocol/protocolfor4gserver.h"
#include "Log/loggerworker.h"
#include "datamgr_fromplatform.h"
#include "AnnuniatorStatus/annuniatorstatus.h"

// 全局实例定义
PlatformTcp gPlatformTcpv2;

PlatformTcp::PlatformTcp(QObject *parent)
    : QObject(parent)
    , serverIp("127.0.0.1")
    , serverPort(8125)
    , beValidConnection(false)
    , inReconnecting(false)
    , status(0)
    , reConnectCount(0)
    , currentReconnectCount(0)
    , maxReconnectAttempts(DEFAULT_MAX_RECONNECT_ATTEMPTS)
    , heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL)
    , connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
    , enableDeviceReboot(true)  // 新增：默认启用设备重启
    , deviceRebootTimeout(DEFAULT_DEVICE_REBOOT_TIMEOUT)  // 新增：15分钟超时
{
    // 初始化定时器
    reconnectTimer = new QTimer(this);
    reconnectTimer->setSingleShot(true);
    connect(reconnectTimer, &QTimer::timeout, this, &PlatformTcp::attemptReconnect);

    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &PlatformTcp::sendHeartbeat);

    healthCheckTimer = new QTimer(this);
    connect(healthCheckTimer, &QTimer::timeout, this, &PlatformTcp::checkConnectionHealth);

    // 新增：初始化设备重启定时器
    deviceRebootTimer = new QTimer(this);
    deviceRebootTimer->setSingleShot(true);
    connect(deviceRebootTimer, &QTimer::timeout, this, &PlatformTcp::triggerDeviceReboot);
    // 设置socket连接
    setupSocketConnections();

    // 初始化时间
    lastsendTime = QDateTime::currentDateTime();
    lastDataTime = QDateTime::currentDateTime();

    // 新增：初始化连接时间
    lastValidConnectionTime = QDateTime::currentDateTime();

    std::cout << "PlatformTcp: Constructor completed" << std::endl;
}

PlatformTcp::~PlatformTcp()
{
    stop();
}

int PlatformTcp::start(const QString &ip, int port)
{
    serverIp = ip;
    serverPort = port;

    std::cout << "PlatformTcp: Starting with server " << serverIp.toStdString()
              << ":" << serverPort << std::endl;

    resetConnectionState();
    return connectServer();
}

int PlatformTcp::stop()
{
    std::cout << "PlatformTcp: Stopping..." << std::endl;

    // 停止所有定时器
    reconnectTimer->stop();
    heartbeatTimer->stop();
    healthCheckTimer->stop();
    deviceRebootTimer->stop();  // 新增：停止设备重启定时器

    // 标记为主动断开，防止自动重连
    inReconnecting = false;
    currentReconnectCount = maxReconnectAttempts;

    // 断开连接
    return disconnectServer();
}

void PlatformTcp::setupSocketConnections()
{
    // 连接socket信号，确保只连接一次
    connect(&client, &QTcpSocket::connected, this, &PlatformTcp::onConnected);
    connect(&client, &QTcpSocket::disconnected, this, &PlatformTcp::onDisconnected);
    connect(&client, &QTcpSocket::readyRead, this, &PlatformTcp::processReadyRead);
    // 兼容不同Qt版本的错误信号连接
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    // Qt 5.15+ 使用 errorOccurred
    connect(&client, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &PlatformTcp::onSocketError);
#else
    // Qt 5.15之前使用 error 信号
    connect(&client, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &PlatformTcp::onSocketError);
#endif
}

int PlatformTcp::connectServer()
{
    if (checkClientStatus() == 1) {
        std::cout << "PlatformTcp: Already connected, don't reconnect" << std::endl;
        return 1;
    }

    std::cout << "PlatformTcp: Connecting to " << serverIp.toStdString()
              << ":" << serverPort << std::endl;

    // 确保之前的连接完全关闭
    client.abort();

    // 开始连接
    client.connectToHost(serverIp, serverPort, QIODevice::ReadWrite);

    QString info = QString("Connecting to 4G Server, %1:%2").arg(serverIp).arg(serverPort);
    logworker.addLogger(info, LOGTYPE_RECORD);

    return 1;
}

int PlatformTcp::disconnectServer()
{
    beValidConnection = false;

    // 停止心跳和健康检查
    heartbeatTimer->stop();
    healthCheckTimer->stop();

    if (checkClientStatus() == 1) {
        client.close();
    }

    updateConnectionStatus();
    return checkClientStatus();
}

int PlatformTcp::reconnectServer()
{
    std::cout << "PlatformTcp: Manual reconnect requested" << std::endl;
    disconnectServer();
    return connectServer();
}

void PlatformTcp::onConnected()
{
    std::cout << "PlatformTcp: Connected to server successfully!" << std::endl;

    beValidConnection = true;
    inReconnecting = false;
    currentReconnectCount = 0;
    reConnectCount = 0;

    // 停止重连定时器
    reconnectTimer->stop();
    // 新增：停止设备重启定时器
    if (enableDeviceReboot) {
        deviceRebootTimer->stop();
        std::cout << "PlatformTcp: Device reboot timer stopped - connection established" << std::endl;
        logworker.addLogger("Device reboot timer stopped - connection established", LOGTYPE_RECORD);
    }

    // 启动心跳和健康检查
    lastsendTime = QDateTime::currentDateTime();
    lastDataTime = QDateTime::currentDateTime();
    lastValidConnectionTime = QDateTime::currentDateTime();  // 新增：更新有效连接时间
    heartbeatTimer->start(heartbeatInterval);
    healthCheckTimer->start(NETWORK_CHECK_INTERVAL);

    updateConnectionStatus();
    emit connectionEstablished();

    QString info = QString("Successfully connected to server %1:%2").arg(serverIp).arg(serverPort);
    logworker.addLogger(info, LOGTYPE_RECORD);
}

void PlatformTcp::onDisconnected()
{
    std::cout << "PlatformTcp: Disconnected from server" << std::endl;

    beValidConnection = false;

    // 停止心跳和健康检查
    heartbeatTimer->stop();
    healthCheckTimer->stop();

    // 新增：启动设备重启定时器
    if (enableDeviceReboot) {
        deviceRebootTimer->start(deviceRebootTimeout);

        QString info = QString("Device reboot timer started - will reboot in %1 minutes if no connection")
                .arg(deviceRebootTimeout / 60000);
        std::cout << "PlatformTcp: " << info.toStdString() << std::endl;
        logworker.addLogger(info, LOGTYPE_RECORD);
    }

    updateConnectionStatus();
    emit connectionLost();

    QString info = "Platform disconnected from server";
    logworker.addLogger(info, LOGTYPE_RECORD);

    // 如果不是主动断开且重连次数未达上限，开始重连
    if (!inReconnecting && currentReconnectCount < maxReconnectAttempts) {
        startReconnectProcess();
    }
}

void PlatformTcp::triggerDeviceReboot()
{
    qint64 disconnectedTime = lastValidConnectionTime.msecsTo(QDateTime::currentDateTime());

    QString info = QString("15-minute connection timeout reached. Device has been disconnected for %1 minutes. Triggering device reboot...")
                  .arg(disconnectedTime / 60000);

    std::cout << "PlatformTcp: " << info.toStdString() << std::endl;
    logworker.addLogger(info, LOGTYPE_RECORD);

    // 记录重启原因到日志
    logworker.addLogger("DEVICE_REBOOT_REASON: Platform connection timeout (15 minutes)", LOGTYPE_RECORD);

    // 停止所有定时器和连接尝试
    stop();

    // 执行系统重启
    std::cout << "PlatformTcp: Executing system reboot command..." << std::endl;
    logworker.addLogger("Executing system reboot...", LOGTYPE_RECORD);

    // 刷新日志缓冲区确保日志被写入
    //QCoreApplication::processEvents();

    // 执行重启命令
    int result = system("reboot");
    if (result != 0) {
        std::cout << "PlatformTcp: Reboot command failed, trying alternative method" << std::endl;
        system("sync && reboot -f");
    }
}

void PlatformTcp::setDeviceRebootTimeout(int timeoutMinutes)
{
    deviceRebootTimeout = timeoutMinutes * 60000; // 转换为毫秒

    QString info = QString("Device reboot timeout set to %1 minutes").arg(timeoutMinutes);
    std::cout << "PlatformTcp: " << info.toStdString() << std::endl;
    logworker.addLogger(info, LOGTYPE_RECORD);
}

void PlatformTcp::setDeviceRebootEnabled(bool enabled)
{
    enableDeviceReboot = enabled;

    if (!enabled && deviceRebootTimer->isActive()) {
        deviceRebootTimer->stop();
        logworker.addLogger("Device reboot timer disabled and stopped", LOGTYPE_RECORD);
    }

    QString info = QString("Device reboot feature %1").arg(enabled ? "enabled" : "disabled");
    std::cout << "PlatformTcp: " << info.toStdString() << std::endl;
    logworker.addLogger(info, LOGTYPE_RECORD);
}

bool PlatformTcp::isDeviceRebootEnabled() const
{
    return enableDeviceReboot;
}

int PlatformTcp::getDeviceRebootTimeout() const
{
    return deviceRebootTimeout / 60000; // 返回分钟数
}

void PlatformTcp::onSocketError(QAbstractSocket::SocketError error)
{
    std::cout << "PlatformTcp: Socket error: " << client.errorString().toStdString() << std::endl;

    beValidConnection = false;
    updateConnectionStatus();

    QString errorInfo = QString("Socket error: %1").arg(client.errorString());
    logworker.addLogger(errorInfo, LOGTYPE_RECORD);

    // 根据错误类型决定是否重连
    switch(error) {
        case QAbstractSocket::RemoteHostClosedError:
            logworker.addLogger("Remote host closed connection", LOGTYPE_RECORD);
            break;
        case QAbstractSocket::HostNotFoundError:
            logworker.addLogger("Host not found. Please check host name and port settings.", LOGTYPE_PRINT);
            break;
        case QAbstractSocket::ConnectionRefusedError:
            logworker.addLogger("Connection refused. Make sure the server is running.", LOGTYPE_PRINT);
            break;
        case QAbstractSocket::NetworkError:
            logworker.addLogger("Network error occurred", LOGTYPE_RECORD);
            break;
        default:
            break;
    }

    // 对于需要重连的错误类型
    if (error == QAbstractSocket::RemoteHostClosedError ||
        error == QAbstractSocket::NetworkError ||
        error == QAbstractSocket::ConnectionRefusedError) {

        if (!inReconnecting && currentReconnectCount < maxReconnectAttempts) {
            startReconnectProcess();
        }
    }
}

void PlatformTcp::processReadyRead()
{
    QByteArray data = client.readAll();
    if (data.size() == 0) return;

    // 更新最后数据接收时间
    lastDataTime = QDateTime::currentDateTime();

    findItemPackets(data);

    QString recdata = data.toHex().toUpper();
    QString recData = QString("Platform Receive:%1").arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
}

void PlatformTcp::processReadyWrite(QByteArray data)
{
    SendData(data);
}

void PlatformTcp::attemptReconnect()
{
    if (currentReconnectCount >= maxReconnectAttempts) {
        std::cout << "PlatformTcp: Max reconnection attempts reached, giving up" << std::endl;
        inReconnecting = false;
        emit maxReconnectAttemptsReached();
        logworker.addLogger("Max reconnection attempts reached", LOGTYPE_RECORD);
        return;
    }

    currentReconnectCount++;
    std::cout << "PlatformTcp: Attempting reconnection #" << currentReconnectCount << std::endl;

    emit reconnectAttemptFailed(currentReconnectCount);

    QString info = QString("Reconnection attempt %1/%2").arg(currentReconnectCount).arg(maxReconnectAttempts);
    logworker.addLogger(info, LOGTYPE_RECORD);

    // 尝试连接
    connectServer();
}

void PlatformTcp::sendHeartbeat()
{
    if (!beValidConnection) return;

    // 检查是否需要发送心跳
    qint64 timeSinceLastSend = lastsendTime.msecsTo(QDateTime::currentDateTime());
    if (timeSinceLastSend < (heartbeatInterval - 5000)) { // 提前5秒检查
        return;
    }

    std::cout << "PlatformTcp: Sending heartbeat" << std::endl;

    QByteArray heartbeatData;
    if (generateHeartbeatPacket(heartbeatData) && heartbeatData.size() > 0) {
        SendData(heartbeatData);
    }
}

void PlatformTcp::checkConnectionHealth()
{
    if (!beValidConnection) return;

    // 检查是否长时间没有数据交互
    qint64 timeSinceLastData = lastDataTime.msecsTo(QDateTime::currentDateTime());
    if (timeSinceLastData > connectionTimeout) {
        std::cout << "PlatformTcp: Connection timeout detected, forcing reconnect" << std::endl;
        logworker.addLogger("Connection timeout detected", LOGTYPE_RECORD);

        beValidConnection = false;
        client.abort();
    }
}

void PlatformTcp::startReconnectProcess()
{
    if (inReconnecting) return; // 避免重复启动重连

    inReconnecting = true;
    heartbeatTimer->stop();
    healthCheckTimer->stop();

    std::cout << "PlatformTcp: Starting reconnection process" << std::endl;
    logworker.addLogger("Starting reconnection process", LOGTYPE_RECORD);

    // 延迟重连，避免频繁尝试
    reconnectTimer->start(DEFAULT_RECONNECT_DELAY);
}

void PlatformTcp::stopReconnectProcess()
{
    inReconnecting = false;
    reconnectTimer->stop();
}

void PlatformTcp::resetConnectionState()
{
    beValidConnection = false;
    inReconnecting = false;
    currentReconnectCount = 0;
    reConnectCount = 0;
    status = 0;
    lastsendTime = QDateTime::currentDateTime();
    lastDataTime = QDateTime::currentDateTime();
}

int PlatformTcp::SendData(QByteArray &data4sending)
{
    if (!beValidConnection) {
        std::cout << "PlatformTcp: Cannot send data - not connected" << std::endl;
        if (currentReconnectCount < maxReconnectAttempts) {
            startReconnectProcess();
        }
        return 0;
    }

    if (!client.isValid()) {
        std::cout << "PlatformTcp: Socket is invalid" << std::endl;
        beValidConnection = false;
        if (currentReconnectCount < maxReconnectAttempts) {
            startReconnectProcess();
        }
        return 0;
    }

    if (!client.isWritable()) {
        std::cout << "PlatformTcp: Socket is not writable" << std::endl;
        beValidConnection = false;
        if (currentReconnectCount < maxReconnectAttempts) {
            startReconnectProcess();
        }
        return 0;
    }

    qint64 bytesWritten = client.write(data4sending);

    if (bytesWritten == -1) {
        std::cout << "PlatformTcp::SendData error: " << client.errorString().toStdString() << std::endl;
        beValidConnection = false;
        client.close();
        if (currentReconnectCount < maxReconnectAttempts) {
            startReconnectProcess();
        }
        return 0;
    }

    // 等待数据发送完成
    if (!client.waitForBytesWritten(3000)) {
        std::cout << "PlatformTcp: Data send timeout" << std::endl;
        return 0;
    }

    // 重置重连计数器（发送成功表示连接正常）
    if (currentReconnectCount != 0) {
        currentReconnectCount = 0;
    }

    // 更新最后发送时间
    lastsendTime = QDateTime::currentDateTime();
    lastDataTime = QDateTime::currentDateTime();

    // 发送网络使用信号
    emit sig4GNetworkDataChanging(1, (int)bytesWritten); // SUPPORT_4GTYPE_PLATFORM4G = 1

    // 记录发送数据
    QString senddata = data4sending.toHex().toUpper();
    QString sendData = QString("Platform Send(%2):%1").arg(senddata).arg(bytesWritten);
    logworker.addLogger(sendData, LOGTYPE_PRINT);

    return (int)bytesWritten;
}

int PlatformTcp::checkClientStatus()
{
    switch (client.state()) {
    case QAbstractSocket::ConnectedState:
        status = 1;
        break;
    case QAbstractSocket::UnconnectedState:
        status = 0;
        break;
    case QAbstractSocket::ConnectingState:
        status = 0; // 连接中状态暂时视为未连接
        break;
    default:
        std::cout << "PlatformTcp: Unknown status=" << client.state() << std::endl;
        status = 0;
        break;
    }

    updateConnectionStatus();
    return status;
}

void PlatformTcp::updateConnectionStatus()
{
    emit clientStatusChanged(status);
}

bool PlatformTcp::parse_basicInfo(QByteArray &data)
{
    heartbeat.clear();
    if (gProtocolFor4GServer.checkProtocolData(data, heartbeat) == 1) {
        SendData(heartbeat);
        return true;
    }
    return false;
}

void PlatformTcp::findItemPackets(QByteArray &data)
{
    if (parse_basicInfo(data)) {
        // 心跳处理成功，更新连接状态
        if (beValidConnection == false) {
            beValidConnection = true;
            updateConnectionStatus();
        }
    } else {
        // 添加数据到发送缓冲区
        dataItem_Platform *dataitem = new dataItem_Platform;
        logworker.addLogger("PlatformTcp::findItemPackets - new dataItem_Platform", LOGTYPE_PRINT_RECORD);
        dataitem->dstIPAddress = "192.168.0.99";
        dataitem->dataFromPlatform = data;
        dataitem->datatime = QTime::currentTime();
        dataMgr_FromPlatform.addPlatformDataItem(dataitem);
    }
}

bool PlatformTcp::generateHeartbeatPacket(QByteArray &heartbeatData)
{
    // 使用已有的心跳数据，如果存在的话
    if (heartbeat.size() > 5) {
        heartbeatData = heartbeat;
        return true;
    }

    // 如果没有心跳数据，生成一个基本的心跳包
    // 这里需要根据您的协议来实现
    heartbeatData = gProtocolFor4GServer.makeHearbeat();
    return true;
}

int PlatformTcp::clientConfig()
{
    // 这个方法在新版本中已经被setupSocketConnections()替代
    // 保留是为了兼容性
    return 1;
}

void PlatformTcp::checkNetworkStatus()
{
    std::cout << "PlatformTcp: Checking network status" << std::endl;

    if (checkClientStatus() == 0 && !inReconnecting) {
        std::cout << "PlatformTcp: Network check failed, starting reconnect" << std::endl;
        startReconnectProcess();
    }

    // 检查心跳
    if (beValidConnection && heartbeat.size() > 5) {
        qint64 timeSinceLastSend = lastsendTime.secsTo(QDateTime::currentDateTime());
        if (timeSinceLastSend > 50) { // 50秒没有发送数据
            std::cout << "PlatformTcp: Sending scheduled heartbeat" << std::endl;
            SendData(heartbeat);
        }
    }
}

void PlatformTcp::processNewConnection()
{
    inReconnecting = false;
    if (beValidConnection) return;

    QString info = "Platform processNewConnection - attempting reconnect";
    logworker.addLogger(info, LOGTYPE_RECORD);
    reconnectServer();
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

// Getter methods
int PlatformTcp::getStatus() const
{
    return status;
}

bool PlatformTcp::isConnected() const
{
    return beValidConnection && client.state() == QAbstractSocket::ConnectedState;
}

bool PlatformTcp::isReconnecting() const
{
    return inReconnecting;
}

int PlatformTcp::getCurrentReconnectCount() const
{
    return currentReconnectCount;
}

// Setter methods
void PlatformTcp::setMaxReconnectAttempts(int attempts)
{
    maxReconnectAttempts = attempts;
}

void PlatformTcp::setHeartbeatInterval(int interval)
{
    heartbeatInterval = interval;
    if (heartbeatTimer->isActive()) {
        heartbeatTimer->start(interval);
    }
}

void PlatformTcp::setConnectionTimeout(int timeout)
{
    connectionTimeout = timeout;
}
