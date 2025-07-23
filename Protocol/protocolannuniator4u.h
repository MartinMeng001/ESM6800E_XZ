#ifndef PROTOCOLANNUNIATOR4U_H
#define PROTOCOLANNUNIATOR4U_H

#include <QObject>
#include <QDateTime>
#include <QByteArray>

#define PROTOCOL4U_DIRECTION_EAST   1
#define PROTOCOL4U_DIRECTION_WEST   2
#define PROTOCOL4U_DIRECTION_SOUTH  3
#define PROTOCOL4U_DIRECTION_NORTH  4
#define PROTOCOL4U_CHANNEL_GO       0x01
#define PROTOCOL4U_CHANNEL_LEFT     0x02
#define PROTOCOL4U_CHANNEL_RIGHT    0x04

#define PROTOCOL4U_INSTR_CANCELGUARD    0xE7
#define PROTOCOL4U_INSTR_GUARD          0xE9
#define PROTOCOL4U_INSTR_TROUBLEUPLOAD  0xA0
#define PROTOCOL4U_INSTR_TROUBLE        0xA1
#define PROTOCOL4U_INSTR_GETTIME        0x88
#define PROTOCOL4U_INSTR_SETTIME        0x87

#define LOCALPARSED_SUPPORT_TROUBLE     1
#define LOCALPARSED_SUPPORT_TIME        2
class ProtocolAnnuniator4U : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolAnnuniator4U(QObject *parent = nullptr);

    QByteArray getRemoteGuardEastWestGo();
    QByteArray getRemoteGuardEastWestLeft();
    QByteArray getRemoteGuardSouthNorthGo();
    QByteArray getRemoteGuardSouthNorthLeft();
    QByteArray setCancleGuardStatus();
    QByteArray getAnnuniatorTime();
    QByteArray setAnnuniatorTime();
    QByteArray getTroubleA1ByTroubleNo(QByteArray troubleNo);

    int setAnnuniatorData(QByteArray data);
    void initProtocol4U();
public slots:
    void processAnnuniatorData(QByteArray data);
protected:
    int getGuardDir(char channelsEast, char channelsWest, char channelsSouth, char channelsNorth);
    void makeData(int guardDir, QByteArray& data);
    void makeDataCancelGuard(QByteArray& data);
    void makeDataGetAnnuniatorTime(QByteArray& data);
    void makeDataSetAnnuniatorTime(QByteArray& data);
    void makeDataSetAnnuniatorTrouble(QByteArray& data, QByteArray troubleNo);
    char getData(int go, int left, int right);
    char getVerifyCode(char* data, int datalength);
    void setHeader(QByteArray& data);
    void setDevAddr(QByteArray& data);
    void setInstr(char instr, QByteArray& data);
    void setTime(QByteArray& data);
    void setData(char east, char west, char south, char north, QByteArray& data);
    void setTime4Instr87(QByteArray& data);

    int parsedLogic(QByteArray data);
    int parseTroubleIndex(QByteArray data);
    int parseTrouble(QByteArray data);
    int parseAnnuniatorTime(QByteArray data);
    void config();
    // basic function
    char GetYearCode(int year);
    char GetTimeCodeInstr87(int data);
    int GetHexValue(char value);
    char DecimalHex(int HighNum, int LowNum);
    int GetYearFromAnnuniator(char value);
    //
    void testTroubleParse();
signals:
    void sigTrouble(QByteArray data);
    void sigNewTrouble(QByteArray troubleNo);
    void sigAnnuniatorTime(QDateTime annuniatorTime);
private:
    bool guardRightAll;
};
extern ProtocolAnnuniator4U gProtocolAnnuniator4U;
#endif // PROTOCOLANNUNIATOR4U_H
