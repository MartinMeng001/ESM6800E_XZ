#include <stdio.h>
#include <string.h>
#include <QDateTime>
#include "Log/loggerworker.h"
#include "Configfile/userinfofile.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "protocolannuniator4u.h"

ProtocolAnnuniator4U gProtocolAnnuniator4U;
ProtocolAnnuniator4U::ProtocolAnnuniator4U(QObject *parent) : QObject(parent)
{
    guardRightAll=false;
}

QByteArray ProtocolAnnuniator4U::getRemoteGuardEastWestGo()
{
    QByteArray ret;
    ret.resize(23);
    char channelsEast=PROTOCOL4U_CHANNEL_GO, channelsWest=PROTOCOL4U_CHANNEL_GO, channelsSouth=0, channelsNorth=0;
    if(guardRightAll){
        channelsEast|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsWest|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsSouth=PROTOCOL4U_CHANNEL_RIGHT;
        channelsNorth=PROTOCOL4U_CHANNEL_RIGHT;
    }
    int guarddir = getGuardDir(channelsEast, channelsWest, channelsSouth, channelsNorth);
    makeData(guarddir, ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::getRemoteGuardEastWestLeft()
{
    QByteArray ret;
    ret.resize(23);
    char channelsEast=PROTOCOL4U_CHANNEL_LEFT, channelsWest=PROTOCOL4U_CHANNEL_LEFT, channelsSouth=0, channelsNorth=0;
    if(guardRightAll){
        channelsEast|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsWest|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsSouth=PROTOCOL4U_CHANNEL_RIGHT;
        channelsNorth=PROTOCOL4U_CHANNEL_RIGHT;
    }
    int guarddir = getGuardDir(channelsEast, channelsWest, channelsSouth, channelsNorth);
    makeData(guarddir, ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::getRemoteGuardSouthNorthGo()
{
    QByteArray ret;
    ret.resize(23);
    char channelsEast=0, channelsWest=0, channelsSouth=PROTOCOL4U_CHANNEL_GO, channelsNorth=PROTOCOL4U_CHANNEL_GO;
    if(guardRightAll){
        channelsEast=PROTOCOL4U_CHANNEL_RIGHT;
        channelsWest=PROTOCOL4U_CHANNEL_RIGHT;
        channelsSouth|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsNorth|=PROTOCOL4U_CHANNEL_RIGHT;
    }
    int guarddir = getGuardDir(channelsEast, channelsWest, channelsSouth, channelsNorth);
    makeData(guarddir, ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::getRemoteGuardSouthNorthLeft()
{
    QByteArray ret;
    ret.resize(23);
    char channelsEast=0, channelsWest=0, channelsSouth=PROTOCOL4U_CHANNEL_LEFT, channelsNorth=PROTOCOL4U_CHANNEL_LEFT;
    if(guardRightAll){
        channelsEast=PROTOCOL4U_CHANNEL_RIGHT;
        channelsWest=PROTOCOL4U_CHANNEL_RIGHT;
        channelsSouth|=PROTOCOL4U_CHANNEL_RIGHT;
        channelsNorth|=PROTOCOL4U_CHANNEL_RIGHT;
    }
    int guarddir = getGuardDir(channelsEast, channelsWest, channelsSouth, channelsNorth);
    makeData(guarddir, ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::setCancleGuardStatus()
{
    QByteArray ret;
    ret.resize(10);
    makeDataCancelGuard(ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::getAnnuniatorTime()
{
    QByteArray ret;
    ret.resize(10);
    makeDataGetAnnuniatorTime(ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::setAnnuniatorTime()
{
    QByteArray ret;
    ret.resize(15);
    makeDataSetAnnuniatorTime(ret);
    return ret;
}

QByteArray ProtocolAnnuniator4U::getTroubleA1ByTroubleNo(QByteArray troubleNo)
{
    QByteArray ret;
    if(troubleNo.size()==2){
        ret.resize(11);
        makeDataSetAnnuniatorTrouble(ret, troubleNo);
    }
    return ret;
}

int ProtocolAnnuniator4U::setAnnuniatorData(QByteArray data)
{
    return parsedLogic(data);
}

void ProtocolAnnuniator4U::initProtocol4U()
{
    if(gUserInfoFile.getGuardRightAll()==1)guardRightAll=true;
    else guardRightAll=false;
    config();
    QString info = QString("Protocol4U GuardRightAll=%1").arg(guardRightAll);
    logworker.addLogger(info, LOGTYPE_PRINT);
}

void ProtocolAnnuniator4U::processAnnuniatorData(QByteArray data)
{
    setAnnuniatorData(data);
    //testTroubleParse();
}

int ProtocolAnnuniator4U::getGuardDir(char channelsEast, char channelsWest, char channelsSouth, char channelsNorth)
{
    int ret = 0;

    if((channelsEast&PROTOCOL4U_CHANNEL_GO)!=0)ret|=0x01;
    if((channelsEast&PROTOCOL4U_CHANNEL_LEFT)!=0)ret|=0x02;
    if((channelsEast&PROTOCOL4U_CHANNEL_RIGHT)!=0)ret|=0x04;

    if((channelsWest&PROTOCOL4U_CHANNEL_GO)!=0)ret|=0x08;
    if((channelsWest&PROTOCOL4U_CHANNEL_LEFT)!=0)ret|=0x10;
    if((channelsWest&PROTOCOL4U_CHANNEL_RIGHT)!=0)ret|=0x20;

    if((channelsSouth&PROTOCOL4U_CHANNEL_GO)!=0)ret|=0x40;
    if((channelsSouth&PROTOCOL4U_CHANNEL_LEFT)!=0)ret|=0x80;
    if((channelsSouth&PROTOCOL4U_CHANNEL_RIGHT)!=0)ret|=0x100;

    if((channelsNorth&PROTOCOL4U_CHANNEL_GO)!=0)ret|=0x200;
    if((channelsNorth&PROTOCOL4U_CHANNEL_LEFT)!=0)ret|=0x400;
    if((channelsNorth&PROTOCOL4U_CHANNEL_RIGHT)!=0)ret|=0x800;
    return ret;
}

void ProtocolAnnuniator4U::makeData(int guardDir, QByteArray& data)
{
    int go, left, right;
    char east, west, south, north;
    go = (guardDir&0x01)==0?0:1;
    left = (guardDir&0x02)==0?0:1;
    right = (guardDir&0x04)==0?0:1;
    east = getData(go,left, right);

    go = (guardDir&0x08)==0?0:1;
    left = (guardDir&0x10)==0?0:1;
    right = (guardDir&0x20)==0?0:1;
    west = getData(go,left, right);

    go = (guardDir&0x40)==0?0:1;
    left = (guardDir&0x80)==0?0:1;
    right = (guardDir&0x100)==0?0:1;
    south = getData(go,left, right);

    go = (guardDir&0x200)==0?0:1;
    left = (guardDir&0x400)==0?0:1;
    right = (guardDir&0x800)==0?0:1;
    north = getData(go,left, right);

    setHeader(data);
    setDevAddr(data);
    setInstr(PROTOCOL4U_INSTR_GUARD, data);
    setTime(data);
    setData(east, west, south, north, data);
    data[20] = getVerifyCode(data.mid(4).data(), 16);
    data[21] = (char)0xAA;
    data[22] = (char)0x55;
}

void ProtocolAnnuniator4U::makeDataCancelGuard(QByteArray& data)
{
    setHeader(data);
    setDevAddr(data);
    setInstr(PROTOCOL4U_INSTR_CANCELGUARD, data);
    data[6] = (char)0x00;
    data[7] = getVerifyCode(data.mid(4).data(), 3);
    data[8] = (char)0xAA;
    data[9] = (char)0x55;
}

void ProtocolAnnuniator4U::makeDataGetAnnuniatorTime(QByteArray &data)
{
    setHeader(data);
    setDevAddr(data);
    setInstr(PROTOCOL4U_INSTR_GETTIME, data);
    data[6] = (char)0x00;
    data[7] = getVerifyCode(data.mid(4).data(), 3);
    data[8] = (char)0xAA;
    data[9] = (char)0x55;
}

void ProtocolAnnuniator4U::makeDataSetAnnuniatorTime(QByteArray &data)
{
    setHeader(data);
    setDevAddr(data);
    setInstr(PROTOCOL4U_INSTR_SETTIME, data);
    setTime4Instr87(data);
    data[12] = getVerifyCode(data.mid(4).data(), 8);
    data[13] = (char)0xAA;
    data[14] = (char)0x55;
}

void ProtocolAnnuniator4U::makeDataSetAnnuniatorTrouble(QByteArray &data, QByteArray troubleNo)
{
    if(troubleNo.size()!=2) return;
    setHeader(data);
    setDevAddr(data);
    setInstr(PROTOCOL4U_INSTR_TROUBLE, data);
    data[6]=(char)troubleNo.data()[0];
    data[7]=(char)troubleNo.data()[1];
    //data.append(troubleNo);
    data[8] = getVerifyCode(data.mid(4).data(), 4);
    data[9] = (char)0xAA;
    data[10] = (char)0x55;
}

char ProtocolAnnuniator4U::getData(int go, int left, int right)
{
    char ret=0;
    if(go==1)ret|=0x04;
    if(left==1)ret|=0x02;
    if(right==1)ret|=0x08;
    return ret;
}

char ProtocolAnnuniator4U::getVerifyCode(char *data, int datalength)
{
    char ret=data[0];
    for(int i=1;i<datalength;i++)
    {
        ret=(char)(ret^data[i]);
    }
    return ret;
}

void ProtocolAnnuniator4U::setHeader(QByteArray &data)
{
    data[0]=(char)0x53;data[1]=(char)0x42;data[2]=(char)0x44;data[3]=(char)0x5A;
}

void ProtocolAnnuniator4U::setDevAddr(QByteArray& data)
{
    data[4]=(char)0x00;
}

void ProtocolAnnuniator4U::setInstr(char instr, QByteArray& data)
{
    data[5]=instr;
}

void ProtocolAnnuniator4U::setTime(QByteArray& data)
{
    data[6]=(char)0x00;
    data[7]=(char)0x00;
}

void ProtocolAnnuniator4U::setData(char east, char west, char south, char north, QByteArray& data)
{
    data[8]=(char)((~east)&0x00FF); data[9]=0x00; data[10]=east;
    data[11]=(char)((~west)&0x00FF); data[12]=0x00; data[13]=west;
    data[14]=(char)((~south)&0x00FF); data[15]=0x00; data[16]=south;
    data[17]=(char)((~north)&0x00FF); data[18]=0x00; data[19]=north;
}

void ProtocolAnnuniator4U::setTime4Instr87(QByteArray &data)
{
    QDateTime now = QDateTime::currentDateTime();
    data[6] = GetYearCode(now.date().year());
    data[7] = GetTimeCodeInstr87( now.date().month() );
    data[8] = GetTimeCodeInstr87( now.date().day() );
    data[9] = GetTimeCodeInstr87( now.time().hour() );
    data[10] = GetTimeCodeInstr87( now.time().minute() );
    data[11] = GetTimeCodeInstr87( now.time().second() );
}

int ProtocolAnnuniator4U::parsedLogic(QByteArray data)
{
    switch (data[0]) {
    case PROTOCOL4U_INSTR_TROUBLEUPLOAD:
        return parseTroubleIndex(data);
    case PROTOCOL4U_INSTR_TROUBLE:
        //logworker.addLogger("****FOUND PROTOCOL4U_INSTR_TROUBLE", LOGTYPE_PRINT);
        return parseTrouble(data);
    case PROTOCOL4U_INSTR_GETTIME:
        return parseAnnuniatorTime(data);
    }
    return 0;
}

int ProtocolAnnuniator4U::parseTroubleIndex(QByteArray data)
{
    if(data[1]==(char)0xC2) return 0;
    if(data.size()<7) return 0;
    char checksum = getVerifyCode(data.data(), 6);
    if(checksum != data.data()[6]) {
        logworker.addLogger("****TROUBLE AUTOUPLOAD CHECKSUM ERR", LOGTYPE_PRINT);
        return 0;
    }
    QByteArray ret = data.mid(1, 2);
    //logworker.addLogger("****EMIT TROUBLE PROTOCOL4U_INSTR_TROUBLE", LOGTYPE_PRINT);
    //QString info = ret.toHex().toUpper();
    //logworker.addLogger("****EMIT TROUBLE NO:"+info, LOGTYPE_PRINT);
    emit sigNewTrouble(ret);
    return 1;
}

int ProtocolAnnuniator4U::parseTrouble(QByteArray data)
{
    if(data[1]==(char)0xC2) return 0;
    if(data.size()<19) return 0;
    char checksum = getVerifyCode(data.data(), 17);
    if(checksum != data.data()[17]) {
        QByteArray a;
        a.append(data);
        a.append(checksum);
        QString info = a.toHex().toUpper();
        logworker.addLogger("****CHECKSUM ERR PROTOCOL4U_INSTR_TROUBLE:"+info, LOGTYPE_PRINT);
        return 0;
    }
    QByteArray ret = data.mid(1, 15);
    emit sigTrouble(ret);
    //QString info = data.toHex().toUpper();
    //logworker.addLogger("TroubleINFO:"+info, LOGTYPE_RECORD);
    return 1;
}

int ProtocolAnnuniator4U::parseAnnuniatorTime(QByteArray data)
{
    if(data.size()<10) return 0;
    char checksum = getVerifyCode(data.data(), 7);
    if(checksum != data[7]) return 0;

    int year=GetYearFromAnnuniator(data[1]);
    int month=GetHexValue(data[2]);//if(month>0)month--;
    int day=GetHexValue(data[3]);
    int hour=GetHexValue(data[4]);
    int minute=GetHexValue(data[5]);
    int second=GetHexValue(data[6]);

    QString timeinfo = QString("%1-%2-%3 %4:%5:%6").arg(year, 4, 10, QLatin1Char('0'))
            .arg(month, 2, 10, QLatin1Char('0'))
            .arg(day, 2, 10, QLatin1Char('0'))
            .arg(hour, 2, 10, QLatin1Char('0'))
            .arg(minute, 2, 10, QLatin1Char('0'))
            .arg(second, 2, 10, QLatin1Char('0'));
    QDateTime datetime = QDateTime::fromString(timeinfo, "yyyy-MM-dd HH:mm:ss");
    //datetime.date().setDate(year, month, day);
    //datetime.time().setHMS(hour, minute, second);
    emit sigAnnuniatorTime(datetime);
    QString infotime = QString("AnnuniatorTime:%1").arg(timeinfo);
    logworker.addLogger(infotime, LOGTYPE_PRINT);
    return 1;
}

void ProtocolAnnuniator4U::config()
{
    connect(this, &ProtocolAnnuniator4U::sigTrouble, &gAnnuniatorStatus, &AnnuniatorStatus::Status_TroubleStatus);
    connect(this, &ProtocolAnnuniator4U::sigNewTrouble, &gAnnuniatorStatus, &AnnuniatorStatus::Action_GetAnnuniatorTrouble);
    connect(this, &ProtocolAnnuniator4U::sigAnnuniatorTime, &gAnnuniatorStatus, &AnnuniatorStatus::Status_AnniniatorTime);
}

char ProtocolAnnuniator4U::GetYearCode(int year)
{
    int code=year-2000;
    char codeh=code/10;
    char codel=code%10;
    return (char)(codeh*0x10+codel);
}

char ProtocolAnnuniator4U::GetTimeCodeInstr87(int data)
{
    char hi = data/10;
    char lo = data%10;
    return DecimalHex(hi, lo);
}

int ProtocolAnnuniator4U::GetHexValue(char value)
{
    char temp[10];
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%02X", value);
    return atoi(temp);
}
char ProtocolAnnuniator4U::DecimalHex(int HighNum, int LowNum)
{
    int high=HighNum*0x10;
    int low=LowNum;
    return (char)(high|low);
}

int ProtocolAnnuniator4U::GetYearFromAnnuniator(char value)
{
    char temp[10];
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "20%02X", value);
    return atoi(temp);
}

void ProtocolAnnuniator4U::testTroubleParse()
{
    QByteArray data;
    data.append((char)0xA1);data.append((char)0x05);
    data.append((char)0x22);data.append((char)0x01);data.append((char)0x08);data.append((char)0x11);data.append((char)0x13);data.append((char)0x42);
    data.append((char)0x01);data.append((char)0x00);
    data.append((char)0x22);data.append((char)0x01);data.append((char)0x08);data.append((char)0x11);data.append((char)0x13);data.append((char)0x47);
    data.append((char)0xFF);data.append((char)0x5F);data.append((char)0xAA);data.append((char)0x55);
    parsedLogic(data);
}

