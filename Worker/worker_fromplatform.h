#ifndef WORKER_FROMPLATFORM_H
#define WORKER_FROMPLATFORM_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QString>

class Worker_FromPlatform : public QObject
{
    Q_OBJECT
public:
    explicit Worker_FromPlatform(QObject *parent = nullptr);
    void initWorker();

signals:
    void sendData2Annuniator(QString ip, QByteArray data);
public slots:
    void work4Platform();

};
class Controller_Platform : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_Platform();
    ~Controller_Platform();

    void startPlatformWorker();
    void stopWorker();
public slots:

signals:
    void startWorker();

};
extern Controller_Platform controllerPlatform;
#endif // WORKER_FROMPLATFORM_H
