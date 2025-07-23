#include <iostream>
#include <QTime>
#include <time.h>
#include <QMutexLocker>
#include "Configfile/userinfofile.h"
#include "Protocol/protocolannuniator4u.h"
//#include "Platform4G/platformtcp.h"
#include "Platform4G/platformtcpv2.h"
#include "Log/loggerworker.h"
#include "regionmanager.h"
#include "pulse315m.h"
Pulse315M pulse315M;
Pulse315M::Pulse315M(QObject *parent) : QObject(parent)
{
    pulseLast.onoff = -1;
    pulseLast.holdontime = 0;
    lasttime.tv_sec = 0;
    lasttime.tv_usec = 0;
    allPulseDataList.clear();
    tempPulseDataList.clear();
    recordOnly = false;
    //remoteLogic.setAllowOrganizationId('F','F','0');
    config();
}

void Pulse315M::setPulseItem(char onoff, timeval &currenttime)
{
    QMutexLocker locker(&mutex_dataitem);
    struct PulseItem* item = makeItem(onoff, currenttime);
    if(item==nullptr) return;
    if(beNewItem()){
    }else if(checkHeader(*item)){
        readyNewItem();
        emit signalHeader();
    }else if(checkValueHigh(*item)){
        readyNewItem();
        emit signalHigh();
    }else if(checkValueLow(*item)){
        readyNewItem();
        emit signalLow();
    }else{
        //displayInvalid(*item);
        emit signalInvalid();
    }
    setLastTimevalItem(currenttime);
    setLastItem(item);
}

void Pulse315M::setPulseItem2(char onoff, timeval &currenttime)
{
    QMutexLocker locker(&mutex_dataitem);
    struct PulseItem* item = new PulseItem();
    item->onoff=(onoff==1?0:1);
    item->holdontime = logicIntervalTimeus(lasttime.tv_sec, lasttime.tv_usec, currenttime.tv_sec, currenttime.tv_usec);
    allPulseDataList.append(item);
    checkPulseItemList();
    setLastTimevalItem(currenttime);
    setLastItem(item);
}

void Pulse315M::setPulseItem3(char onoff, timeval &currenttime)
{
    QMutexLocker locker(&mutex_dataitem);
    struct PulseItem* item = new PulseItem();
    item->onoff=(onoff==1?0:1);
    item->holdontime = logicIntervalTimeus(lasttime.tv_sec, lasttime.tv_usec, currenttime.tv_sec, currenttime.tv_usec);
    setLastTimevalItem(currenttime);
    setLastItem(item);
    if(tempPulseDataList.size()>1500) return;
    tempPulseDataList.append(item);
    //checkPulseItemList();
}

void Pulse315M::checkDataOnce()
{
    pulseBufferFromTemp();
    //QMutexLocker locker(&mutex_dataitem);
    //tempTime.restart();
    //int tempSize = allPulseDataList.size();
    while(allPulseDataList.size()>=50){
        bool dataready=true;
        PulseItem* last = allPulseDataList.at(0);
        PulseItem* cur = allPulseDataList.at(1);
        if(logicHeader(last->onoff, last->holdontime, cur->holdontime)){
            remoteLogic.reset();
            PulseItem* items[48];
            for(int i=0;i<48;i++){
                items[i] = allPulseDataList.at(2+i);
            }
            QByteArray info;
            for(int i=0;i<24;i++){
                char val = checkOnOffLogic(items[2*i], items[2*i+1]);
                if(val == 0){
                    dataready=false;
                    break;
                }
                remoteLogic.setValue(i, val);
                info.append(val);
            }
            if(dataready){
                int mode = remoteLogic.getResultByMode2(8);
                if(remoteLogic.checkIdValid())
                    execAction(mode);
                else{
                    logworker.addLogger("Invalid GuardId, don't control", LOGTYPE_PRINT);
                }
            }
            delete last;
            delete cur;
            allPulseDataList.removeFirst();
            allPulseDataList.removeFirst();
        }else{
            PulseItem* last = allPulseDataList.at(0);
            delete last;
            allPulseDataList.removeFirst();
        }
    }
//    if(tempSize>=50){
//        QString info=QString("cousumed=%1, totalsize=%2").arg(tempTime.elapsed()).arg(tempSize);
//        std::cout << info.toStdString() << std::endl;
//    }
    //logworker.addLogger(info, LOGTYPE_RECORD);
}

void Pulse315M::setAllowOrganizationId(char A, char B, char C)
{
//    QString invalidinfo = "Invalid setAllowOrganizationId guardIds="+gUserInfoFile.getDisableGuardList().at(0);
//    logworker.addLogger(invalidinfo, LOGTYPE_PRINT);
    regionManager.setAllRegions(gUserInfoFile.getOrganizations());
    regionManager.setAllDisabledIds(gUserInfoFile.getDisableGuardList());
    if(A=='F' && B=='F' && C=='F'){
        std::cout << "Allow region id is FFF" << std::endl;
        remoteLogic.setAllowOrganizationId('F', 'F', 'F');
    }else{
        remoteLogic.setAllowOrganizationId(A, B, C);
    }
}

void Pulse315M::setRecordOnly(char value)
{
    if(value=='1')recordOnly = true;
    else recordOnly = false;
    //recordOnly =
    //remoteLogic.setRecordOnly(value);
}

bool Pulse315M::beNewItem()
{
    if(pulseLast.onoff == (char)-1){
        return true;
    }
    return false;
}

bool Pulse315M::checkHeader(PulseItem &currentitem)
{
    char onoff = logicOnOff(pulseLast.onoff, currentitem.onoff);
    if(onoff == (char)(-1)) return false;
    return logicHeader(pulseLast.onoff, pulseLast.holdontime, currentitem.holdontime);
}

bool Pulse315M::checkValueHigh(PulseItem &currentitem)
{
    char onoff = logicOnOff(pulseLast.onoff, currentitem.onoff);
    if(onoff == (char)(-1)) return false;
    return logicValueHigh(pulseLast.onoff, pulseLast.holdontime, currentitem.holdontime);
}

bool Pulse315M::checkValueLow(PulseItem &currentitem)
{
    char onoff = logicOnOff(pulseLast.onoff, currentitem.onoff);
    if(onoff == (char)(-1)) return false;
    return logicValueLow(pulseLast.onoff, pulseLast.holdontime, currentitem.holdontime);
}

void Pulse315M::displayInvalid(PulseItem &currentitem)
{
    char onoff = logicOnOff(pulseLast.onoff, currentitem.onoff);
    if(onoff == (char)(-1)) return;
}

char Pulse315M::checkOnOffLogic(PulseItem *last, PulseItem *cur)
{
    if(logicValueHigh(last->onoff, last->holdontime, cur->holdontime))return 'H';
    if(logicValueLow(last->onoff, last->holdontime, cur->holdontime))return 'L';
    return 0;
}

bool Pulse315M::logicHeader(char status, long lastinterval, long curinterval)
{
    if(status==0) {
        return false;
    }
    if(lastinterval==0) {
        return false;
    }
    long count = curinterval/lastinterval;
    if(count>=31 && count<40){
        if(lastinterval>400 && lastinterval<500) {
            return true;
        }
    }
    return false;
}

bool Pulse315M::logicValueHigh(char status, long lastinterval, long curinterval)
{
    if(status==0) return false;
    if(lastinterval==0) return false;
    long count = lastinterval/curinterval;

    if(count>=2 && count<6){    // should determine here
        return true;
    }
    return false;
}

bool Pulse315M::logicValueLow(char status, long lastinterval, long curinterval)
{
    if(status==0) return false;
    if(lastinterval==0) return false;
    long count = curinterval/lastinterval;
    if(count>=2 && count<6){
        return true;
    }
    return false;
}



char Pulse315M::logicOnOff(char lastOnoff, char curOnoff)
{
    char ret = -1;
    if(lastOnoff == curOnoff) return ret;
    return lastOnoff;
}

//long Pulse315M::logicTimeus(long sec, long usec)
//{
//    return 0;
//}

long Pulse315M::logicIntervalTimeus(long sec_last, long usec_last, long sec_cur, long usec_cur)
{
    if(sec_last>sec_cur) return 0;
    if(sec_last==0 && usec_last==0){    // the first value is invalid
        return 1;
    }
    long sec = sec_cur - sec_last;
    long interval = 0;
    if(sec == 0){
        interval = usec_cur - usec_last;
    }else{
        interval = sec * 1000*1000 + usec_cur - usec_last;
    }
    if(interval<0) return 0;
    return interval;
}

PulseItem *Pulse315M::makeItem(char onoff, timeval &currenttime)
{
    static PulseItem item;
    item.onoff = onoff==1?0:1;
    item.holdontime = logicIntervalTimeus(lasttime.tv_sec, lasttime.tv_usec, currenttime.tv_sec, currenttime.tv_usec);
    return (PulseItem *)(&item);
}

void Pulse315M::setLastItem(PulseItem *item)
{
    // should debug here
    pulseLast.onoff = item->onoff;
    pulseLast.holdontime = item->holdontime;
}

void Pulse315M::setLastTimevalItem(timeval &item)
{
    lasttime.tv_sec=item.tv_sec;
    lasttime.tv_usec=item.tv_usec;
}

void Pulse315M::readyNewItem()
{
    pulseLast.onoff = -1;
    pulseLast.holdontime = 0;
}

void Pulse315M::checkPulseItemList()
{
    if(allPulseDataList.size()>200){
        allPulseDataList.removeFirst();
    }
}

void Pulse315M::config()
{
    if(recordOnly==false){
        connect(this, &Pulse315M::signalGuardEWGo, &gPlatformTcpv2, &PlatformTcpV2::processExtraData4Annuniator);
        connect(this, &Pulse315M::signalGuardEWLeft, &gPlatformTcpv2, &PlatformTcpV2::processExtraData4Annuniator);
        connect(this, &Pulse315M::signalGuardSNGo, &gPlatformTcpv2, &PlatformTcpV2::processExtraData4Annuniator);
        connect(this, &Pulse315M::signalGuardSNLeft, &gPlatformTcpv2, &PlatformTcpV2::processExtraData4Annuniator);
        connect(this, &Pulse315M::signalGuardClear, &gPlatformTcpv2, &PlatformTcpV2::processExtraData4Annuniator);
        connect(this, &Pulse315M::signalGuard2Platform, &gAnnuniatorStatus, &AnnuniatorStatus::Status_GuardStatus);
    }
}

void Pulse315M::execAction(int mode)
{
    bool isGuard=true;

    switch(mode){
    case 1:{
        QByteArray ret = gProtocolAnnuniator4U.getRemoteGuardEastWestLeft();
        emit signalGuardEWLeft(ret);
        logworker.addLogger("guard left: east and west", LOGTYPE_PRINT);
        break;
    }
    case 2:{
        QByteArray ret = gProtocolAnnuniator4U.getRemoteGuardSouthNorthLeft();
        emit signalGuardSNLeft(ret);
        logworker.addLogger("guard left: south and north", LOGTYPE_PRINT);
        break;
    }
    case 3:{
        QByteArray ret = gProtocolAnnuniator4U.getRemoteGuardEastWestGo();
        emit signalGuardEWGo(ret);
        logworker.addLogger("guard go: east and west", LOGTYPE_PRINT);
        break;
    }
    case 4:{
        QByteArray ret = gProtocolAnnuniator4U.getRemoteGuardSouthNorthGo();
        emit signalGuardSNGo(ret);
        logworker.addLogger("guard go: south and north", LOGTYPE_PRINT);
        break;
    }
    case 5:{
        QByteArray ret = gProtocolAnnuniator4U.setCancleGuardStatus();
        emit signalGuardClear(ret);
        logworker.addLogger("clear guard 5", LOGTYPE_PRINT);
        break;
    }
    case 6:{
        QByteArray ret = gProtocolAnnuniator4U.setCancleGuardStatus();
        emit signalGuardClear(ret);
        logworker.addLogger("clear guard 6", LOGTYPE_PRINT);
        break;
    }
    default:
        logworker.addLogger("Invalid guard data, don't guard", LOGTYPE_PRINT);
        isGuard=false;
        return;
    }
    if(isGuard){
        time_t curtime = time(nullptr);
        char resultId[8];
        memset(resultId, 0, sizeof(resultId));
        if(remoteLogic.getResultID(resultId, sizeof(resultId))==1){
            if(mode == 6 || mode == 5){
                setRemoteControlData(resultId, 0, 0, curtime);
            }else{
                setRemoteControlData(resultId, 1, mode2direction(mode), curtime);
            }
        }
    }
}

char Pulse315M::mode2direction(int mode)
{
    switch (mode) {
    case 1: return REMOTEGUARD_DIR_LEFT_EW_PROTOCOL5U;
    case 2: return REMOTEGUARD_DIR_LEFT_SN_PROTOCOL5U;
    case 3: return REMOTEGUARD_DIR_GO_EW_PROTOCOL5U;
    case 4: return REMOTEGUARD_DIR_GO_SN_PROTOCOL5U;
    default:
        break;
    }
    return 0;
}
char Pulse315M::idConv(char *id)
{
    char data[4];
    for(int i=0;i<4;i++){
        if(id[i]=='F')data[i]=0;
        else if(id[i]=='0')data[i]=1;
        else if(id[i]=='1')data[i]=2;
        else data[i]=0;
    }
    int ret = data[0]+data[1]*3+data[2]*9+data[3]*27;
    return (char)ret;
}
void Pulse315M::setRemoteControlData(char *id, char action, char direction, time_t curtime)
{
    RemoteControlDataItem guardItem;
    guardItem.id = idConv(id);
    guardItem.idExt = idConv(&id[4]);
    guardItem.action = action;
    guardItem.direction = direction;
    guardItem.guardtime = curtime;
    emit signalGuard2Platform(guardItem);
}

void Pulse315M::pulseBufferFromTemp()
{
    QMutexLocker locker(&mutex_dataitem);
    if(tempPulseDataList.size()>0){
        //std::cout << "tempPulseDataList.size()=" << tempPulseDataList.size() << std::endl;
        foreach(auto item, tempPulseDataList){
            PulseItem* newitem = new PulseItem();
            newitem->onoff=item->onoff;
            newitem->holdontime=item->holdontime;
            //delete item;
            allPulseDataList.append(newitem);
            tempPulseDataList.removeOne(item);
            delete item;
        }

//        while(tempPulseDataList.isEmpty()==false){
//            PulseItem* item = tempPulseDataList.takeFirst();
//            PulseItem* newitem = new PulseItem();
//            newitem->onoff=item->onoff;
//            newitem->holdontime=item->holdontime;
//            delete item;
//            allPulseDataList.append(newitem);
//        }
        checkPulseItemList();
    }
}

ValueItem::ValueItem()
{
    clear();
}

ValueItem::ValueItem(char itemA, char itemB)
{
    dataA=itemA; dataB=itemB;
}

void ValueItem::clear()
{
    dataA=0; dataB=0; result=0;
}

char ValueItem::getDataA() const
{
    return dataA;
}

void ValueItem::setDataA(char value)
{
    dataA = value;
}

char ValueItem::getDataB() const
{
    return dataB;
}

void ValueItem::setDataB(char value)
{
    dataB = value;
}

char ValueItem::getResult()
{
    convLogic();
    return result;
}

void ValueItem::convLogic()
{
    result = 0;
    if((dataA == 'L') && (dataB == 'L')) result='0';
    else if((dataA == 'H') && (dataB == 'H')) result='1';
    else if((dataA == 'L') && (dataB == 'H')) result='F';
}

RemoteValueLogic::RemoteValueLogic()
{
    codeA='F'; codeB='F'; codeC='F';  // don't check
    recordOnly = false;
}

void RemoteValueLogic::reset()
{
    for(int i=0;i<12;i++){
        valueItems[i].clear();
    }
    memset(resultId, 0, sizeof(resultId));
    memset(resultAction, 0, sizeof (resultAction));
    result=0;
}

void RemoteValueLogic::setValue(int index, char value)
{
    if(index>=0 && index<24){
        if((index%2) == 0)valueItems[index/2].setDataA(value);
        else valueItems[index/2].setDataB(value);
    }
}

int RemoteValueLogic::getResultByMode(int mode)
{
    if(mode==6) {
        logicActionMode6();
        return 1;
    }else if(mode==8){
        logicActionMode8();
        return 1;
    }
    return 0;
}

int RemoteValueLogic::getResultByMode2(int mode)
{
    int ret = 0;
    if(mode==6) {
        ret = logicActionMode6();
    }else if(mode==8){
        ret = logicActionMode8();
    }
    return ret;
}

void RemoteValueLogic::setAllowOrganizationId(char A, char B, char C)
{
    codeA = A; codeB = B; codeC = C;
}

void RemoteValueLogic::setRecordOnly(char value)
{
    if(value=='1')setRecordOnly(true);
    else setRecordOnly(false);
}

int RemoteValueLogic::logicActionMode6()
{
    if(check4InvalidIdMode6()) return 0;
    for(int i=0;i<6;i++){
        resultId[i] = valueItems[i].getResult();
        resultAction[i]=valueItems[i+6].getResult();
    }
    if(logicCheckIdMode6()){
        return execActionMode6();
    }
    return 0;
}

int RemoteValueLogic::logicActionMode8()
{
    if(check4InvalidIdMode8()) return 0;
    for(int i=0;i<8;i++){
        resultId[i] = valueItems[i].getResult();
        if(i<4)
            resultAction[i]=valueItems[i+8].getResult();
    }
    if(logicCheckIdMode8()){
        return execActionMode8();
    }
    return 0;
}

bool RemoteValueLogic::logicCheckIdMode6()
{
    if((codeA=='F') && (codeB=='F') && (codeC=='F')) return true;   // don't check
    if((resultId[0]==codeA) && (resultId[1]==codeB) && (resultId[2]==codeC)) return true;
    return false;
}

bool RemoteValueLogic::logicCheckDisableId(){
    QString id;
    for(int i=0;i<8;i++)id.append(resultId[i]);
    if(regionManager.checkIdAllowed(id)==false) {
        QString invalidinfo = "Disable GuardId="+id;
        std::cout << invalidinfo.toStdString() << std::endl;
//        logworker.addLogger(invalidinfo, LOGTYPE_PRINT);
        return false;
    }
    return true;
}
bool RemoteValueLogic::logicCheckIdMode8()
{
    if(logicCheckDisableId()==false) return false;
    if((codeA=='F') && (codeB=='F') && (codeC=='F')) return true;   // don't check
    QString regionid;
    regionid.append(resultId[0]);regionid.append(resultId[1]);regionid.append(resultId[2]);
    if(regionManager.checkRegion(regionid)) return true;
    QString invalidinfo = "Invalid GuardId="+regionid;
    logworker.addLogger(invalidinfo, LOGTYPE_PRINT);
    std::cout << invalidinfo.toStdString() << std::endl;
    return false;
}

bool RemoteValueLogic::check4InvalidIdMode6()
{
    if((codeA=='F') && (codeB=='F') && (codeC=='F')) return false;
    bool ret=true;
    for(int i=0;i<6;i++){
        if(resultId[i]!='F'){
            ret=false;
            break;
        }
    }
    return ret;
}

bool RemoteValueLogic::check4InvalidIdMode8()
{
    if((codeA=='F') && (codeB=='F') && (codeC=='F')) return false;
    bool ret=true;
    for(int i=0;i<8;i++){
        if(resultId[i]!='F'){
            ret=false;
            break;
        }
    }
    return ret;
}

int RemoteValueLogic::execActionMode6()
{
    if(resultAction[0]=='1' && resultAction[1]=='0' && resultAction[2]=='0' && resultAction[3]=='0' && resultAction[4]=='0' && resultAction[5]=='0'){
        justTest_execAction(6);
        std::cout << "Mode6 Action 6" << std::endl;
        return 6;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='1' && resultAction[2]=='0' && resultAction[3]=='0' && resultAction[4]=='0' && resultAction[5]=='0'){
        justTest_execAction(5);
        std::cout << "Mode6 Action 5" << std::endl;
        return 5;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='0' && resultAction[2]=='1' && resultAction[3]=='0' && resultAction[4]=='0' && resultAction[5]=='0'){
        justTest_execAction(4);
        std::cout << "Mode6 Action 4" << std::endl;
        return 4;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='0' && resultAction[2]=='0' && resultAction[3]=='1' && resultAction[4]=='0' && resultAction[5]=='0'){
        justTest_execAction(3);
        std::cout << "Mode6 Action 3" << std::endl;
        return 3;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='0' && resultAction[2]=='0' && resultAction[3]=='0' && resultAction[4]=='1' && resultAction[5]=='0'){
        justTest_execAction(2);
        std::cout << "Mode6 Action 2" << std::endl;
        return 2;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='0' && resultAction[2]=='0' && resultAction[3]=='0' && resultAction[4]=='0' && resultAction[5]=='1'){
        justTest_execAction(1);
        std::cout << "Mode6 Action 1" << std::endl;
        return 1;
    }
    return 0;
}

int RemoteValueLogic::execActionMode8()
{
    if(resultAction[0]=='1' && resultAction[1]=='0' && resultAction[2]=='0' && resultAction[3]=='0'){
        justTest_execAction(1);
        std::cout << "Mode8 Action 1" << std::endl;
        return 1;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='1' && resultAction[2]=='0' && resultAction[3]=='0'){
        justTest_execAction(2);
        std::cout << "Mode8 Action 2" << std::endl;
        return 2;
    }
    else if(resultAction[0]=='1' && resultAction[1]=='1' && resultAction[2]=='0' && resultAction[3]=='0'){
        justTest_execAction(3);
        std::cout << "Mode8 Action 3" << std::endl;
        return 3;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='0' && resultAction[2]=='1' && resultAction[3]=='0'){
        justTest_execAction(4);
        std::cout << "Mode8 Action 4" << std::endl;
        return 4;
    }
    else if(resultAction[0]=='1' && resultAction[1]=='0' && resultAction[2]=='1' && resultAction[3]=='0'){
        justTest_execAction(5);
        std::cout << "Mode8 Action 5" << std::endl;
        return 5;
    }
    else if(resultAction[0]=='0' && resultAction[1]=='1' && resultAction[2]=='1' && resultAction[3]=='0'){
        justTest_execAction(6);
        std::cout << "Mode8 Action 6" << std::endl;
        return 6;
    }
    return 0;
}

void RemoteValueLogic::justTest_execAction(int mode)
{
//    time_t curtime = time(nullptr);
//    if(mode == 6){
//        remoteControlDataBuffer.setRemoteControlData(resultId, 0, 0, curtime);
//    }else{
//        remoteControlDataBuffer.setRemoteControlData(resultId, 1, mode2direction(mode), curtime);
//    }
//    if(recordOnly){
//        if(mode == 0){
//            remoteControlDataBuffer.setRemoteControlData(resultId, 0, 0, curtime);
//        }else{
//            remoteControlDataBuffer.setRemoteControlData(resultId, 1, mode2direction(mode), curtime);
//        }
//        return;
//    }

//    bool isGuard=true;
//    switch(mode){
//    case 1:
//        QByteArray ret = gProtocolAnnuniator4U.getRemoteGuardEastWestLeft();
//        emit
//        printf("guard left: east and west");
//        //guardEntry.setGuardEWLeft();
//        break;
//    case 2:
//        //if(checkLastAction(2)==false)return;
//        printf("guard left: south and north");
//        guardEntry.setGuardSNLeft();
//        break;
//    case 3:
//        //if(checkLastAction(3)==false)return;
//        printf("guard go: east and west");
//        guardEntry.setGuardEWGo();
//        break;
//    case 4:
//        //if(checkLastAction(4)==false)return;
//        printf("guard go: south and north");
//        guardEntry.setGuardSNGo();
//        break;
//    case 5:
//        //if(checkLastAction(5)==false)return;
//        printf("clear guard");
//        isGuard=false;
//        break;
//    case 6:
//        //if(checkLastAction(5)==false)return;
//        printf("clear guard");
//        isGuard=false;
//        break;
//    default:
//        isGuard=false;
//        return;
//    }


//#ifdef VERSION_TCPSERVER
//    TcpServer2Annuniator *pAnnuniator=nullptr;
//    pAnnuniator = g_AnnuniatorsMgr.getAnnuniatorFromList(pAnnuniator,0);
//    if(pAnnuniator==nullptr) {
//        printf("\r\n*^*^ Guard Quit..\r\n");
//        return;
//    }
//    if(isGuard){
//        printf("\r\n*^*^ Guard run..\r\n");
//        retEntry=pAnnuniator->SetAnnuniatorGuardControl(&cmdobj, &guardEntry);
//        printf("\r\n*^*^ Guard send..\r\n");
//    }
//    else retEntry=pAnnuniator->ClearAnnuniatorGuardControl(&cmdobjclear, &clrguardEntry);
//#else
//    if(isGuard)retEntry=m_SerialAnnuniator.SetAnnuniatorGuardControl(&cmdobj, &guardEntry);
//    else retEntry=m_SerialAnnuniator.ClearAnnuniatorGuardControl(&cmdobjclear, &clrguardEntry);
//#endif
//    if(retEntry==nullptr) return;
//    if(retEntry->getSucceed())
//    {
//        printf("\r\n*^*^ Guard ok..\r\n");
//        //guardstatus=1;
//        if(mode == 5|| mode == 6){
//            remoteControlDataBuffer.setRemoteControlData(resultId, 0, 0, curtime);
//        }else{
//            //if(mode2direction(mode)==0)return;
//            remoteControlDataBuffer.setRemoteControlData(resultId, 1, mode2direction(mode), curtime);
//        }
//        return;
//    }
    return;
}

char RemoteValueLogic::mode2direction(int mode)
{
    switch (mode) {
    case 1: return REMOTEGUARD_DIR_LEFT_EW_PROTOCOL5U;
    case 2: return REMOTEGUARD_DIR_LEFT_SN_PROTOCOL5U;
    case 3: return REMOTEGUARD_DIR_GO_EW_PROTOCOL5U;
    case 4: return REMOTEGUARD_DIR_GO_SN_PROTOCOL5U;
    default:
        break;
    }
    return 0;
}

bool RemoteValueLogic::getRecordOnly() const
{
    return recordOnly;
}

void RemoteValueLogic::setRecordOnly(bool value)
{
    recordOnly = value;
}

int RemoteValueLogic::getResultID(char *buff, int length)
{
    if(length<8) return 0;
    memcpy(buff, resultId, sizeof(resultId));
    return 1;
}

bool RemoteValueLogic::checkIdValid()
{
    return logicCheckIdMode8();
}
