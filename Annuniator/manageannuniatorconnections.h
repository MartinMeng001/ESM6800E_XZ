#ifndef MANAGEANNUNIATORCONNECTIONS_H
#define MANAGEANNUNIATORCONNECTIONS_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QByteArray>
#include <QHostAddress>
#include <QTcpSocket>
#include "itemannuniatorconnection.h"

class ManageAnnuniatorConnections : public QObject
{
    Q_OBJECT
public:
    explicit ManageAnnuniatorConnections(QObject *parent = nullptr);

    int addNewConnection(ItemAnnuniatorConnection* newConn);
    ItemAnnuniatorConnection* findSocketByAnnuIP(QString annuip);
    ItemAnnuniatorConnection *findSocketFirst();

    int closeAllConnections();

signals:

public slots:
    void refreshClients();
    void ClientsHeartBeat();
    void ClientHeartBeat(QString annuip);
    void sendData2Annuniator(QString annuip, QByteArray data);
protected:
    void checkValid();
    bool checkExisted(QString ip);
private:
    QMap<QString, ItemAnnuniatorConnection*> socketmap;
    QMutex mutex_Annuniator;

};
extern ManageAnnuniatorConnections manager_AnnuniatorConnections;

#endif // MANAGEANNUNIATORCONNECTIONS_H
