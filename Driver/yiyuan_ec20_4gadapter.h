#ifndef YIYUAN_EC20_4GADAPTER_H
#define YIYUAN_EC20_4GADAPTER_H

#include <QObject>
#define EC20_CONN_STATUS_FAILED     0
#define EC20_CONN_STATUS_SUCCESS	1
#define EC20_CONN_STATUS_UNLOADDRV	2
#define EC20_CONN_STATUS_LOADDRV	3
#define EC20_CONN_STATUS_DIAL       4
#define EC20_CONN_STATUS_ROUTE      5
#define EC20_CONN_STATUS_PPPIDCHK	6
#define EC20_CONN_STATUS_PPPIDNONE	7
#define EC20_CONN_STATUS_IPCHK      19

#define ME909_VENDORID	"0x12d1"
#define ME909_PRODUCTID	"0x15c1"
#define EC20_VERDORID	"0x2c7c"
#define EC20_PRODUCTID	"0x0125"
#define USBSERIAL_CMD	"usbserial"
#define PPP0_CMD	"ppp0"
#define INSMOD_CMD	"insmod /lib/modules/4.1.15-g4f1387e/kernel/drivers/usb/serial/usbserial.ko vendor=%s product=%s"
#define MODPROBE_CMD	"modprobe usbserial vendor=%s product=%s"
#define LSMOD_CMD	"lsmod"
#define RMMOD_CMD	"rmmod %s"
#define DIAL4G_CMD	"pppd call lte-connect-script"
#define DIAL3G_CMD	"pppd call wcdma-connect-script"
#define ROUTE_CMD	"route add default gw %s"
#define ROUTE_CMD_FOR_MULTINETWORKS	"ip route add default via %s dev ppp0 metric 100"
#define ROUTE_CMD_FOR_SPECIALIPS	"route add -net %s netmask 255.255.255.0 gw %s"
#define INET_ADDR_CMD "ip addr show | grep \"inet\""

class YiYuan_EC20_4GAdapter : public QObject
{
    Q_OBJECT
public:
    explicit YiYuan_EC20_4GAdapter(QObject *parent = nullptr);
    ~YiYuan_EC20_4GAdapter();

    void Connect4G();
    void Connect4GWithoutDial();
    void Connect3G();
    void KeepAlive();
    int getConnectStatus(){return conn_status;};
    int getConnectStatusPPPid();
    int getKeepAliveStatus(){return alive_status;}
    int getIP4Conn(char* ip, int maxlen);
    int getGW4Conn(char* gw, int maxlen);
    int setRouteIps(char* ips, int maxlen);
    int setRouteIps2(char* ips, int maxlen);
    int setRouteIpNtp(char* ips, int maxlen);
    void clearAliveStatus(void){alive_status=0;}
    void Disconnect4G();
signals:

protected:
    int UnloadDrv();
    int LoadDrv();
    int ChkDrv();
    int Dial();
    int Dial3G();
    int ChkPPP0pid();
    int Route();
    int Route2SpecialIps(char* ip);
    int getIP();

    int findIP(char* source, const char* begin, char* ips, int len);

    int systemWithRet(char* cmdstring, char* buf, int len, const char* chkstr);
    int	FileIsExist( char* FileName );

    int checkNetwork();
    int checkConnection();
    int checkModule();

    int flow_DrvChk(int retrynum);
    int flow_DialChk(int retrynum);
    int flow_Failed();
private:
    int conn_status;
    int alive_status;
    char ipAddr[20], gwAddr[20], aimIps[20], aimIps2[20], aimIpNtp[20];
};
extern YiYuan_EC20_4GAdapter gYiYuanEC20_4GAdapter;
#endif // YIYUAN_EC20_4GADAPTER_H
