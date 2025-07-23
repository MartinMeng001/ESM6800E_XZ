#ifndef PLATFORMTCP_H
#define PLATFORMTCP_H

#include <QObject>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QDateTime>
#include <atomic>

class PlatformTcp : public QObject
{
    Q_OBJECT
public:
    explicit PlatformTcp(QObject *parent = nullptr);

    int start(QString ip, int port);
    int close();
    int SendData(QByteArray& data4sending);
    int SendData2(QByteArray& data4sending);

    int getStatus() const;
signals:
    void clientStatusChanged(int status);
    void sig4GNetworkDataChanging(int type, int datasize);
public slots:
    void processNewConnection();
    void processDisconnected();
    void processError(QAbstractSocket::SocketError socketError);
    void processReadyRead();
    void processReadyWrite(QByteArray data);
    void processExtraData4Annuniator(QByteArray data);
    void checkNetworkStatus();

protected:
    int checkClientStatus();
    int disconnectServer();
    int connectServer();
    int reconnectServer();
    int clientConfig();
    bool parse_basicInfo(QByteArray& data);
    void findItemPackets(QByteArray& data);
    void Delay_MSec(int msec);
private:
    bool beValidConnection;
    bool beStopNew;
    bool inReconnecting;
    int status;
    QString serverIp;
    int serverPort;
    QTcpSocket client;
    std::atomic<int> reConnectCount;
    QByteArray heartbeat;
    QDateTime lastsendTime;
};
extern PlatformTcp gPlatformTcp;

#endif // PLATFORMTCP_H
