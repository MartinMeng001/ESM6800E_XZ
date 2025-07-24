#ifndef MANAGEANNUNIATORCONNECTIONS_H
#define MANAGEANNUNIATORCONNECTIONS_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QByteArray>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include "itemannuniatorconnection.h"

class ManageAnnuniatorConnections : public QObject
{
    Q_OBJECT
public:
    explicit ManageAnnuniatorConnections(QObject *parent = nullptr);
    ~ManageAnnuniatorConnections();

    // 连接管理
    int addNewConnection(ItemAnnuniatorConnection* newConn);
    ItemAnnuniatorConnection* findSocketByAnnuIP(QString annuip);
    ItemAnnuniatorConnection* findSocketFirst();
    int closeAllConnections();
    void initialize();
    // 新增：连接统计和状态
    int getConnectionCount() const;
    QStringList getConnectedIPs() const;
    bool hasValidConnections() const;

    // 新增：连接健康检查
    void performHealthCheck();
    int removeInvalidConnections();

signals:
    void connectionCountChanged(int count);
    void connectionAdded(const QString& ip);
    void connectionRemoved(const QString& ip);

public slots:
    void refreshClients();
    void ClientsHeartBeat();
    void ClientHeartBeat(QString annuip);
    void sendData2Annuniator(QString annuip, QByteArray data);

    // 新增：定期清理无效连接
    void performPeriodicCleanup();

protected:
    void checkValid();
    bool checkExisted(QString ip);

    // 新增：安全移除连接
    void removeConnection(const QString& ip);
    void removeConnection(QMap<QString, ItemAnnuniatorConnection*>::iterator& it);

    // 新增：连接状态验证
    bool isConnectionValid(ItemAnnuniatorConnection* conn) const;

private:
    QMap<QString, ItemAnnuniatorConnection*> socketmap;
    mutable QMutex mutex_Annuniator;  // 保护socketmap的互斥锁

    // 新增：定期清理定时器
    QTimer* cleanupTimer;
    static const int CLEANUP_INTERVAL = 30000;  // 30秒清理间隔

    // 新增：连接统计
    int lastConnectionCount;
    bool initialized;

    void emitConnectionCountChanged();
};

extern ManageAnnuniatorConnections *manager_AnnuniatorConnections;

#endif // MANAGEANNUNIATORCONNECTIONS_H
