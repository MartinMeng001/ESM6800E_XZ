#include "Log/loggerworker.h"
#include "protocolfor4gserver.h"

ProtocolFor4GServer gProtocolFor4GServer;
ProtocolFor4GServer::ProtocolFor4GServer(QObject *parent) : QObject(parent)
{
    clearPowerAlarm();
    deviceip = "";
}

void ProtocolFor4GServer::setPowerAlarm()
{
    poweroffStatus = 1;
}

void ProtocolFor4GServer::clearPowerAlarm()
{
    poweroffStatus = 0;
}

void ProtocolFor4GServer::initProtocol4G(QString ip)
{
    deviceip = ip;
}

int ProtocolFor4GServer::checkProtocolData(QByteArray &indata, QByteArray &outdata)
{
    if(indata.size() == 6){
        logworker.addLogger(indata, LOGTYPE_PRINT);
        if(indata[0] == '*' && indata[1] == '#' && indata[2] == 'A' && indata[3] == '>' && indata[4] == '4' && indata[5] == 'G'){
            outdata = makeHearbeatInfo();
            logworker.addLogger(outdata, LOGTYPE_PRINT);
            return 1;
        }
    }
    return 0;
}

int ProtocolFor4GServer::makeProtocolData(QByteArray &indata, QByteArray &outdata)
{
    outdata = indata;
    return 1;
}

QByteArray ProtocolFor4GServer::makeHearbeat()
{
    return makeHearbeatInfo();
}

QByteArray ProtocolFor4GServer::makeHearbeatInfo()
{
    QByteArray retarray;
    retarray.append("4G;");
    retarray.append(deviceip+";");
    retarray.append(deviceip+";");
    retarray.append(makeTypeByte());
    retarray.append(";");
    return retarray;
}

char ProtocolFor4GServer::makeTypeByte()
{
    char type = 0x01;   // 0x00 5U, 0x01 4U
    if(poweroffStatus) type|=0x02;
    if(type==0x01) return '1';
    else return '3';
}
