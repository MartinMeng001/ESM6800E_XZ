#ifndef TCPSERVER4PLATFORM_H
#define TCPSERVER4PLATFORM_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

class TcpServer4Platform : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer4Platform(QObject *parent = nullptr);

    int start(int port);
    int close();
    int stopNewConnection();
    int allowNewConnection();  // 新增：允许新连接
    int getStatus() const;
    int getConnectionCount() const;  // 新增：获取连接数
    bool isStopNewEnabled() const;   // 新增：检查是否停止新连接
    void initialize();
    // 诊断方法
    void logServerStatus();          // 新增：记录服务器状态
    void logConnectionStats();       // 新增：记录连接统计

private slots:
    void processNewConnection();
    void onClientDisconnected();    // 新增：处理客户端断开

private:
    int checkServerStatus();
    int closeServer();
    int startServer();
    int rebuildServer();
    int serverConfig();
    void Delay_MSec(int msec);

    QTcpServer server;
    int status;
    int listenPort;
    bool beStopNew;
    bool initialized;
    QTimer *statusCheckTimer;        // 新增：状态检查定时器

signals:
    void serverStatusChanged(int status);
    void clientConnected(const QString &clientIP);
    void clientDisconnected(const QString &clientIP);
};

extern TcpServer4Platform *gtcpServer4Platfrom;
#endif // TCPSERVER4PLATFORM_H
