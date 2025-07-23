#ifndef WORKER_FROMANNUNIATOR_H
#define WORKER_FROMANNUNIATOR_H

#include <QObject>
#include <QThread>

class Worker_fromAnnuniator : public QObject
{
    Q_OBJECT
public:
    explicit Worker_fromAnnuniator(QObject *parent = nullptr);

signals:
    void sendData2Platform(QByteArray data);
    void checkNetworkStatus();
    void reconnect2Platform();
public slots:
    void work4Annuniator();
protected:
    //void sendDataCommon(QByteArray data, int besameport, QString externalip, int type);

};

class Controller_Annuniator : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_Annuniator(QObject *parent = nullptr);
    ~Controller_Annuniator();

    void stopWorker();
    void startSendWorker();
public slots:

signals:
    void startWorker();
};
extern Controller_Annuniator annuniatorController;

#endif // WORKER_FROMANNUNIATOR_H
