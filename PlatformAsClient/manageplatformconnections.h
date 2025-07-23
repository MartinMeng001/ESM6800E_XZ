#ifndef MANAGEPLATFORMCONNECTIONS_H
#define MANAGEPLATFORMCONNECTIONS_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QHostAddress>
#include <QTcpSocket>
#include "itemplatformconnection.h"

class ManagePlatformConnections : public QObject
{
    Q_OBJECT
public:
    explicit ManagePlatformConnections(QObject *parent = nullptr);

    int addNewConnection(ItemPlatformConnection* newConn);
    ItemPlatformConnection* findSocketByAnnuIP(QString annuip);
    ItemPlatformConnection *findSocketFirst();

    int closeAllConnections();

signals:

public slots:
    void refreshClients();
    void ClientsHeartBeat();
    void ClientHeartBeat(QString annuip);
    void sendData2Annuniator(QString annuip, QByteArray data);
    void sendData2AllClients(QByteArray data);
protected:
    void checkValid();
    bool checkExisted(QString ip);
private:
    QMap<QString, ItemPlatformConnection*> socketmap;
};
extern ManagePlatformConnections manager_PlatformClientConnections;

#endif // MANAGEPLATFORMCONNECTIONS_H
