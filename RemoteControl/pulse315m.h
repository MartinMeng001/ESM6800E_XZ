#ifndef PULSE315M_H
#define PULSE315M_H

#include <sys/time.h>
#include <QObject>
#include <QList>
#include <QMutex>
#include <QTime>
#include "AnnuniatorStatus/annuniatorstatus.h"

#define REMOTEGUARD_DIR_GO_EW_PROTOCOL5U    1
#define REMOTEGUARD_DIR_GO_SN_PROTOCOL5U    2
#define REMOTEGUARD_DIR_LEFT_EW_PROTOCOL5U  3
#define REMOTEGUARD_DIR_LEFT_SN_PROTOCOL5U  4
#define REMOTEGUARD_DIR_EALL_PROTOCOL5U     5
#define REMOTEGUARD_DIR_WALL_PROTOCOL5U     6
#define REMOTEGUARD_DIR_SALL_PROTOCOL5U     7
#define REMOTEGUARD_DIR_NALL_PROTOCOL5U     8

struct PulseItem{
    char onoff; // -1=invalid, 0=off, 1=on
    long holdontime;
};

class ValueItem{
public:
    ValueItem();
    ValueItem(char itemA, char itemB);
    void clear();
    void setValueA(char itemA);
    void setValueB(char itemB);
    char getDataA() const;
    void setDataA(char value);

    char getDataB() const;
    void setDataB(char value);

    char getResult();

protected:
    void convLogic();
private:
    char dataA, dataB;
    char result;
};

class RemoteValueLogic{
public:
    RemoteValueLogic();
    void reset(); // progress start
    void setValue(int index, char value);   // progress set data
    int getResultByMode(int mode);  // progress result
    int getResultByMode2(int mode);
    // initial setting
    void setAllowOrganizationId(char A, char B, char C);
    void setRecordOnly(char value);
    bool getRecordOnly() const;
    void setRecordOnly(bool value);
    int getResultID(char* buff, int length);
    bool checkIdValid();
protected:
    int logicActionMode6();
    int logicActionMode8();
    bool logicCheckIdMode6();
    bool logicCheckIdMode8();
    bool logicCheckDisableId();
    bool check4InvalidIdMode6();
    bool check4InvalidIdMode8();
    int execActionMode6();
    int execActionMode8();
    void justTest_execAction(int mode);
    char mode2direction(int mode);
private:
    ValueItem valueItems[12];
    char result, resultId[8],resultAction[6];
    char codeA, codeB, codeC;
    bool recordOnly;
};

class Pulse315M : public QObject
{
    Q_OBJECT
public:
    explicit Pulse315M(QObject *parent = nullptr);

    void setPulseItem(char onoff, struct timeval& currenttime);
    void setPulseItem2(char onoff, struct timeval& currenttime);
    void setPulseItem3(char onoff, struct timeval& currenttime);
    void checkDataOnce();
    void setAllowOrganizationId(char A, char B, char C);
    void setRecordOnly(char value);
signals:
    void signalHeader();
    void signalHigh();
    void signalLow();
    void signalInvalid();
    void signalGuardEWLeft(QByteArray data);
    void signalGuardEWGo(QByteArray data);
    void signalGuardSNLeft(QByteArray data);
    void signalGuardSNGo(QByteArray data);
    void signalGuardClear(QByteArray data);
    void signalGuard2Platform(RemoteControlDataItem item);

protected:
    bool beNewItem();
    bool checkHeader(struct PulseItem& currentitem);
    bool checkValueHigh(struct PulseItem& currentitem);
    bool checkValueLow(struct PulseItem& currentitem);
    void displayInvalid(struct PulseItem& currentitem);

    char checkOnOffLogic(PulseItem* last, PulseItem* cur);

    bool logicHeader(char status, long lastinterval, long curinterval);
    bool logicValueHigh(char status, long lastinterval, long curinterval);
    bool logicValueLow(char status, long lastinterval, long curinterval);

    char logicOnOff(char lastOnoff, char curOnoff);
    //long logicTimeus(long sec, long usec);
    long logicIntervalTimeus(long sec_last, long usec_last, long sec_cur, long usec_cur);
    struct PulseItem* makeItem(char onoff, struct timeval& currenttime);
    void setLastItem(PulseItem* item);
    void setLastTimevalItem(struct timeval& item);
    void readyNewItem();

    void checkPulseItemList();
    void config();
    void execAction(int mode);
    char mode2direction(int mode);
    void setRemoteControlData(char *id, char action, char direction, time_t curtime);

    void pulseBufferFromTemp();
    char idConv(char *id);
private:
    QMutex  mutex_dataitem;
    struct PulseItem  pulseLast;
    struct timeval lasttime;
    QList<PulseItem*> allPulseDataList;
    QList<PulseItem*> tempPulseDataList;
    RemoteValueLogic remoteLogic;
    bool recordOnly;
    QTime tempTime;
};

extern Pulse315M pulse315M;

#endif // PULSE315M_H
