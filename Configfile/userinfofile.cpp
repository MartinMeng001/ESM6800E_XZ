#include <iostream>
#include <QSettings>
#include <QString>
#include <QDateTime>
#include <QTextCodec>
#include "userinfofile.h"
#include "Log/loggerworker.h"

#define SETTING_PARAMS_VALID            "/PARAMS/Valid"
#define SETTING_PARAMS_ROADNAME         "/PARAMS/Roadname"
#define SETTING_PARAMS_LOCALPORT        "/PARAMS/LocalPort"
#define SETTING_PARAMS_ORGANIZATION     "/PARAMS/Organization"
#define SETTING_PARAMS_ORGANIZATIONS    "/PARAMS/Organizations"
#define SETTING_PARAMS_DISABLEDGUARDIDS "/PARAMS/DisableGuardIDs"
#define SETTING_PARAMS_ANNUNIATORIP     "/PARAMS/AnnuniatorIP"
#define SETTING_PARAMS_ANNUNIATORPORT "/PARAMS/AnnuniatorPort"
#define SETTING_PARAMS_NTPSERVER "/PARAMS/NtpServer"
#define SETTING_PARAMS_DISWATCHDOG "/PARAMS/DisWatchDog"
#define SETTING_PARAMS_DISSYNCTIMELOC "/PARAMS/DisSyncTimeLoc"
#define SETTING_PARAMS_GUARDRIGHTALL "/PARAMS/GuardRightAll"
#define SETTING_PARAMS_4GREBOOTING "/PARAMS/RebootingCount4G"
#define SETTING_PARAMS_PLATFORMREBOOTING "/PARAMS/RebootingPlatform"
#define SETTING_PARAMS_808BOOTING   "/PARAMS/Rebooting808"
#define SETTING_PARAMS_DISABLELOGCOMMON   "/PARAMS/DisableLogCommon"
#define SETTING_PARAMS_TIMER4REBOOT   "/PARAMS/RebootTimer"

#define SETTING_SERVER_SERVERIP "/SERVER/ServerIP"
#define SETTING_SERVER_SERVERPORT "/SERVER/ServerPort"
#define SETTING_SERVER_CONNECTTYPE "/SERVER/ConnectType"

#define SETTING_SERVER808_NUMBER808ID "/SERVER808/Number808Id"
#define SETTING_SERVER808_SERVERIP "/SERVER808/ServerIP"
#define SETTING_SERVER808_SERVERPORT "/SERVER808/ServerPort"

#define DEFAULTVALLUE_4GREBOOTING               3
#define DEFAULTVALUE_PLATFORMREBOOOTING         1
#define DEFAULTVALUE_808REBOOTING               1


UserInfoFile gUserInfoFile;
UserInfoFile::UserInfoFile(QObject *parent) : QObject(parent)
{

}

int UserInfoFile::testReadFunction()
{
    QSettings *configIniRead = new QSettings(USERINFOFILENAME, QSettings::IniFormat);
    configIniRead->setIniCodec(QTextCodec::codecForName("utf-8"));
    QString dhcp = configIniRead->value("/LOCAL_MACHINE/DHCP").toString();
    QString IPAddress = configIniRead->value("/LOCAL_MACHINE/IPAddress").toString();
    QString SubNetmask = configIniRead->value("/LOCAL_MACHINE/SubnetMask").toString();
    QString Roadname2 = QString::fromLatin1((char*)configIniRead->value("/LOCAL_MACHINE/Roadname").data());
    QString RoadName = QString::fromUtf8(configIniRead->value("/LOCAL_MACHINE/Roadname").toByteArray());
    delete configIniRead;
    std::cout << Roadname2.toStdString() << std::endl;
    std::cout << RoadName.toStdString() << std::endl;
    logworker.addLogger(dhcp, LOGTYPE_PRINT);
    logworker.addLogger(IPAddress, LOGTYPE_PRINT);
    logworker.addLogger(SubNetmask, LOGTYPE_PRINT);
    logworker.addLogger(RoadName, LOGTYPE_PRINT);
    return 0;
}

int UserInfoFile::testWritefunction()
{
    QSettings *configIniWrite = new QSettings("/mnt/mmc/userinfo2.txt", QSettings::IniFormat);
    configIniWrite->setIniCodec(QTextCodec::codecForName("utf-8"));
    //configIniWrite->beginGroup("setting");
    configIniWrite->setValue("/LOCAL_MACHINE/DHCP",QVariant("1"));
    configIniWrite->setValue("/LOCAL_MACHINE/IPAddress",QString("%1.%2").arg("192.168.3.87").arg(""));
    configIniWrite->setValue("/LOCAL_MACHINE/SubnetMask",QVariant("255.255.248.0"));
    configIniWrite->setValue("/LOCAL_MACHINE/Roadname",QString("测试路口"));
    configIniWrite->setValue("/LOCAL_MACHINE/Roadname2",QString("泰山路与文昌路"));
    delete configIniWrite;
    logworker.addLogger(QString("泰山路与文昌路"), LOGTYPE_PRINT);
    logworker.addLogger("write data to config file", LOGTYPE_PRINT);
    return 0;
}

void UserInfoFile::initUserInfoData()
{
    initConfigData();
}

int UserInfoFile::getConfigData(ConfigStructure *configdata)
{
    configdata = &configData;
    return 1;
}

int UserInfoFile::getAnnuniatorServerPort()
{
    return configData.annuniatorPort;
}

QString UserInfoFile::getAnnuniatorIP()
{
    return configData.annuniatorIP;
}

QString UserInfoFile::get4GServerIP()
{
    return configData.serverIP;
}

int UserInfoFile::get4GServerPort()
{
    return configData.serverPort;
}

QString UserInfoFile::getLocalIP()
{
    return configData.localIP;
}

int UserInfoFile::getLocalPort()
{
    return configData.localPort;
}

int UserInfoFile::getDisableWatchDog()
{
    return configData.diableWachdog;
}

int UserInfoFile::getDisableSyncTimeLoc()
{
    return configData.disableSyncTimeLoc;
}

int UserInfoFile::getGuardRightAll()
{
    return configData.guardRightAll;
}

QString UserInfoFile::getOrganization()
{
    return configData.organization;
}

QStringList UserInfoFile::getOrganizations()
{
    return configData.organizations;
}

QStringList UserInfoFile::getDisableGuardList()
{
    return configData.disabledGuardIds;
}

QString UserInfoFile::get808PhoneId()
{
    return configData.number808Id;
}

QString UserInfoFile::get808ServerIP()
{
    return configData.server808IP;
}

int UserInfoFile::get808ServerPort()
{
    return configData.server808Port;
}

QString UserInfoFile::getNtpServer()
{
    return configData.ntpServer;
}

int UserInfoFile::getDisableLogCommon()
{
    return configData.disableLogCommon;
}

int UserInfoFile::getReboot4Timer()
{
    return configData.reboot4timer;
}

bool UserInfoFile::beRebootTimer()
{
    if(configData.reboot4timer == -1) return false;
    if(configData.reboot4timer<0 || configData.reboot4timer>23) return false;
    QDateTime now=QDateTime::currentDateTime();
    //std::cout << now.toString().toStdString() << std::endl;
    if(now.time().second()==0 && now.time().minute()==0){
        if(now.time().hour()==configData.reboot4timer) return true;
    }
    return false;
}

int UserInfoFile::getRebooting4G()
{
    int ret = configData.rebooting4G;
    if(ret>0){
        configData.rebooting4G--;
        setRebooting4G();
    }
    return ret;
}

int UserInfoFile::resetRebooting4GAll()
{
    if(configData.rebooting4G == DEFAULTVALLUE_4GREBOOTING){
        if(configData.rebootingPlatform == DEFAULTVALUE_PLATFORMREBOOOTING){
            if(configData.rebooting808 == DEFAULTVALUE_808REBOOTING){
                return 0;
            }
        }
    }
    configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    setRebooting4GAll();
    return 1;
}
int UserInfoFile::setRebooting4GAll()
{
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    delete configIniWrite;
    return 0;
}

int UserInfoFile::setRebooting4G()
{
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
//    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
//    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    delete configIniWrite;
    return 0;
}

int UserInfoFile::resetRebooting4G()
{
    if(configData.rebooting4G == DEFAULTVALLUE_4GREBOOTING){
        //if(configData.rebootingPlatform == DEFAULTVALUE_PLATFORMREBOOOTING){
        //    if(configData.rebooting808 == DEFAULTVALUE_808REBOOTING){
                return 0;
        //    }
        //}
    }
    configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    //configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    //configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    setRebooting4G();
    return 1;
}

int UserInfoFile::getRebootingPlatform()
{
    int ret = configData.rebootingPlatform;
    if(ret>0){
        configData.rebootingPlatform--;
        setRebootingPlatform();
    }
    return ret;
}

int UserInfoFile::setRebootingPlatform()
{
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
//    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
//    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    delete configIniWrite;
    return 0;
}

int UserInfoFile::resetRebootingPlatform()
{
    //if(configData.rebooting4G == DEFAULTVALLUE_4GREBOOTING){
        if(configData.rebootingPlatform == DEFAULTVALUE_PLATFORMREBOOOTING){
        //    if(configData.rebooting808 == DEFAULTVALUE_808REBOOTING){
                return 0;
        //    }
        }
    //}
    //configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    //configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    setRebootingPlatform();
    return 1;
}

int UserInfoFile::getRebooting808()
{
    int ret = configData.rebooting808;
    if(ret>0){
        configData.rebooting808--;
        setRebooting808();
    }
    return ret;
}

int UserInfoFile::setRebooting808()
{
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
//    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
//    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    delete configIniWrite;
    return 0;
}

int UserInfoFile::resetRebooting808()
{
    //if(configData.rebooting4G == DEFAULTVALLUE_4GREBOOTING){
        //if(configData.rebootingPlatform == DEFAULTVALUE_PLATFORMREBOOOTING){
            if(configData.rebooting808 == DEFAULTVALUE_808REBOOTING){
                return 0;
            }
        //}
    //}
    //configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    //configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    setRebooting808();
    return 1;
}


void UserInfoFile::initConfigData()
{
    QSettings *configIniRead2 = new QSettings(USERINFOFILENAME, QSettings::IniFormat);
    configData.localIP = configIniRead2->value("/LOCAL_MACHINE/IPAddress").toString();
    if((configData.localIP == "192.168.0.100") || (configData.localIP == "193.168.0.100"))
        configData.localIP = configIniRead2->value("/ETH1/IPAddress").toString();
    delete configIniRead2;
    QSettings *configIniRead = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
    if(configIniRead==nullptr){
        logworker.addLogger("configfile don't exist, reset data", LOGTYPE_PRINT);
        resetConfigData();
    }

    if(configIniRead->contains(SETTING_PARAMS_VALID)==false){
        delete configIniRead;
        logworker.addLogger("config file is invalid, reset data", LOGTYPE_PRINT);
        resetConfigData();
    }else{
        configData.valid = configIniRead->value(SETTING_PARAMS_VALID).toInt();
        if(configData.valid==2){
            logworker.addLogger("restore config file from userinfo.txt", LOGTYPE_PRINT);
            resetConfigDataFromUserInfo();
        }else{
            logworker.addLogger("read params from config file", LOGTYPE_PRINT);
            configData.roadname = QString::fromUtf8(configIniRead->value(SETTING_PARAMS_ROADNAME).toByteArray());
            configData.localPort = configIniRead->value(SETTING_PARAMS_LOCALPORT).toInt();
            configData.annuniatorPort = configIniRead->value(SETTING_PARAMS_ANNUNIATORPORT).toInt();
            configData.annuniatorIP = configIniRead->value(SETTING_PARAMS_ANNUNIATORIP).toString();
            if(configData.annuniatorIP==nullptr)configData.annuniatorIP="192.168.0.99";
            configData.organization = configIniRead->value(SETTING_PARAMS_ORGANIZATION).toString();
            configData.ntpServer = configIniRead->value(SETTING_PARAMS_NTPSERVER).toString();
            configData.diableWachdog = configIniRead->value(SETTING_PARAMS_DISWATCHDOG).toInt();
            configData.disableSyncTimeLoc = configIniRead->value(SETTING_PARAMS_DISSYNCTIMELOC).toInt();
            configData.guardRightAll = configIniRead->value(SETTING_PARAMS_GUARDRIGHTALL).toInt();
            configData.organizations = configIniRead->value(SETTING_PARAMS_ORGANIZATIONS).value<QStringList>();
            configData.disabledGuardIds = configIniRead->value(SETTING_PARAMS_DISABLEDGUARDIDS).value<QStringList>();
            configData.rebooting4G = configIniRead->value(SETTING_PARAMS_4GREBOOTING).toInt();
            configData.rebootingPlatform = configIniRead->value(SETTING_PARAMS_PLATFORMREBOOTING).toInt();
            configData.rebooting808 = configIniRead->value(SETTING_PARAMS_808BOOTING).toInt();
            configData.disableLogCommon = configIniRead->value(SETTING_PARAMS_DISABLELOGCOMMON).toInt();
            configData.reboot4timer = configIniRead->value(SETTING_PARAMS_TIMER4REBOOT).toInt();

            configData.serverIP = configIniRead->value(SETTING_SERVER_SERVERIP).toString();
            configData.serverPort = configIniRead->value(SETTING_SERVER_SERVERPORT).toInt();
            configData.ConnectType = configIniRead->value(SETTING_SERVER_CONNECTTYPE).toInt();

            configData.number808Id = configIniRead->value(SETTING_SERVER808_NUMBER808ID).toString();
            configData.server808IP = configIniRead->value(SETTING_SERVER808_SERVERIP).toString();
            configData.server808Port = configIniRead->value(SETTING_SERVER808_SERVERPORT).toInt();
            checkDisableGuardList();

        }
        delete configIniRead;
    }
    std::cout << "****** reboot for timer " << configData.reboot4timer << std::endl;
    std::cout << "RebootingCount4G=" << configData.rebooting4G << std::endl;
    //configIniRead->setValue("/PARAMS/Valid",configData.valid);
}

void UserInfoFile::resetConfigData()
{
    configData.valid = 1;
    configData.roadname = QString::fromUtf8(QByteArray("未知路口"));
    configData.ntpServer = "120.25.115.20";
    configData.serverIP = "117.158.154.17";
    configData.localPort = 5800;
    configData.annuniatorPort = 5801;
    configData.annuniatorIP = "192.168.0.99";
    configData.serverPort = 8125;
    configData.ConnectType = 1;
    configData.number808Id = "0000000000";
    configData.server808IP = "1.194.232.48";
    configData.server808Port = 6808;
    configData.organization = "0FF0";
    configData.guardRightAll = 1;
    configData.diableWachdog = 0;
    configData.disableLogCommon = 0;
    configData.reboot4timer = -1;
    configData.disableSyncTimeLoc = 1;
    configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    configData.organizations.clear();
    configData.disabledGuardIds.clear();
    configData.organizations << "FF0" << "FF1" << "F0F" << "F00" << "F01" << "F1F" << "F11" << "0F0" << "000" << "001" << "010" << "011" << "100" << "1F1" << "111" << "FFF" << "FFF" << "FFF" << "FFF" << "FFF";
    configData.disabledGuardIds << "F0FFFFF1" << "FFFFFFFF";
    logworker.addLogger(configData.roadname, LOGTYPE_PRINT);
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
    configIniWrite->setIniCodec(QTextCodec::codecForName("utf-8"));
    configIniWrite->setValue(SETTING_PARAMS_VALID,configData.valid);
    configIniWrite->setValue(SETTING_PARAMS_ROADNAME, configData.roadname);
    configIniWrite->setValue(SETTING_PARAMS_LOCALPORT, configData.localPort);
    configIniWrite->setValue(SETTING_PARAMS_ANNUNIATORPORT, configData.annuniatorPort);
    configIniWrite->setValue(SETTING_PARAMS_ANNUNIATORIP, configData.annuniatorIP);
    configIniWrite->setValue(SETTING_PARAMS_ORGANIZATION, configData.organization);
    configIniWrite->setValue(SETTING_PARAMS_ORGANIZATIONS, configData.organizations);
    configIniWrite->setValue(SETTING_PARAMS_DISABLEDGUARDIDS, configData.disabledGuardIds);
    configIniWrite->setValue(SETTING_PARAMS_NTPSERVER, configData.ntpServer);
    configIniWrite->setValue(SETTING_PARAMS_DISWATCHDOG, configData.diableWachdog);
    configIniWrite->setValue(SETTING_PARAMS_DISSYNCTIMELOC, configData.disableSyncTimeLoc);
    configIniWrite->setValue(SETTING_PARAMS_GUARDRIGHTALL, configData.guardRightAll);
    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    configIniWrite->setValue(SETTING_PARAMS_DISABLELOGCOMMON, configData.disableLogCommon);
    configIniWrite->setValue(SETTING_PARAMS_TIMER4REBOOT, configData.reboot4timer);

    configIniWrite->setValue(SETTING_SERVER_SERVERIP,configData.serverIP);
    configIniWrite->setValue(SETTING_SERVER_SERVERPORT, configData.serverPort);
    configIniWrite->setValue(SETTING_SERVER_CONNECTTYPE, configData.ConnectType);
    configIniWrite->setValue(SETTING_SERVER808_NUMBER808ID, configData.number808Id);
    configIniWrite->setValue(SETTING_SERVER808_SERVERIP, configData.server808IP);
    configIniWrite->setValue(SETTING_SERVER808_SERVERPORT, configData.server808Port);

    delete configIniWrite;
}

void UserInfoFile::checkDisableGuardList()
{
    if(configData.organizations.size()<20){
        configData.organizations.clear();
        configData.organizations << "FF0" << "FF1" << "F0F" << "F00" << "F01" << "F1F" << "F11" << "0F0" << "000" << "001" << "010" << "011" << "100" << "101" << "111" << "FFF" << "FFF" << "FFF" << "FFF" << "FFF";
        QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
        configIniWrite->setValue(SETTING_PARAMS_ORGANIZATIONS, configData.organizations);
        delete configIniWrite;
    }
    if(configData.disabledGuardIds.size()==0){
        configData.disabledGuardIds << "F0FFFFF1" << "FFFFFFFF";
        QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
        configIniWrite->setValue(SETTING_PARAMS_DISABLEDGUARDIDS, configData.disabledGuardIds);
        delete configIniWrite;
    }
}

void UserInfoFile::resetConfigDataFromUserInfo()
{
    configData.valid = 1;
    QSettings *configIniRead = new QSettings(USERINFOFILENAME, QSettings::IniFormat);
    configIniRead->setIniCodec(QTextCodec::codecForName("utf-8"));
    configData.roadname = QString::fromUtf8(configIniRead->value("/LOCAL_MACHINE/Roadname").toByteArray());
    //configData.roadname = configIniRead->value("/LOCAL_MACHINE/Roadname").toString();
    configData.serverIP = "117.158.154.17";//configIniRead->value("/USER_EXE/ServerIPRem").toString();
    configData.localPort = configIniRead->value("/USER_EXE/LocalPort").toInt();
    configData.annuniatorPort = 5801;
    configData.ntpServer = "120.25.115.20";
    configData.diableWachdog = 0;
    configData.disableSyncTimeLoc = 1;
    configData.guardRightAll = 1;
    configData.organization = "0FF0";
    configData.rebooting4G = DEFAULTVALLUE_4GREBOOTING;
    configData.rebootingPlatform = DEFAULTVALUE_PLATFORMREBOOOTING;
    configData.rebooting808 = DEFAULTVALUE_808REBOOTING;
    configData.disableLogCommon = 0;
    configData.reboot4timer = -1;
    //configData.organizations << "FF0" << "FF1" << "F0F" << "F00" << "F01" << "F1F" << "F11" << "FFF" << "FFF" << "FFF";
    configData.organizations << "FF0" << "FF1" << "F0F" << "F00" << "F01" << "F1F" << "F11" << "0F0" << "000" << "001" << "010" << "011" << "100" << "101" << "111" << "FFF" << "FFF" << "FFF" << "FFF" << "FFF";
    configData.disabledGuardIds << "F0FFFFF1" << "FFFFFFFF";
    configData.serverPort = 8125;//configIniRead->value("/USER_EXE/ServerPort").toInt();
    configData.ConnectType = configIniRead->value("/USER_EXE/ConnectionType").toInt();
    configData.number808Id = configIniRead->value("/USER_EXE/Number808ID").toString();
    configData.server808IP = configIniRead->value("/USER_EXE/Server808IP").toString();
    configData.server808Port = configIniRead->value("/USER_EXE/Server808Port").toInt();
    //configData.organization = configIniRead->value("/USER_EXE/Organization").toString();
    delete configIniRead;
    logworker.addLogger(configData.roadname, LOGTYPE_PRINT);
    QSettings *configIniWrite = new QSettings(CONFIGFILENAME, QSettings::IniFormat);
    configIniWrite->setIniCodec(QTextCodec::codecForName("utf-8"));
    configIniWrite->setValue(SETTING_PARAMS_VALID,configData.valid);
    configIniWrite->setValue(SETTING_PARAMS_ROADNAME, configData.roadname);
    configIniWrite->setValue(SETTING_PARAMS_LOCALPORT, configData.localPort);
    configIniWrite->setValue(SETTING_PARAMS_ANNUNIATORPORT, configData.annuniatorPort);
    configIniWrite->setValue(SETTING_PARAMS_ORGANIZATION, configData.organization);
    configIniWrite->setValue(SETTING_PARAMS_ORGANIZATIONS, configData.organizations);
    configIniWrite->setValue(SETTING_PARAMS_DISABLEDGUARDIDS, configData.disabledGuardIds);
    configIniWrite->setValue(SETTING_PARAMS_NTPSERVER, configData.ntpServer);
    configIniWrite->setValue(SETTING_PARAMS_DISWATCHDOG, configData.diableWachdog);
    configIniWrite->setValue(SETTING_PARAMS_DISSYNCTIMELOC, configData.disableSyncTimeLoc);
    configIniWrite->setValue(SETTING_PARAMS_GUARDRIGHTALL, configData.guardRightAll);
    configIniWrite->setValue(SETTING_PARAMS_4GREBOOTING, configData.rebooting4G);
    configIniWrite->setValue(SETTING_PARAMS_PLATFORMREBOOTING, configData.rebootingPlatform);
    configIniWrite->setValue(SETTING_PARAMS_808BOOTING, configData.rebooting808);
    configIniWrite->setValue(SETTING_SERVER_SERVERIP,configData.serverIP);
    configIniWrite->setValue(SETTING_SERVER_SERVERPORT, configData.serverPort);
    configIniWrite->setValue(SETTING_SERVER_CONNECTTYPE, configData.ConnectType);
    configIniWrite->setValue(SETTING_SERVER808_NUMBER808ID, configData.number808Id);
    configIniWrite->setValue(SETTING_SERVER808_SERVERIP, configData.server808IP);
    configIniWrite->setValue(SETTING_SERVER808_SERVERPORT, configData.server808Port);
    configIniWrite->setValue(SETTING_PARAMS_DISABLELOGCOMMON, configData.disableLogCommon);
    configIniWrite->setValue(SETTING_PARAMS_TIMER4REBOOT, configData.reboot4timer);
    delete configIniWrite;
}


