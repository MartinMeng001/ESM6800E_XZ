#include <iostream>
#include <QHostAddress>
#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "datamgr_fromannuniator.h"
#include "itemannuniatorconnection.h"
#include "Protocol/protocolannuniator4u.h"

ItemAnnuniatorConnection::ItemAnnuniatorConnection(QObject *parent) : QObject(parent)
{
    psocket = nullptr;
    annuniatorIP = "";
    beExiting = false;
    beValidConnection = false;  // 初始化为false，需要验证
    validationInProgress = false;
    validationAttempts = 0;

    // 初始化定时器
    validationTimer = new QTimer(this);
    validationTimer->setSingleShot(true);
    connect(validationTimer, &QTimer::timeout, this, &ItemAnnuniatorConnection::onValidationTimeout);

    heartbeatTimer = new QTimer(this);
    heartbeatTimer->setSingleShot(false);
    heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
    connect(heartbeatTimer, &QTimer::timeout, this, &ItemAnnuniatorConnection::req_basicInfo);

    lastDataTime = QTime::currentTime();

    logworker.addLogger("ItemAnnuniatorConnection created", LOGTYPE_PRINT_RECORD);
}

ItemAnnuniatorConnection::~ItemAnnuniatorConnection()
{
    logworker.addLogger(QString("Destroying ItemAnnuniatorConnection for IP: %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    // 停止所有定时器
    if (validationTimer) {
        validationTimer->stop();
        delete validationTimer;
        validationTimer = nullptr;
    }

    if (heartbeatTimer) {
        heartbeatTimer->stop();
        delete heartbeatTimer;
        heartbeatTimer = nullptr;
    }

    // 安全地清理socket连接
    if (psocket != nullptr) {
        disConnect();

        // 使用deleteLater以避免在信号处理过程中删除对象
        psocket->deleteLater();
        psocket = nullptr;
    }

    logworker.addLogger(QString("ItemAnnuniatorConnection destroyed for IP: %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
}

void ItemAnnuniatorConnection::initItem(QTcpSocket *socket_new)
{
    QMutexLocker locker(&connectionMutex);

    if (psocket != nullptr) {
        logworker.addLogger("Attempt to initialize already initialized connection", LOGTYPE_PRINT);
        return;
    }

    if (socket_new == nullptr) {
        logworker.addLogger("Attempt to initialize with null socket", LOGTYPE_PRINT);
        return;
    }

    psocket = socket_new;

    // 获取客户端IP地址
    QString ipnow = psocket->peerAddress().toString();
    if (ipnow.startsWith("::ffff:")) {
        annuniatorIP = ipnow.mid(QString("::ffff:").size());
    } else {
        annuniatorIP = ipnow;
    }

    logworker.addLogger(QString("Initializing connection for AnnuniatorIP: %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    // 配置socket连接
    if (!config()) {
        logworker.addLogger(QString("Failed to configure connection for %1").arg(annuniatorIP), LOGTYPE_PRINT);
        return;
    }

    // 重置连接状态
    beValidConnection = false;
    beExiting = false;
    lastDataTime = QTime::currentTime();

    // 开始验证过程
    startValidationProcess();
}

bool ItemAnnuniatorConnection::config()
{
    if (psocket == nullptr) {
        logworker.addLogger("Cannot configure null socket", LOGTYPE_PRINT);
        return false;
    }

    try {
        // 先断开可能存在的旧连接
        disconnect(psocket, nullptr, this, nullptr);

        // 连接socket信号
        connect(psocket, &QTcpSocket::readyRead, this, &ItemAnnuniatorConnection::processReadyRead);
        connect(psocket, &QTcpSocket::disconnected, this, &ItemAnnuniatorConnection::processDisconnected);

        // 连接错误处理信号
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        // Qt 5.15+ 使用 errorOccurred
        connect(psocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                this, &ItemAnnuniatorConnection::onSocketError);
#else
        // Qt 5.15之前的版本使用 error
        connect(psocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif

        // 连接数据处理信号
        connect(this, &ItemAnnuniatorConnection::sigNewData,
                &gProtocolAnnuniator4U, &ProtocolAnnuniator4U::processAnnuniatorData);

        logworker.addLogger(QString("Socket configured successfully for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
        return true;

    } catch (const std::exception& e) {
        logworker.addLogger(QString("Exception during socket configuration for %1: %2").arg(annuniatorIP).arg(e.what()), LOGTYPE_PRINT);
        return false;
    } catch (...) {
        logworker.addLogger(QString("Unknown exception during socket configuration for %1").arg(annuniatorIP), LOGTYPE_PRINT);
        return false;
    }
}

void ItemAnnuniatorConnection::startValidationProcess()
{
    QMutexLocker locker(&connectionMutex);

    if (validationInProgress) {
        return;  // 验证已在进行中
    }

    validationInProgress = true;
    validationAttempts = 0;

    logworker.addLogger(QString("Starting validation process for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    // 启动验证超时定时器
    validationTimer->start(VALIDATION_TIMEOUT);

    // 立即发送第一个验证请求
    req_basicInfo();
}

void ItemAnnuniatorConnection::stopValidationProcess()
{
    QMutexLocker locker(&connectionMutex);

    if (!validationInProgress) {
        return;
    }

    validationInProgress = false;
    validationTimer->stop();

    if (beValidConnection) {
        // 验证成功，开始心跳
        heartbeatTimer->start();
        emit connectionValidated();
        logworker.addLogger(QString("Connection validated successfully for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
    } else {
        // 验证失败
        emit connectionFailed();
        logworker.addLogger(QString("Connection validation failed for %1").arg(annuniatorIP), LOGTYPE_PRINT);
    }
}

void ItemAnnuniatorConnection::onValidationTimeout()
{
    QMutexLocker locker(&connectionMutex);

    if (!validationInProgress) {
        return;
    }

    validationAttempts++;

    if (validationAttempts >= MAX_VALIDATION_ATTEMPTS) {
        logworker.addLogger(QString("Validation timeout for %1 after %2 attempts").arg(annuniatorIP).arg(validationAttempts), LOGTYPE_PRINT);
        validationInProgress = false;
        beValidConnection = false;
        emit connectionFailed();
    } else {
        // 重试验证
        logworker.addLogger(QString("Retrying validation for %1 (attempt %2/%3)").arg(annuniatorIP).arg(validationAttempts).arg(MAX_VALIDATION_ATTEMPTS), LOGTYPE_PRINT_RECORD);
        req_basicInfo();
        validationTimer->start(VALIDATION_TIMEOUT);
    }
}

int ItemAnnuniatorConnection::SendData(QByteArray &data4sending)
{
    QMutexLocker locker(&connectionMutex);

    if (beExiting) {
        logworker.addLogger(QString("Cannot send data to %1 - connection is exiting").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
        return 0;
    }

    if (psocket == nullptr) {
        logworker.addLogger(QString("Cannot send data to %1 - socket is null").arg(annuniatorIP), LOGTYPE_PRINT);
        return 0;
    }

    if (!psocket->isValid()) {
        logworker.addLogger(QString("Cannot send data to %1 - socket is invalid").arg(annuniatorIP), LOGTYPE_PRINT);
        return 0;
    }

    if (!psocket->isWritable()) {
        logworker.addLogger(QString("Cannot send data to %1 - socket is not writable").arg(annuniatorIP), LOGTYPE_PRINT);
        return 0;
    }

    int size = psocket->write(data4sending);
    if (size > 0) {
        QString senddata = data4sending.toHex().toUpper();
        QString sendData = QString("Annuniator Send to %1:%2").arg(annuniatorIP).arg(senddata);
        logworker.addLogger(sendData, LOGTYPE_PRINT);

        // 更新最后发送时间
        lastDataTime = QTime::currentTime();
    } else {
        logworker.addLogger(QString("Failed to send data to %1").arg(annuniatorIP), LOGTYPE_PRINT);
    }

    return size;
}

void ItemAnnuniatorConnection::setAnnuniatorIP(const QString &value)
{
    QMutexLocker locker(&connectionMutex);
    annuniatorIP = value;
}

bool ItemAnnuniatorConnection::getBeValidConnection() const
{
    return beValidConnection;
}

bool ItemAnnuniatorConnection::isSocketValid()
{
    QMutexLocker locker(&connectionMutex);
    return (psocket != nullptr && psocket->isValid() &&
            psocket->state() == QAbstractSocket::ConnectedState);
}

QAbstractSocket::SocketState ItemAnnuniatorConnection::getSocketState()
{
    QMutexLocker locker(&connectionMutex);
    if (psocket != nullptr) {
        return psocket->state();
    }
    return QAbstractSocket::UnconnectedState;
}

bool ItemAnnuniatorConnection::isConnectionHealthy()
{
    QMutexLocker locker(&connectionMutex);

    if (!isSocketValid()) {
        return false;
    }

    // 检查是否超时
    int timeSinceLastData = lastDataTime.msecsTo(QTime::currentTime());
    if (timeSinceLastData > CONNECTION_TIMEOUT) {
        return false;
    }

    return true;
}

void ItemAnnuniatorConnection::resetConnectionTimeout()
{
    QMutexLocker locker(&connectionMutex);
    lastDataTime = QTime::currentTime();
}

void ItemAnnuniatorConnection::req_basicInfo()
{
    QByteArray dataarray;
    dataarray.resize(10);
    dataarray[0] = 0x53; dataarray[1] = 0x42; dataarray[2] = 0x44; dataarray[3] = 0x5A;
    dataarray[4] = 0x00; dataarray[5] = 0x80; dataarray[6] = 0x01; dataarray[7] = 0x81;
    dataarray[8] = 0xAA; dataarray[9] = 0x55;

    if (SendData(dataarray) > 0) {
        timeMeasuring.start();
        logworker.addLogger(QString("Requested basic info from %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
    } else {
        logworker.addLogger(QString("Failed to request basic info from %1").arg(annuniatorIP), LOGTYPE_PRINT);
    }
}

int ItemAnnuniatorConnection::getBasicInfoElapsed()
{
    return timeMeasuring.getElapsedTime();
}

void ItemAnnuniatorConnection::closeSocket()
{
    QMutexLocker locker(&connectionMutex);

    logworker.addLogger(QString("Closing socket for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    beExiting = true;
    stopValidationProcess();
    disConnect();
}

void ItemAnnuniatorConnection::processReadyRead()
{
    QMutexLocker locker(&connectionMutex);

    if (beExiting) {
        return;  // 如果正在退出，不处理数据
    }

    if (psocket == nullptr || !psocket->isValid()) {
        logworker.addLogger(QString("Received readyRead signal but socket is invalid for %1").arg(annuniatorIP), LOGTYPE_PRINT);
        return;
    }

    QByteArray data = psocket->readAll();
    if (data.size() == 0) {
        return;
    }

    // 更新最后数据接收时间
    lastDataTime = QTime::currentTime();

    // 处理接收到的数据
    findItemPackets(data);

    QString recdata = data.toHex().toUpper();
    QString recData = QString("Annuniator Receive from %1:%2").arg(annuniatorIP).arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
}

void ItemAnnuniatorConnection::processReadyWrite(QByteArray &data)
{
    SendData(data);
}

void ItemAnnuniatorConnection::processDisconnected()
{
    QMutexLocker locker(&connectionMutex);

    logworker.addLogger(QString("Socket disconnected for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    beValidConnection = false;
    beExiting = true;

    // 停止所有定时器
    if (validationTimer) {
        validationTimer->stop();
    }
    if (heartbeatTimer) {
        heartbeatTimer->stop();
    }

    // 发送连接失败信号
    emit connectionFailed();
}

void ItemAnnuniatorConnection::onSocketError(QAbstractSocket::SocketError error)
{
    QMutexLocker locker(&connectionMutex);

    QString errorString = psocket ? psocket->errorString() : "Unknown error";
    logworker.addLogger(QString("Socket error for %1: %2 (%3)").arg(annuniatorIP).arg(errorString).arg(error), LOGTYPE_PRINT);

    beValidConnection = false;

    // 根据错误类型决定处理方式
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            logworker.addLogger(QString("Remote host %1 closed connection").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
            break;
        case QAbstractSocket::HostNotFoundError:
            logworker.addLogger(QString("Host %1 not found").arg(annuniatorIP), LOGTYPE_PRINT);
            break;
        case QAbstractSocket::ConnectionRefusedError:
            logworker.addLogger(QString("Connection to %1 refused").arg(annuniatorIP), LOGTYPE_PRINT);
            break;
        case QAbstractSocket::NetworkError:
            logworker.addLogger(QString("Network error for %1").arg(annuniatorIP), LOGTYPE_PRINT);
            break;
        default:
            break;
    }

    emit connectionFailed();
}

void ItemAnnuniatorConnection::disConnect()
{
    if (psocket == nullptr) {
        return;
    }

    logworker.addLogger(QString("Disconnecting socket for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);

    // 断开所有信号连接
    disconnect(psocket, &QTcpSocket::readyRead, this, &ItemAnnuniatorConnection::processReadyRead);
    disconnect(psocket, &QTcpSocket::disconnected, this, &ItemAnnuniatorConnection::processDisconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    disconnect(psocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
               this, &ItemAnnuniatorConnection::onSocketError);
#else
    disconnect(psocket, SIGNAL(error(QAbstractSocket::SocketError)),
               this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif
    disconnect(this, &ItemAnnuniatorConnection::sigNewData,
               &gProtocolAnnuniator4U, &ProtocolAnnuniator4U::processAnnuniatorData);

    // 停止定时器
    if (validationTimer) {
        validationTimer->stop();
    }
    if (heartbeatTimer) {
        heartbeatTimer->stop();
    }

    // 关闭socket
    if (psocket->state() != QAbstractSocket::UnconnectedState) {
        psocket->close();
    }

    timeMeasuring.end(1);
    beExiting = true;
    beValidConnection = false;
}

void ItemAnnuniatorConnection::findItemPackets(QByteArray &data)
{
    if (parse_basicInfo(data)) {
        // 基本信息验证成功
        if (!beValidConnection) {
            beValidConnection = true;
            timeMeasuring.end(0);

            if (validationInProgress) {
                stopValidationProcess();
            }

            logworker.addLogger(QString("Basic info validated for %1").arg(annuniatorIP), LOGTYPE_PRINT_RECORD);
        }
    } else {
        // 处理其他数据包
        dataItem_Annuniator *dataitem = new dataItem_Annuniator;
        dataitem->dataFromAnnuniator = data;
        dataitem->datatime = QTime::currentTime();
        dataMgrFromAnnuniator.addAnnuniatorDataItem(dataitem);
        emit sigNewData(data);
    }
}

bool ItemAnnuniatorConnection::parse_basicInfo(QByteArray &data)
{
    if (data.size() < 5) {
        return false;
    }

    // 检查基本信息包头
    if (data[0] == (char)0x80 && data[1] == (char)0xC2 &&
        data[2] == (char)0x42 && data[3] == (char)0xAA && data[4] == (char)0x55) {
        return true;
    }

    return false;
}

void ItemAnnuniatorConnection::ip2Hex(QByteArray &data, int index, QString ip)
{
    QStringList ips = ip.split(".");
    if (ips.size() == 4) {
        data[index] = ips.at(0).toInt();
        data[index + 1] = ips.at(1).toInt();
        data[index + 2] = ips.at(2).toInt();
        data[index + 3] = ips.at(3).toInt();
    } else {
        data[index] = 0;
        data[index + 1] = 0;
        data[index + 2] = 0;
        data[index + 3] = 0;
    }
}
