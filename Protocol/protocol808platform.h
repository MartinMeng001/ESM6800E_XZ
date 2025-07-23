#ifndef PROTOCOL808PLATFORM_H
#define PROTOCOL808PLATFORM_H

#include <QObject>
struct Trouble808Item{
    char alarmtype;
    char alarmstatus;
    char alarmcolor;
    char alarmdirA;
    char alarmchanA;
    char alarmdirB;
    char alarmchanB;
    char bcdtime[6];    // from 2000, bcd format, used the original data directly
};

class Protocol808Platform : public QObject
{
    Q_OBJECT
public:
    explicit Protocol808Platform(QObject *parent = nullptr);
    // param init
    int initSetPhoneNum(QString phonestring);

    // interfaces
    QByteArray getCmd0002();	// heartbeat
    QByteArray getCmdFF00(Trouble808Item item);	// trouble alarm
    QByteArray parseReplyCommon();// don't achieve it now
signals:

protected:
    void numberOrderTick();
    int makeHeader(QByteArray& data);	//id(2byte) bodyprop(2bytes 00-3C) phonenum(BCD6) msgnum(2bytes)
    int makeEnder(QByteArray& data);
    int makeHeartbeatId(QByteArray& data);
    int makeTroubleId(QByteArray& data);
    int makeHeartbeatProp(QByteArray& data);
    int makeTroubeProp(QByteArray& data);
    int makeNumberOrder(QByteArray& data);
    int makeAlarmFlag(QByteArray& data);
    int makeAlarmStatus(QByteArray& data);
    int makeAlarmData(QByteArray& data, Trouble808Item item);
    char checksum(char* dataArray, int length);	//xor
    //int conv7E(); // -->0x7d 0x02
    //int conv7D(); // -->0x7d 0x01
    char Char2Hex(char ch);
    char Char2Bcd(char high, char low);
    //int BCD8421Time();
    QByteArray checkDataEscape(QByteArray& data);
    //
    int makeCmdHeartBeat(QByteArray& data);
    int makeCmdFF00(QByteArray& data, Trouble808Item item);	// flag(4)+status(4)+Wei(4)+Jing(4)+Height(2)+Speed(2)+direction(2)+time(6)

private:
    const char prefixHeader = 0x7E;
    QByteArray phonenumber;
    unsigned short numeralorder;
};
extern Protocol808Platform gProtocol808Platform;

#endif // PROTOCOL808PLATFORM_H
