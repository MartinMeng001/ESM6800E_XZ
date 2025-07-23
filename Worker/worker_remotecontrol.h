#ifndef WORKER_REMOTECONTROL_H
#define WORKER_REMOTECONTROL_H

#include <QObject>
#include <QThread>

class Worker_RemoteControl : public QObject
{
    Q_OBJECT
public:
    explicit Worker_RemoteControl(QObject *parent = nullptr);
    void initWorker();

signals:

public slots:
    void work4RemoteGuard();
signals:

};

class Controller_RemoteControl : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_RemoteControl();
    ~Controller_RemoteControl();

    void startRemoteControlWorker();
    void stopWorker();
public slots:

signals:
    void startWorker();

};
extern Controller_RemoteControl controllerRemoteControl;

#endif // WORKER_REMOTECONTROL_H
