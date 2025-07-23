#ifndef USERINFOFILE_H
#define USERINFOFILE_H

#include <QObject>
#include <QStringList>

#define CONFIGFILENAME	"/mnt/mmc/config.txt"
#define USERINFOFILENAME	"/mnt/mmc/userinfo.txt"

struct ConfigStructure{
    int valid;
    QString roadname;
    QString localIP;
    QString ntpServer;
    int localPort;  // 5800
    int annuniatorPort; // 5801
    QString annuniatorIP;
    int serverPort;
    QString serverIP;
    int ConnectType;    // 1 - Tcp Client
    QString server808IP;
    int server808Port;
    QString number808Id;
    QString organization;   // FFF0
    QStringList organizations;
    QStringList disabledGuardIds;
    int diableWachdog;
    int disableSyncTimeLoc;
    int guardRightAll;
    int rebooting4G;
    int rebootingPlatform;
    int rebooting808;
    int disableLogCommon;
    int reboot4timer;
};



class UserInfoFile : public QObject
{
    Q_OBJECT
public:
    explicit UserInfoFile(QObject *parent = nullptr);

    int testReadFunction();
    int testWritefunction();
    void initUserInfoData();
    int getConfigData(ConfigStructure* configdata);

    int getAnnuniatorServerPort();
    QString getAnnuniatorIP();
    QString get4GServerIP();
    int get4GServerPort();
    QString getLocalIP();
    int getLocalPort();
    int getDisableWatchDog();
    int getDisableSyncTimeLoc();
    int getGuardRightAll();
    QString getOrganization();
    QStringList getOrganizations();
    QStringList getDisableGuardList();
    QString get808PhoneId();
    QString get808ServerIP();
    int get808ServerPort();
    QString getNtpServer();
    int getDisableLogCommon();
    int getReboot4Timer();
    bool beRebootTimer();

    int getRebooting4G();
    int setRebooting4G();
    int resetRebooting4G();

    int getRebootingPlatform();
    int setRebootingPlatform();
    int resetRebootingPlatform();

    int getRebooting808();
    int setRebooting808();
    int resetRebooting808();

    int resetRebooting4GAll();
    int setRebooting4GAll();

protected:
    void initConfigData();
    void resetConfigData();
    void checkDisableGuardList();
    void resetConfigDataFromUserInfo();
signals:

private:
    ConfigStructure configData;
};
extern UserInfoFile gUserInfoFile;
#endif // USERINFOFILE_H
