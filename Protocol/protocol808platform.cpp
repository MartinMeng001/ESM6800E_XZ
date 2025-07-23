#include "Log/loggerworker.h"
#include "protocol808platform.h"
Protocol808Platform gProtocol808Platform;
Protocol808Platform::Protocol808Platform(QObject *parent) : QObject(parent)
{
    phonenumber.clear();
}

int Protocol808Platform::initSetPhoneNum(QString phonestring)
{
    int length = phonestring.length();
    if(length>12) return 0;
    int j = 5;
    phonenumber.clear();
    phonenumber.resize(6);
    for(int i=length-1;i>=0;i-=2){
        int hi = i-1, lo=i;
        if(j<0)break;
        if(hi>=0)phonenumber[j] = Char2Bcd(phonestring.toLatin1()[hi], phonestring.toLatin1()[lo]);
        else phonenumber[j] = Char2Bcd(0, phonestring.toLatin1()[lo]);
        j--;
    }
    return 1;
}

QByteArray Protocol808Platform::getCmd0002()
{
    QByteArray ret;
    int ok=makeCmdHeartBeat(ret);
    if(ok==1)numberOrderTick();
    return ret;
}

QByteArray Protocol808Platform::getCmdFF00(Trouble808Item item)
{
    QByteArray ret;
    int ok=makeCmdFF00(ret, item);
    if(ok==1)numberOrderTick();
    return ret;
}

QByteArray Protocol808Platform::parseReplyCommon()
{
    QByteArray ret;
    return ret;
}

void Protocol808Platform::numberOrderTick()
{
    if(numeralorder>=10000)numeralorder=0;
    else numeralorder++;
}

int Protocol808Platform::makeHeader(QByteArray &data)
{
    data.append(prefixHeader);  // pos 0
    return 1;
}

int Protocol808Platform::makeEnder(QByteArray &data)
{
    data.append(prefixHeader);
    return 1;
}

int Protocol808Platform::makeHeartbeatId(QByteArray &data)
{
    data.append((char)0x00); data.append((char)0x02);
    return 1;
}

int Protocol808Platform::makeTroubleId(QByteArray &data)
{
    data.append((char)0xFF); data.append((char)0x00);
    return 1;
}

int Protocol808Platform::makeHeartbeatProp(QByteArray &data)
{
    data.append((char)0x00); data.append((char)0x00);
    return 1;
}

int Protocol808Platform::makeTroubeProp(QByteArray &data)
{
    data.append((char)0x00); data.append((char)0x1C);
    return 1;
}

int Protocol808Platform::makeNumberOrder(QByteArray &data)
{
    data.append((char)(numeralorder>>8)); data.append((char)(numeralorder&0x00FF));
    return 1;
}

int Protocol808Platform::makeAlarmFlag(QByteArray &data)
{
    data.append((char)0x00); data.append((char)0x00);data.append((char)0x80); data.append((char)0x00);
    return 1;
}

int Protocol808Platform::makeAlarmStatus(QByteArray &data)
{
    data.append((char)0x00); data.append((char)0x00);data.append((char)0x80); data.append((char)0x00);
    return 1;
}

int Protocol808Platform::makeAlarmData(QByteArray &data, Trouble808Item item)
{
    data.append(item.alarmtype); data.append(item.alarmstatus);data.append(item.alarmcolor); data.append((char)0x00);
    data.append(item.alarmdirA); data.append(item.alarmchanA); data.append(item.alarmdirB); data.append(item.alarmchanB);
    data.append((char)0x00); data.append((char)0x00);//cmdbuffers[29]=0x00;cmdbuffers[30]=0x00;	// height, invalid
    data.append((char)0x00); data.append((char)0x00);//cmdbuffers[31]=0x00;cmdbuffers[32]=0x00;	// speed, invalid
    data.append((char)0x00); data.append((char)0x00);//cmdbuffers[33]=0x00;cmdbuffers[34]=0x00;	// direction, invalid
    for(int i=0;i<6;i++)data.append(item.bcdtime[i]);
    QByteArray timeinfo;
    timeinfo.append(item.bcdtime, 6);
    QString a = timeinfo.toHex().toUpper();
    QString info = QString("TroubleTime:%1").arg(a);
    logworker.addLogger(info, LOGTYPE_PRINT);
    return 1;
}

char Protocol808Platform::checksum(char *dataArray, int length)
{
    char ret=0;
    for(int i=0;i<length;i++)
    {
        ret^=dataArray[i];
    }
    return ret;
}

char Protocol808Platform::Char2Hex(char ch)
{
    switch(ch)
    {
    case '0': return 0x00;
    case '1': return 0x01;
    case '2': return 0x02;
    case '3': return 0x03;
    case '4': return 0x04;
    case '5': return 0x05;
    case '6': return 0x06;
    case '7': return 0x07;
    case '8': return 0x08;
    case '9': return 0x09;
    default: return 0x00;
    }
    return 0x00;
}

char Protocol808Platform::Char2Bcd(char high, char low)
{
    char ret=0, hi=0, lo=0;
    hi=Char2Hex(high); lo=Char2Hex(low);
    ret=(char)((hi<<4)|lo);
    return ret;
}

QByteArray Protocol808Platform::checkDataEscape(QByteArray &data)
{
    QByteArray ret;
    int length = data.size();
    for(int i=0;i<length;i++){
        if(i==0 || i==(length-1)){
            ret.append(data[i]);
            continue;
        }
        if(data[i]==(char)0x7E){
            ret.append((char)0x7D);
            ret.append((char)0x02);
        }else if(data[i]==(char)0x7D){
            ret.append((char)0x7D);
            ret.append((char)0x01);
        }else{
            ret.append(data[i]);
        }
    }
    return ret;
}

int Protocol808Platform::makeCmdHeartBeat(QByteArray &data)
{
    QByteArray temp;
    temp.clear();
    makeHeader(temp);           // pos 0
    makeHeartbeatId(temp);      // Heartbeat id, pos 1,2
    makeHeartbeatProp(temp);    // Heartbeat prop pos 3,4
    if(phonenumber.size()!=6) return 0;
    temp.append(phonenumber);   // pos 5~10
    makeNumberOrder(temp);      // numerical order, pos 11, 12
    temp.append((char)checksum(temp.mid(1).data(),12));
    makeEnder(temp);//temp.append(prefixHeader);
    data.clear();
    data = checkDataEscape(temp);
    return 1;
}

int Protocol808Platform::makeCmdFF00(QByteArray& data, Trouble808Item item)
{
    QByteArray temp;
    temp.clear();
    makeHeader(temp);           // pos 0
    makeTroubleId(temp);      // Heartbeat id, pos 1,2
    makeTroubeProp(temp);    // Heartbeat prop pos 3,4
    if(phonenumber.size()!=6) return 0;
    temp.append(phonenumber);   // pos 5~10
    makeNumberOrder(temp);      // numerical order, pos 11, 12
    // body
    makeAlarmFlag(temp);
    makeAlarmStatus(temp);
    makeAlarmData(temp, item);
    temp.append((char)checksum(temp.mid(1).data(),40));
    makeEnder(temp);//temp.append(prefixHeader);
    data.clear();
    data = checkDataEscape(temp);
    return 1;
}
