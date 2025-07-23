#ifndef WORKER_FROMPLATFORMCLIENT_H
#define WORKER_FROMPLATFORMCLIENT_H

#include <QObject>
#include <QThread>

class Worker_fromPlatformClient : public QObject
{
    Q_OBJECT
public:
    explicit Worker_fromPlatformClient(QObject *parent = nullptr);

signals:
    void sendData2Annuniator(QString ip, QByteArray data);
public slots:
    void work4PlatformClient();
};

class Controller_PlatformClient : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_PlatformClient(QObject *parent = nullptr);
    ~Controller_PlatformClient();

    void stopWorker();
    void startSendWorker();
public slots:

signals:
    void startWorker();
};
extern Controller_PlatformClient platformClientController;
#endif // WORKER_FROMPLATFORMCLIENT_H
