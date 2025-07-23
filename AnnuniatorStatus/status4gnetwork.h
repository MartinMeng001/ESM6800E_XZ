#ifndef STATUS4GNETWORK_H
#define STATUS4GNETWORK_H

#include <QObject>
#include <QTime>

class TimerStatus{
public:
    TimerStatus();
    void init(int status, int maxtime);
    void setStatus(int status);
    void resetOverTime();
    bool beOverTime();
    QString logStatus();
private:
    int initStatus;
    int currentStatus; // -1 = invalid, other = valid status
    QTime timeCount;
    int overTime;   // unit second
};


class Status4GNetwork : public QObject
{
    Q_OBJECT
public:
    explicit Status4GNetwork(QObject *parent = nullptr);

    void setPPPidStatus(int status);
    void set808Status(int status);
    void setPlatformStatus(int status);
    bool Allow2RebootingFromPPPID();
    bool Allow2RebootingFrom8084G();
    bool Allow2RebootinngFromPlatform4G();
    void tick();
signals:

protected:
    void initStatusItem();
    void openSubStatus(int status);
    void logCurrentStatus();
private:
    bool allow4GStatus;
    TimerStatus statusPPPid;    // 10 minutes, 0 was wrong
    TimerStatus status808_4G;   // 10 minutes, 0 was wrong
    TimerStatus statusPlatform_4G;  // 10 minutes, 0 was wrong
};

#endif // STATUS4GNETWORK_H
