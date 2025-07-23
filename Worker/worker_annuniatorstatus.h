#ifndef WORKER_ANNUNIATORSTATUS_H
#define WORKER_ANNUNIATORSTATUS_H

#include <QObject>
#include <QThread>

class Worker_AnnuniatorStatus : public QObject
{
    Q_OBJECT
public:
    explicit Worker_AnnuniatorStatus(QObject *parent = nullptr);

signals:
    void sigStartGetAnnuniatorTimeWorker();
    void sigStartAnnuniatorStatusSlotsCommon();
public slots:
    void work4AnnuniatorStatus();
};

class Controller_AnnuniatorStatus : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_AnnuniatorStatus(QObject *parent = nullptr);
    ~Controller_AnnuniatorStatus();

    void stopWorker();
    void startStatusWorker();
public slots:

signals:
    void startWorker();
};
extern Controller_AnnuniatorStatus annuniatorControllerStatus;

#endif // WORKER_ANNUNIATORSTATUS_H
