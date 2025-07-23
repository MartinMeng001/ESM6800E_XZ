#include <QDateTime>
#include <QTimer>
#include "Log/loggerworker.h"
#include "Configfile/userinfofile.h"
#include "Driver/yiyuan_ec20_4gadapter.h"
#include "Platform808/datamgr_to808platform.h"
#include "Platform4G/platformtcp.h"
#include "Platform4G/platformtcpv2.h"
#include "Annuniator/datamgr_fromannuniator.h"
#include "Protocol/protocolannuniator4u.h"
#include "Worker/worker_forgpio.h"
#include "Worker/worker_4g.h"
#include "annuniatorstatus.h"

AnnuniatorStatus gAnnuniatorStatus;
AnnuniatorStatus::AnnuniatorStatus(QObject *parent) : QObject(parent)
{
    //AnnunitorTimeMgr.setParent(parent);
    powerStatusEvent.setInitStatus(STATUS_POWEROFF);
    inProcessing_CloseBatteryPower = false;
    bePowerOn = true;
    disSyncTimeLoc = true;
    beRebooting = false;
    lastUpdatingDatetime = QDateTime::currentDateTime();
    //config();
}
AnnuniatorStatus::~AnnuniatorStatus(){
    clearTimer4GetAnnuniatorTimer();
}

void AnnuniatorStatus::initAnnuniatorStatusParams()
{
    if(gUserInfoFile.getDisableSyncTimeLoc()==1) disSyncTimeLoc=true;
    else disSyncTimeLoc=false;
    QString info = QString("AnnuniatorStatus Disabled SyncTime=%1").arg(disSyncTimeLoc);
    logworker.addLogger(info, LOGTYPE_PRINT);
}

void AnnuniatorStatus::tick()
{
    if(lastUpdatingDatetime.date().day()!=QDateTime::currentDateTime().date().day()){
        lastUpdatingDatetime = QDateTime::currentDateTime();
        gUserInfoFile.resetRebooting4GAll();
        logworker.addLogger("resetRebooting4GAll done ", LOGTYPE_PRINT);
        return;
    }
    if(beRebooting){
        status4GNetwork.tick();
        return;
    }
    if(status4GNetwork.Allow2RebootingFromPPPID()){
        //logworker.addLogger("PPPID REBOOT enter", LOGTYPE_PRINT_RECORD);
        if(gUserInfoFile.getRebooting4G()>0){
            beRebooting = true;
            //Action_RebootDevice();
            logworker.addLogger("PPPID REBOOT", LOGTYPE_PRINT_RECORD);
        }
    }else if(status4GNetwork.Allow2RebootinngFromPlatform4G()){
        if(gUserInfoFile.getRebootingPlatform()>0){
            beRebooting = true;
            Action_RebootDevice();
            logworker.addLogger("4GPLATFORM REBOOT", LOGTYPE_PRINT_RECORD);
        }
    }else if(status4GNetwork.Allow2RebootingFrom8084G()){
        if(gUserInfoFile.getRebooting808()>0){
            beRebooting = true;
            Action_RebootDevice();
            logworker.addLogger("808 PLATFORM REBOOT", LOGTYPE_PRINT_RECORD);
        }
    }
    status4GNetwork.tick();
}

void AnnuniatorStatus::Status_PowerOn()
{
    powerStatusEvent.setCurrentStatus(STATUS_POWERON);
    PowerStatusLogic();
}

void AnnuniatorStatus::Status_PowerOff()
{
    powerStatusEvent.setCurrentStatus(STATUS_POWEROFF);
    PowerStatusLogic();
}

void AnnuniatorStatus::Action_BatteryOn()
{
    emit sigBatteryOn();
}

void AnnuniatorStatus::Action_BatteryOff()
{
    inProcessing_CloseBatteryPower = false;
    //logworker.addLogger("Action_BatteryOff", LOGTYPE_PRINT);
    if(bePowerOn==false){
        //emit sigBatteryOff();
        controllerGPIO.actionBatteryOff();
        logworker.addLogger("SIGNAL_BatteryOff", LOGTYPE_PRINT);
    }
}

void AnnuniatorStatus::Action_4GPowerReset()
{
    logworker.addLogger("emit sig4GPowerReset();", LOGTYPE_PRINT);
    //controllerGPIO.actionReboot4GModule();
}

void AnnuniatorStatus::Action_4GReseted()
{
    //gYiYuanEC20_4GAdapter.Connect4G();
    logworker.addLogger("4G Power Reseted", LOGTYPE_PRINT);
    emit sig4GPowerReseted();
    //controller4G.restart4G();
    //Action_RebootDevice();
}

void AnnuniatorStatus::Action_RebootDevice()
{
    controllerGPIO.actionRebootDev();
}

void AnnuniatorStatus::Action_808Heartbeat()
{
    troubleStatus.heartbeatTo808Data();
    usage4GNetwork.print();
}

void AnnuniatorStatus::Action_GetAnnuniatorTime()
{
    QByteArray ret = gProtocolAnnuniator4U.getAnnuniatorTime();
    emit sigGetAnnuniatorTime(ret);
    logworker.addLogger("Try to Get AnnuniatorTime", LOGTYPE_PRINT);
}

void AnnuniatorStatus::Action_GetAnnuniatorTrouble(QByteArray troubleNo)
{
    QByteArray ret = gProtocolAnnuniator4U.getTroubleA1ByTroubleNo(troubleNo);
    if(ret.size()==0) return;
    troubleStatus.setCurrentTroubleNo(troubleNo);
    emit sigGetAnnuniatorTrouble(ret);
    //logworker.addLogger("Try to Get AnnuniatorTrouble", LOGTYPE_PRINT);
}

void AnnuniatorStatus::Worker_GetAnnuniatorTime()
{
    if(disSyncTimeLoc) return;
    initTimer4GetAnnuniatorTimer();
    logworker.addLogger("Worker_GetAnnuniatorTime begin", LOGTYPE_PRINT);
}

void AnnuniatorStatus::Worker_AnnuniatorStatusSlots()
{
    config();
    logworker.addLogger("Worker_AnnuniatorStatusSlots begin", LOGTYPE_PRINT);
}

void AnnuniatorStatus::Status_TroubleStatus(QByteArray data)
{
//    QString a = data.toHex().toUpper();
//    QString infoA = QString("Annuniator Trouble Data(Rec):%1").arg(a);
//    logworker.addLogger(infoA, LOGTYPE_PRINT);
    if(troubleStatus.setData(data)==0)return;
//    QString infoB = QString("Annuniator Trouble Data Parsed");
//    logworker.addLogger(infoB, LOGTYPE_PRINT);
    //logworker.addLogger("Annuniator Trouble Event", LOGTYPE_PRINT);
    troubleStatus.alarmTo808Data();
}

void AnnuniatorStatus::Status_GuardStatus(RemoteControlDataItem item)
{
    QByteArray data2send = guardStatus.setCurrentGuard(item);
    dataItem_Annuniator *item2platform = new dataItem_Annuniator();
    logworker.addLogger("AnnuniatorStatus::Status_GuardStatus new dataItem_Annuniator", LOGTYPE_PRINT_RECORD);
    item2platform->dataFromAnnuniator = data2send;
    item2platform->datatime = QTime::currentTime();
    dataMgrFromAnnuniator.addAnnuniatorDataItem(item2platform);
}

void AnnuniatorStatus::Status_AnniniatorTime(QDateTime datetime)
{
    if(disSyncTimeLoc) return;
    AnnunitorTimeMgr.setCurrentAnnuniatorTime(datetime);
    updateTimer4GetAnnuniatorTimer();
}

void AnnuniatorStatus::Status_4GNetworkUsage(int type, int datasize)
{
    usage4GNetwork.set4GUsageData(type, datasize);
}

void AnnuniatorStatus::Status_PPPIDUpdating(int status)
{
    if(status==1){
        gUserInfoFile.resetRebooting4G();
    }
    status4GNetwork.setPPPidStatus(status);
}

void AnnuniatorStatus::Status_4G808(int status)
{
    if(status==1){
        gUserInfoFile.resetRebooting808();
    }
    status4GNetwork.set808Status(status);
}

void AnnuniatorStatus::Status_4GPlatform(int status)
{
    if(status==1){
        gUserInfoFile.resetRebootingPlatform();
    }
    status4GNetwork.setPlatformStatus(status);
}

void AnnuniatorStatus::PowerStatusLogic()
{
    int offevent = powerStatusEvent.getEvent();
    if(offevent==TRIGGERSTATUS_CHANGE2ON){
        logworker.addLogger("Power OFF Event", LOGTYPE_PRINT);
        troubleStatus.setPowerOff();
        troubleStatus.alarmTo808Data();
        bePowerOn = false;
        Delay_MSec(60000);
        emit sigPowerOff();
    }else if(offevent==TRIGGERSTATUS_CHANGE2OFF){
        logworker.addLogger("Power ON Event", LOGTYPE_PRINT);
        troubleStatus.setPowerOn();
        troubleStatus.alarmTo808Data();
        bePowerOn = true;
        emit sigPowerOn();
    }

}

void AnnuniatorStatus::initTimer4GetAnnuniatorTimer()
{
    if(timer4CheckAnnuniatorTime==nullptr){
        timer4CheckAnnuniatorTime = new QTimer(this);
        logworker.addLogger("AnnuniatorStatus::timer4CheckAnnuniatorTime new QTimer", LOGTYPE_PRINT_RECORD);
        connect(timer4CheckAnnuniatorTime, &QTimer::timeout, this, &AnnuniatorStatus::Action_GetAnnuniatorTime);
        timer4CheckAnnuniatorTime->start(TIMERVALUE_CHECKANNUNIATORTIME);
    }
}

void AnnuniatorStatus::updateTimer4GetAnnuniatorTimer()
{
    if(timer4CheckAnnuniatorTime==nullptr) return;
    timer4CheckAnnuniatorTime->start();
}

void AnnuniatorStatus::clearTimer4GetAnnuniatorTimer()
{
    if(timer4CheckAnnuniatorTime==nullptr) return;
    timer4CheckAnnuniatorTime->stop();
    logworker.addLogger("AnnuniatorStatus::timer4CheckAnnuniatorTime delete QTimer", LOGTYPE_PRINT_RECORD);
    delete timer4CheckAnnuniatorTime;
}

void AnnuniatorStatus::Delay_MSec(int msec)
{
    if(inProcessing_CloseBatteryPower) return;
    inProcessing_CloseBatteryPower = true;
    //QEventLoop loop;
    QTimer::singleShot(msec, this, SLOT(Action_BatteryOff()));
    //loop.exec();
}

void AnnuniatorStatus::config()
{
    connect(this, &AnnuniatorStatus::sigGetAnnuniatorTime, &gPlatformTcpv2, &PlatformTcp::processExtraData4Annuniator);
    connect(this, &AnnuniatorStatus::sigGetAnnuniatorTrouble, &gPlatformTcpv2, &PlatformTcp::processExtraData4Annuniator);
    connect(&(this->AnnunitorTimeMgr), &AnnuniatorTime::updateAnnuniatorTime, &gPlatformTcpv2, &PlatformTcp::processExtraData4Annuniator);
}

TroubleStatus::TroubleStatus()
{
    resetParams();
}
// troubletype(1)+time(6)+troubleinfo(2)+troublefixtime(6)
// 0 - (1,2,3,4,5,6) - (7,8) - (9,10,11,12,13,14)
int TroubleStatus::setData(QByteArray data)
{
    if(data.size()<15) return 0;
    resetParams();
    troubleType = (int)(data[0]&0xFF);
    if(troubleType==0x07) {
        troubleType = 0;
        return 0;
    }
    troubleInfo = data.mid(7, 2);
    if(data.data()[9]!=(char)0xFF){
        fixed=true;
        troubleTime = data.mid(9, 6);
    }else{
        fixed = false;
        troubleTime = data.mid(1, 6);
    }
    if(currntTroubleNo.size()==0) return 0;
    currntTroubleNo.clear();
    return 1;
}

int TroubleStatus::setPowerOn()
{
    resetParams();
    troubleType = TROUBLETYPE_POWER;
    QByteArray troubleTime;
    fixed = true;
    setCurrenttimeTo808();
    return 1;
}

int TroubleStatus::setPowerOff()
{
    resetParams();
    troubleType = TROUBLETYPE_POWER;
    QByteArray troubleTime;
    fixed = false;
    setCurrenttimeTo808();
    return 1;
}

void TroubleStatus::alarmTo808Data()
{
    Trouble808Item item;
    int ret = get808Type(item);
    if(ret==0) return;
    QByteArray data = gProtocol808Platform.getCmdFF00(item);
    if(data.size()==0) return;
    dataItem_808Platform *dataitem=new dataItem_808Platform;
    logworker.addLogger("TroubleStatus alarmTo808Data new dataItem_808Platform", LOGTYPE_PRINT_RECORD);
    dataitem->dstIPAddress = "192.168.0.99";
    dataitem->dataFromPlatform = data;
    dataitem->datatime=QTime::currentTime();
    dataMgr_To808Platform.addPlatformDataItem(dataitem);
//    dataMgr_To808Platform.addPlatformDataItem(dataitem);
//    dataMgr_To808Platform.addPlatformDataItem(dataitem);
    QString a = data.toHex().toUpper();
    QString info = QString("TroubleEvent: %1").arg(a);
    logworker.addLogger(info, LOGTYPE_PRINT_RECORD);
}

void TroubleStatus::heartbeatTo808Data()
{
    QByteArray data = gProtocol808Platform.getCmd0002();
    if(data.size()==0) return;
    dataItem_808Platform *dataitem=new dataItem_808Platform;
    logworker.addLogger("TroubleStatus heartbeatTo808Data new dataItem_808Platform", LOGTYPE_PRINT_RECORD);
    dataitem->dstIPAddress = "192.168.0.99";
    dataitem->dataFromPlatform = data;
    dataitem->datatime=QTime::currentTime();
    dataMgr_To808Platform.addPlatformDataItem(dataitem);
}

void TroubleStatus::setCurrentTroubleNo(QByteArray troubleNo)
{
    currntTroubleNo.clear();
    currntTroubleNo = troubleNo;
}

char TroubleStatus::get808Type(Trouble808Item &data)
{
    char ret = 0;
    switch (troubleType) {
    case TROUBLETYPE_GREENCRASH:
        Data808GreenCrash(data);
        data.alarmtype = Type808GreenCrash();
        logworker.addLogger("TROUBLETYPE_GREENCRASH", LOGTYPE_PRINT);
        ret = 1;
        break;
    case TROUBLETYPE_OUTPUT:
        Data808Output(data);
        data.alarmtype = Type808Output();
        logworker.addLogger("TROUBLETYPE_OUTPUT", LOGTYPE_PRINT);
        ret = 1;
        break;
    case TROUBLETYPE_PERSON:
        Data808Person(data);
        data.alarmtype = Type808Person();
        ret = 1;
        break;
    case TROUBLETYPE_FRONTDOOR:
        Data808FrontDoor(data);
        data.alarmtype = Type808FrontDoor();
        logworker.addLogger("TROUBLETYPE_FRONTDOOR", LOGTYPE_PRINT);
        ret = 1;
        break;
    case TROUBLETYPE_BACKDOOR:
        Data808BackDoor(data);
        data.alarmtype = Type808BackDoor();
        logworker.addLogger("TROUBLETYPE_BACKDOOR", LOGTYPE_PRINT);
        ret = 1;
        break;
    case TROUBLETYPE_POWER:
        Data808Power(data);
        data.alarmtype = Type808Power();
        ret = 1;
        break;
    default:
        break;
    }
    if(ret==1)get808Time(data);
    return ret;
}

void TroubleStatus::get808Time(Trouble808Item &data)
{
    QByteArray timeitem = getCurrentBCDTime();
    if(timeitem.size()<6) return;
    for(int i=0;i<troubleTime.size();i++){
        if(i>=6) break;
        data.bcdtime[i] = timeitem.data()[i];//troubleTime.data()[i];
    }
}

void TroubleStatus::Data808GreenCrash(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    data.alarmcolor = 0;
    Data808GreenCrash_DirectionChannel(data);
}

void TroubleStatus::Data808Output(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    Data808Output_DirectionChannel(data);
}

void TroubleStatus::Data808Person(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    Data808Person_DirectionChannel(data);
}

void TroubleStatus::Data808FrontDoor(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    data.alarmdirA = 0;
    data.alarmdirB = 0;
    data.alarmchanA = 0;
    data.alarmchanB = 0;
    data.alarmcolor = 0;
}

void TroubleStatus::Data808BackDoor(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    data.alarmdirA = 0;
    data.alarmdirB = 0;
    data.alarmchanA = 0;
    data.alarmchanB = 0;
    data.alarmcolor = 0;
}

void TroubleStatus::Data808Power(Trouble808Item &data)
{
    if(fixed) data.alarmstatus = (char)0x01; else data.alarmstatus = (char)0x02;
    data.alarmdirA = 0;
    data.alarmdirB = 0;
    data.alarmchanA = 0;
    data.alarmchanB = 0;
    data.alarmcolor = 0;
}

QByteArray TroubleStatus::getCurrentBCDTime()
{
    QDateTime now = QDateTime::currentDateTime();
    QByteArray ret;
    ret.append(getBcdCode(now.date().year()%100));
    ret.append(getBcdCode(now.date().month()));
    ret.append(getBcdCode(now.date().day()));
    ret.append(getBcdCode(now.time().hour()));
    ret.append(getBcdCode(now.time().minute()));
    ret.append(getBcdCode(now.time().second()));
    return ret;
}

char TroubleStatus::getBcdCode(int number)
{
    char ret = 0;
    if(number>100 || number<=0) return ret;
    char high = number/10;
    char low = number%10;
    ret = (char)((high << 4)&0xF0|(low&0x0F));
    return ret;
}


void TroubleStatus::Data808GreenCrash_DirectionChannel(Trouble808Item &data)
{
    char statusA = 0;
    char alarmdirA= 0, alarmChanA=0;	// east
    char alarmdirB= 0, alarmChanB=0;	// west
    statusA = troubleInfo.data()[0];
    //statusB = troubleInfo.data()[1];
    switch ((char)(statusA >> 6))
    {
    case 0:
        alarmdirA= 1;	// east
        alarmdirB= 2;	// west
        break;
    case 1:
        alarmdirA= 3;	// south
        alarmdirB= 4;	// north
        break;
    case 2:
        alarmdirA= 10;	// east west
        alarmdirB= 11;	// south north
        break;
    default:
        break;
    }
    alarmChanA=0;
    switch ((char)(statusA & 0x07))
    {
    case 1:
        alarmChanA|= 0x1;	//left
        break;
    case 2:
        alarmChanA|= 0x2; 	//go
        break;
    case 3:
        alarmChanA|= 0x4; 	//righg
        break;
    case 4:
        alarmChanA|= 0x8; 	// bicycle
        break;
    case 5:
        alarmChanA|= 0x10; 	// resered
        break;
    case 6:
        alarmChanA|= 0x20; 	// person1
        break;
    case 7:
        alarmChanA|= 0x40; 	// person2
        break;
    default:
        break;
    }
    alarmChanB=0;
    switch ((char)((statusA>>3) & 0x07))
    {
    case 1:
        alarmChanB|= 0x1;	//left
        break;
    case 2:
        alarmChanB|= 0x2;	//go
        break;
    case 3:
        alarmChanB|= 0x4;	//righg
        break;
    case 4:
        alarmChanB|= 0x8;	// bicycle
        break;
    case 5:
        alarmChanB|= 0x10;	// resered
        break;
    case 6:
        alarmChanB|= 0x20;	// person1
        break;
    case 7:
        alarmChanB|= 0x40;	// person2
        break;
    default:
        break;
    }
    data.alarmdirA = alarmdirA;
    data.alarmdirB = alarmdirB;
    data.alarmchanA = alarmChanA;
    data.alarmchanB = alarmChanB;
}

void TroubleStatus::Data808Output_DirectionChannel(Trouble808Item &data)
{
    char statusA = 0, statusB = 0;
    char alarmdirA= 0, alarmChanA=0;	// east
    char alarmcolor= 0, alarmstatus=0;	// west
    statusA = troubleInfo.data()[0];
    statusB = troubleInfo.data()[1];
    switch ((char)(((unsigned char)statusA / 7) / 3))
    {
    case 0:alarmdirA=1;break;//east
    case 1:alarmdirA=2;break;//west
    case 2:alarmdirA=3;break;//south
    case 3:alarmdirA=4;break;//north
    default:
        break;
    }
    alarmChanA=0;
    //printf("output statusA=%02X, conv=%d\r\n", statusA, ((unsigned char)statusA % 7));
    switch ((char)((unsigned char)statusA % 7))
    {
    case 0:alarmChanA|=0x1;break;//left
    case 1:alarmChanA|=0x2;break;//go
    case 2:alarmChanA|=0x4;break;//right
    case 3:alarmChanA|=0x8;break;//bicycle
    case 4:alarmChanA|=0x10;break;//reserved
    case 5:alarmChanA|=0x20;break;//person1
    case 6:alarmChanA|=0x40;break;//person2
    default:
        break;
    }
    //printf("alarmchanA=%d\r\n", alarmChanA);
    alarmcolor=0;
    switch ((char)(((unsigned char)statusA / 7) % 3))
    {
    case 0:
        alarmcolor|=0x4;
        logworker.addLogger("TROUBLE RED", LOGTYPE_PRINT);
        break;// red
    case 1:
        alarmcolor|=0x2;
        logworker.addLogger("TROUBLE YELLOW", LOGTYPE_PRINT);
        break;// yellow
    case 2:
        alarmcolor|=0x1;
        logworker.addLogger("TROUBLE GREEN", LOGTYPE_PRINT);
        break;// green
    default: break;
    }
    alarmstatus=0;
    switch ((char)statusB)
    {
    case 1:alarmstatus|=0x4;break;// breakoff
    case 2:alarmstatus|=0x8;break;// red&green
    case 3:alarmstatus|=0x10;break;// external green crash
    default: break;
    }
    data.alarmdirA = alarmdirA;
    data.alarmdirB = 0;
    data.alarmchanA = alarmChanA;
    data.alarmchanB = 0;
    data.alarmcolor = alarmcolor;
}

void TroubleStatus::Data808Person_DirectionChannel(Trouble808Item &data)
{
    char statusA = 0;
    char alarmdirA= 0, alarmChanA=0;	// east
    char alarmcolor= 0;	// west
    statusA = troubleInfo.data()[0];
    //statusB = troubleInfo.data()[1];

    switch(statusA)
    {
    case 1: alarmdirA=1; break;//east
    case 2: alarmdirA=2; break;//west
    case 3: alarmdirA=3; break;//south
    case 4: alarmdirA=4; break;//north
    default: break;
    }

    data.alarmdirA = alarmdirA;
    data.alarmdirB = 0;
    data.alarmchanA = alarmChanA;
    data.alarmchanB = 0;
    data.alarmcolor = alarmcolor;
}

//time_t TroubleStatus::troubletime2UTC(QByteArray troubletime)
//{

//}

//QString TroubleStatus::troubleInfo2String(QByteArray troubleinfo)
//{

//}


void TroubleStatus::resetParams()
{
    troubleType=0;
    troubleTime.clear();
    troubleInfo.clear();
    fixed = false;
}

void TroubleStatus::setCurrenttimeTo808()
{
    QDateTime now = QDateTime::currentDateTime();
    troubleTime.append((char)(now.date().year()-2000));
    troubleTime.append((char)(now.date().month()));
    troubleTime.append((char)(now.date().day()));
    troubleTime.append((char)(now.time().hour()));
    troubleTime.append((char)(now.time().minute()));
    troubleTime.append((char)(now.time().second()));
}

GuardStatus::GuardStatus()
{
    resetCurrentGuard();
}

GuardStatus::~GuardStatus()
{
    logworker.addLogger("Annuniator Status Closed", LOGTYPE_PRINT);
}

QByteArray GuardStatus::setCurrentGuard(RemoteControlDataItem &guardItem)
{
    setCurrentGuardData(guardItem);
    return getCurrentGuardAsDataFmt();
}

QByteArray GuardStatus::getCurrentGuardAsDataFmt()
{
    QByteArray ret;
    ret.append((char)0x06);
    ret.append((char)0x47);
    setTime2AsFmt(ret);
    ret.append(CurrentGuard.id);
    ret.append(CurrentGuard.action);
    ret.append(CurrentGuard.direction);
    ret.append(CurrentGuard.idExt);
    return ret;
}

void GuardStatus::setCurrentGuardData(RemoteControlDataItem &guardItem)
{
    CurrentGuard.id = guardItem.id;
    CurrentGuard.idExt = guardItem.idExt;
    CurrentGuard.action = guardItem.action;
    CurrentGuard.direction = guardItem.direction;
    CurrentGuard.guardtime = guardItem.guardtime;
}

void GuardStatus::resetCurrentGuard()
{
    CurrentGuard.id = 0;
    CurrentGuard.idExt = 0;
    CurrentGuard.action = 0;
    CurrentGuard.direction = 0;
    CurrentGuard.guardtime = 0;
}

void GuardStatus::setTime2AsFmt(QByteArray &data)
{
    time_t tt = CurrentGuard.guardtime;
    data.append((char)(tt&0x000000FF));
    data.append((char)((tt>>8)&0x000000FF));
    data.append((char)((tt>>16)&0x000000FF));
    data.append((char)((tt>>24)&0x000000FF));
}
