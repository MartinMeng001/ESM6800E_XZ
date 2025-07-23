#ifndef WORKER_4G_H
#define WORKER_4G_H

#include <QObject>
#include <QThread>

#define TYPE4G_ME909S   1   //"insmod /lib/modules/4.1.15/usbserial.ko vendor=0x12d1 product=0x15c1"
#define TYPE4G_EC20     2   //"insmod /lib/modules/4.1.15/usbserial.ko vendor=0x12d1 product=0x15c1"
#define MAXRETRY_COUNTS 60

class Worker_4G : public QObject
{
    Q_OBJECT
public:
    explicit Worker_4G(QObject *parent = nullptr);

signals:
    void sigReset4G();
    void sigRebootDevice();
    void sigPPPidStatus(int status);
public slots:
    void workFor4G();
    void reconnect();
protected:
    int load4GDriver(int type4G);
    int startDiagMgr();
    int reset4GModule();
private:
    bool flag_Rebooting;
    int counts_MaxRetry;
};

class Controller_4G : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_4G(QObject *parent = nullptr);
    ~Controller_4G();

    void stopWorker();
    void start4GWorker();
    void restart4G();
public slots:

signals:
    void startWorker();
private:
    Worker_4G* worker = nullptr;
};
extern Controller_4G controller4G;

#endif // WORKER_4G_H
