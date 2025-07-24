#ifndef PLATFORMTCP_H
#define PLATFORMTCP_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QTime>

class PlatformTcp : public QObject
{
    Q_OBJECT

public:
    explicit PlatformTcp(QObject *parent = nullptr);
    ~PlatformTcp();

    // 公共接口
    int start(const QString &ip, int port);
    int stop();
    int connectServer();
    int disconnectServer();
    int reconnectServer();
    int SendData(QByteArray &data4sending);
    int checkClientStatus();
    int getStatus() const;

    // 配置方法
    void setMaxReconnectAttempts(int attempts);
    void setHeartbeatInterval(int interval);
    void setConnectionTimeout(int timeout);

    // 状态查询
    bool isConnected() const;
    bool isReconnecting() const;
    int getCurrentReconnectCount() const;

    // 设备重启配置方法
    void setDeviceRebootTimeout(int timeoutMinutes);
    void setDeviceRebootEnabled(bool enabled);
    bool isDeviceRebootEnabled() const;
    int getDeviceRebootTimeout() const;

signals:
    void clientStatusChanged(int status);
    void sig4GNetworkDataChanging(int type, int size);
    void connectionEstablished();
    void connectionLost();
    void reconnectAttemptFailed(int attempt);
    void maxReconnectAttemptsReached();

public slots:
    void checkNetworkStatus();
    void processNewConnection();
    void processExtraData4Annuniator(QByteArray data);
    void processReadyRead();
    void processReadyWrite(QByteArray data);

private slots:
    void onConnected();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

    void attemptReconnect();
    void sendHeartbeat();
    void checkConnectionHealth();
    void triggerDeviceReboot();             // 触发设备重启

protected:
    // 内部方法
    void setupSocketConnections();
    void startReconnectProcess();
    void stopReconnectProcess();
    void resetConnectionState();
    void updateConnectionStatus();

    // 协议相关
    bool parse_basicInfo(QByteArray &data);
    void findItemPackets(QByteArray &data);
    bool generateHeartbeatPacket(QByteArray &heartbeat);

    // 配置相关
    int clientConfig();

private:
    // 网络相关
    QTcpSocket client;
    QString serverIp;
    int serverPort;

    // 状态管理
    bool beValidConnection;
    bool inReconnecting;
    int status;
    int reConnectCount;
    int currentReconnectCount;
    int maxReconnectAttempts;

    // 定时器
    QTimer *reconnectTimer;
    QTimer *heartbeatTimer;
    QTimer *healthCheckTimer;

    // 心跳和健康检查
    QByteArray heartbeat;
    QDateTime lastsendTime;
    QDateTime lastDataTime;
    int heartbeatInterval;
    int connectionTimeout;
    // 15分钟重启定时器相关
    QTimer *deviceRebootTimer;              // 15分钟重启定时器
    bool enableDeviceReboot;                // 启用设备重启功能
    int deviceRebootTimeout;                // 设备重启超时时间(毫秒)
    QDateTime lastValidConnectionTime;      // 最后有效连接时间

    // 常量定义
    static const int DEFAULT_RECONNECT_DELAY = 5000;        // 5秒重连延迟
    static const int DEFAULT_HEARTBEAT_INTERVAL = 30000;    // 30秒心跳间隔
    static const int DEFAULT_CONNECTION_TIMEOUT = 60000;    // 60秒连接超时
    static const int DEFAULT_MAX_RECONNECT_ATTEMPTS = 5;    // 默认最大重连次数
    static const int NETWORK_CHECK_INTERVAL = 10000;       // 10秒网络检查间隔
    static const int DEFAULT_DEVICE_REBOOT_TIMEOUT = 900000;  // 15分钟 (毫秒)
};

// 全局实例声明
extern PlatformTcp gPlatformTcpv2;

#endif // PLATFORMTCP_H
