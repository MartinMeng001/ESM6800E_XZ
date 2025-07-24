#ifndef TCPSERVER4ANNUNIATOR_H
#define TCPSERVER4ANNUNIATOR_H

#include <QObject>
#include <QTcpServer>
#include <QTimer>
#include <QMutex>

class TcpServer4Annuniator : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer4Annuniator(QObject *parent = nullptr);
    ~TcpServer4Annuniator();

    int start(int port);
    int close();
    int stopNewConnection();
    int resumeNewConnection();  // 新增：恢复接收新连接
    int getStatus() const;
    // 新增：初始化函数
    void initialize();

    // 新增：获取连接统计信息
    int getActiveConnectionCount() const;

signals:
    void serverStatusChanged(int status);
    void newConnectionEstablished(const QString& clientIP);
    void connectionValidationFailed(const QString& clientIP);

public slots:
    void processNewConnection();
    void onConnectionValidationTimeout();  // 新增：验证超时处理

protected:
    int checkServerStatus();
    int closeServer();
    int startServer();
    int rebuildServer();
    int serverConfig();

    // 改进的延时函数
    void Delay_MSec(int msec);

    // 新增：清理无效连接
    void cleanupInvalidConnections();

private:
    bool beStopNew;
    int status;
    int listenPort;
    QTcpServer server;

    // 新增：连接验证相关
    QTimer* validationTimer;
    QMutex serverMutex;  // 新增：线程安全保护

    // 新增：服务器配置参数
    static const int MAX_PENDING_CONNECTIONS = 10;
    static const int CONNECTION_VALIDATION_TIMEOUT = 10000;  // 10秒验证超时

    bool serverConfigured;  // 防止重复配置
    bool initialized;
};

extern TcpServer4Annuniator *annuniatorTcpServer;

#endif // TCPSERVER4ANNUNIATOR_H
