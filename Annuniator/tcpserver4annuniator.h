#ifndef TCPSERVER4ANNUNIATOR_H
#define TCPSERVER4ANNUNIATOR_H

#include <QObject>
#include <QTcpServer>

class TcpServer4Annuniator : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer4Annuniator(QObject *parent = nullptr);

    int start(int port);
    int close();
    int stopNewConnection();
    int getStatus() const;
signals:
    void serverStatusChanged(int status);
public slots:
    void processNewConnection();

protected:
    int checkServerStatus();
    int closeServer();
    int startServer();
    int rebuildServer();
    int serverConfig();

    void Delay_MSec(int msec);
private:
    bool beStopNew;
    int status;
    int listenPort;
    QTcpServer server;
};
extern TcpServer4Annuniator annuniatorTcpServer;

#endif // TCPSERVER4ANNUNIATOR_H
