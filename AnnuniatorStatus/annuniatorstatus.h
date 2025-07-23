#ifndef ANNUNIATORSTATUS_H
#define ANNUNIATORSTATUS_H

#include <QObject>
//#include "eventflagbuffer.h"
#include <QDateTime>
#include <QTimer>
#include "event_custom.hpp"
#include "annuniatortime.h"
#include "usage4gnetwork.h"
#include "status4gnetwork.h"
#include "Protocol/protocol808platform.h"

#define STATUS_POWERON  0
#define STATUS_POWEROFF 1

#define TROUBLETYPE_GREENCRASH 0x01
#define TROUBLETYPE_OUTPUT     0x02
#define TROUBLETYPE_PERSON     0x03
#define TROUBLETYPE_FRONTDOOR  0x05
#define TROUBLETYPE_BACKDOOR   0x06
#define TROUBLETYPE_POWER      0x07

#define TIMERVALUE_CHECKANNUNIATORTIME  100000
class TroubleStatus{
public:
    TroubleStatus();
    int setData(QByteArray data);
    int setPowerOn();
    int setPowerOff();
    void alarmTo808Data();
    void heartbeatTo808Data();
    void setCurrentTroubleNo(QByteArray troubleNo);
protected:
    char get808Type(Trouble808Item &data);
    void get808Time(Trouble808Item &data);
    char Type808GreenCrash(){return (char)1;}
    char Type808Output(){return (char)2;}
    char Type808Person(){return (char)3;}
    char Type808FrontDoor(){return (char)5;}
    char Type808BackDoor(){return (char)6;}
    char Type808Power(){return (char)7;}
    void Data808GreenCrash(Trouble808Item &data);
    void Data808Output(Trouble808Item &data);
    void Data808Person(Trouble808Item &data);
    void Data808FrontDoor(Trouble808Item &data);
    void Data808BackDoor(Trouble808Item &data);
    void Data808Power(Trouble808Item &data);
    QByteArray getCurrentBCDTime();
    char getBcdCode(int number);


    void Data808GreenCrash_DirectionChannel(Trouble808Item &data);
    void Data808Output_DirectionChannel(Trouble808Item &data);
    void Data808Person_DirectionChannel(Trouble808Item &data);
    //time_t troubletime2UTC(QByteArray troubletime);
    //QString troubleInfo2String(QByteArray troubleinfo);
    void resetParams();
    void setCurrenttimeTo808();
private:
    int troubleType;
    QByteArray troubleTime;
    QByteArray troubleInfo;
    QByteArray currntTroubleNo;
    bool fixed;
};
struct RemoteControlDataItem{
    char id;
    char idExt;
    time_t guardtime;
    char action;    // 0 or 1
    char direction; // 01~08
};
class GuardStatus{
public:
    GuardStatus();
    ~GuardStatus();

    QByteArray setCurrentGuard(RemoteControlDataItem &guardItem);
protected:
    QByteArray getCurrentGuardAsDataFmt();
    void setCurrentGuardData(RemoteControlDataItem &guardItem);
    void resetCurrentGuard();
    void setTime2AsFmt(QByteArray &data);
private:
    RemoteControlDataItem CurrentGuard;

};

class AnnuniatorStatus : public QObject
{
    Q_OBJECT
public:
    explicit AnnuniatorStatus(QObject *parent = nullptr);
    ~AnnuniatorStatus();
    void initAnnuniatorStatusParams();
    void tick();
signals:
    void sigPowerOn();
    void sigPowerOff();
    void sig4GPowerReset();
    void sig4GPowerReseted();
    void sigBatteryOn();
    void sigBatteryOff();
    void sigGetAnnuniatorTime(QByteArray data);
    void sigGetAnnuniatorTrouble(QByteArray data);
public slots:
    void Status_PowerOn();
    void Status_PowerOff();
    void Action_BatteryOn();
    void Action_BatteryOff();
    void Action_4GPowerReset();
    void Action_4GReseted();
    void Action_RebootDevice();
    void Action_808Heartbeat();
    void Action_GetAnnuniatorTime();
    void Action_GetAnnuniatorTrouble(QByteArray troubleNo);
    void Worker_GetAnnuniatorTime();
    void Worker_AnnuniatorStatusSlots();
    void Status_TroubleStatus(QByteArray data);
    void Status_GuardStatus(RemoteControlDataItem item);
    void Status_AnniniatorTime(QDateTime datetime);
    void Status_4GNetworkUsage(int type, int datasize);
    void Status_PPPIDUpdating(int status);
    void Status_4G808(int status);
    void Status_4GPlatform(int status);
protected:
    void PowerStatusLogic();
    void initTimer4GetAnnuniatorTimer();
    void updateTimer4GetAnnuniatorTimer();
    void clearTimer4GetAnnuniatorTimer();
    void Delay_MSec(int msec);
    void config();

private:
    Event powerStatusEvent;
    TroubleStatus troubleStatus;
    GuardStatus guardStatus;
    AnnuniatorTime AnnunitorTimeMgr;
    Usage4GNetwork usage4GNetwork;
    Status4GNetwork status4GNetwork;

    bool beRebooting;
    bool bePowerOn;
    bool disSyncTimeLoc;
    bool inProcessing_CloseBatteryPower;
    //bool allowCheckAnnuniatorTime, doNotThisTime;
    QTimer* timer4CheckAnnuniatorTime = nullptr;
    QDateTime lastUpdatingDatetime;
};
extern AnnuniatorStatus gAnnuniatorStatus;

#endif // ANNUNIATORSTATUS_H
