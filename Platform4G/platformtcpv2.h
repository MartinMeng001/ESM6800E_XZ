#ifndef PLATFORMTCPV2_H
#define PLATFORMTCPV2_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class PlatformTcpV2 : public QObject
{
    Q_OBJECT
public:
    explicit PlatformTcpV2(QObject *parent = nullptr);

    int start(QString ip, int port, QString annuip);
    int SendData(QByteArray& data4sending);
signals:

public slots:
    void connectToServer();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReconnect();
    void reconnect();

    void processReadyWrite(QByteArray data);
    void processReadyRead();
    void processExtraData4Annuniator(QByteArray data);

protected:
    bool parse_basicInfo(QByteArray& data);
    void findItemPackets(QByteArray& data);
private:
    QTcpSocket *socket;
    QTimer *reconnectTimer;
    QString ip="127.0.0.1";
    QString annuniatorip = "192.168.0.99";
    int port = 8125;
    QByteArray heartbeat;
};
extern PlatformTcpV2 gPlatformTcpv2;
#endif // PLATFORMTCPV2_H
