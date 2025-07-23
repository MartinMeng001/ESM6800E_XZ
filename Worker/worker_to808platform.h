#ifndef WORKER_TO808PLATFORM_H
#define WORKER_TO808PLATFORM_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QString>

class Worker_To808Platform : public QObject
{
    Q_OBJECT
public:
    explicit Worker_To808Platform(QObject *parent = nullptr);
    void initWorker();

signals:
    void sendDataTo808Platform(QByteArray data);
    void sendHeartbeatTo808Platform();
public slots:
    void work4Platform();
signals:

};

class Controller_808Platform : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_808Platform();
    ~Controller_808Platform();

    void startPlatformWorker();
    void stopWorker();
public slots:

signals:
    void startWorker();

};
extern Controller_808Platform controller808Platform;

#endif // WORKER_TO808PLATFORM_H
