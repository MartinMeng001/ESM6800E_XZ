#ifndef PLATFORM808TCPV2_H
#define PLATFORM808TCPV2_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Platform808TcpV2 : public QObject
{
    Q_OBJECT
public:
    explicit Platform808TcpV2(QObject *parent = nullptr);

    int start(QString ip, int port);
    int SendData(QByteArray& data4sending);
    int getHearbeat(){ return heartbeat; }
signals:

public slots:
    void connectToServer();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void reconnect();

    void processReadyRead();
    void processReadyWrite(QByteArray data);

protected:
    void findItemPackets(QByteArray& data);
private:
    QTcpSocket *socket;
    QTimer *reconnectTimer;

    QString serverip;
    int serverport;
    int heartbeat;
};
extern Platform808TcpV2 gPlatform808Tcpv2;

#endif // PLATFORM808TCPV2_H
