#ifndef PLATFORM808TCP_H
#define PLATFORM808TCP_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDateTime>
#include <atomic>

class Platform808Tcp : public QObject
{
    Q_OBJECT
public:
    explicit Platform808Tcp(QObject *parent = nullptr);

    int start(QString ip, int port);
    int close();
    int SendData(QByteArray& data4sending);
    int getStatus() const;
signals:
    void clientStatusChanged(int status);
    void sig4GNetworkDataChanging(int type, int datasize);
public slots:
    void processNewConnection();
    void processConnected();
    void processDisconnected();
    void processError(QAbstractSocket::SocketError socketError);
    void processReadyRead();
    void processReadyWrite(QByteArray data);
    //void processExtraData4Annuniator(QByteArray data);

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
    std::atomic<bool> beValidConnection;
    std::atomic<bool> beStopNew;
    std::atomic<bool> inReconnecting;
    int status;
    QString serverIp;
    int serverPort;
    QTcpSocket client;
    std::atomic<int> reConnectCount;
    QDateTime lastsendTime;
};
extern Platform808Tcp gPlatform808Tcp;
#endif // PLATFORM808TCP_H
