#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include "Log/loggerworker.h"
#include "yiyuan_ec20_4gadapter.h"

YiYuan_EC20_4GAdapter gYiYuanEC20_4GAdapter;
YiYuan_EC20_4GAdapter::YiYuan_EC20_4GAdapter(QObject *parent) : QObject(parent)
{
    conn_status=EC20_CONN_STATUS_FAILED;
    alive_status=0;

    memset(ipAddr,0,sizeof(ipAddr));
    memset(gwAddr, 0, sizeof(gwAddr));
    memset(aimIps, 0, sizeof(aimIps));
    memset(aimIps2, 0, sizeof(aimIps2));
    memset(aimIpNtp, 0, sizeof(aimIpNtp));
}

YiYuan_EC20_4GAdapter::~YiYuan_EC20_4GAdapter()
{

}

void YiYuan_EC20_4GAdapter::Connect4G()
{
    conn_status=EC20_CONN_STATUS_FAILED;
    if(checkNetwork()==1)
    {
        conn_status=EC20_CONN_STATUS_SUCCESS;
        return;
    }
    logworker.addLogger("network check failed, check Connection", LOGTYPE_PRINT);
    if(checkConnection()==1)
    {
        conn_status=EC20_CONN_STATUS_SUCCESS;
        return;
    }
    logworker.addLogger("Connection check failed, check Module", LOGTYPE_PRINT);
    if(checkModule()==1)
    {
        conn_status=EC20_CONN_STATUS_SUCCESS;
        return;
    }
    conn_status=EC20_CONN_STATUS_FAILED;
    logworker.addLogger("Module check failed, Quit", LOGTYPE_PRINT);
}

void YiYuan_EC20_4GAdapter::Connect4GWithoutDial()
{
    if(conn_status==EC20_CONN_STATUS_SUCCESS) return;
    //conn_status=EC20_CONN_STATUS_FAILED;
    if(checkNetwork()==1)
    {
        conn_status=EC20_CONN_STATUS_SUCCESS;
        return;
    }
    logworker.addLogger("4G network check failed", LOGTYPE_PRINT);
    conn_status=EC20_CONN_STATUS_FAILED;
    //logworker.addLogger("Module check failed, Quit", LOGTYPE_PRINT);
}

void YiYuan_EC20_4GAdapter::Connect3G()
{

}

void YiYuan_EC20_4GAdapter::KeepAlive()
{

}

int YiYuan_EC20_4GAdapter::getConnectStatusPPPid()
{
    //return checkNetwork();
    if(checkNetwork()==1) return 1;
    return 0;
}

int YiYuan_EC20_4GAdapter::getIP4Conn(char *ip, int maxlen)
{
    if(conn_status!=EC20_CONN_STATUS_SUCCESS) return 0;
    if(maxlen<(int)strlen(ipAddr)) return 0;
    strcpy(ip, ipAddr);
    return 1;
}

int YiYuan_EC20_4GAdapter::getGW4Conn(char *gw, int maxlen)
{
    if(conn_status!=EC20_CONN_STATUS_SUCCESS) return 0;
    if(maxlen<(int)strlen(gwAddr)) return 0;
    strcpy(gw, gwAddr);
    return 1;
}

int YiYuan_EC20_4GAdapter::setRouteIps(char *ips, int maxlen)
{
    if(maxlen>(int)sizeof(aimIps)) return 0;
    int length=strlen(ips);
    if(length>maxlen) return 0;
    memset(aimIps, 0, sizeof(aimIps));
    int breakcount=3;
    for(int i=0;i<length;i++)
    {
        aimIps[i]=ips[i];
        if(ips[i]=='.')breakcount--;
        if(breakcount==0)
        {
            aimIps[i+1]='0';
            break;
        }
    }
    return 1;
}

int YiYuan_EC20_4GAdapter::setRouteIps2(char *ips, int maxlen)
{
    if(maxlen>(int)sizeof(aimIps2)) return 0;
    int length=strlen(ips);
    if(length>maxlen) return 0;
    memset(aimIps2, 0, sizeof(aimIps2));
    int breakcount=3;
    for(int i=0;i<length;i++)
    {
        aimIps2[i]=ips[i];
        if(ips[i]=='.')breakcount--;
        if(breakcount==0)
        {
            aimIps2[i+1]='0';
            break;
        }
    }
    return 1;
}

int YiYuan_EC20_4GAdapter::setRouteIpNtp(char *ips, int maxlen)
{
    if(maxlen>(int)sizeof(aimIpNtp)) return 0;
    int length=strlen(ips);
    if(length>maxlen) return 0;
    memset(aimIpNtp, 0, sizeof(aimIpNtp));
    int breakcount=3;
    for(int i=0;i<length;i++)
    {
        aimIpNtp[i]=ips[i];
        if(ips[i]=='.')breakcount--;
        if(breakcount==0)
        {
            aimIpNtp[i+1]='0';
            break;
        }
    }
    return 1;
}

void YiYuan_EC20_4GAdapter::Disconnect4G()
{
    conn_status=EC20_CONN_STATUS_FAILED;
}

int YiYuan_EC20_4GAdapter::UnloadDrv()
{
    char cmdstr[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    sprintf(cmdstr, RMMOD_CMD, USBSERIAL_CMD);
    system(cmdstr);
    conn_status=EC20_CONN_STATUS_UNLOADDRV;
    return 0;
}

int YiYuan_EC20_4GAdapter::LoadDrv()
{
    char cmdstr[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    sprintf(cmdstr, MODPROBE_CMD, EC20_VERDORID, EC20_PRODUCTID);
    system(cmdstr);
    conn_status=EC20_CONN_STATUS_LOADDRV;
    return 0;
}

int YiYuan_EC20_4GAdapter::ChkDrv()
{
    char cmdstr[10], retbuf[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    memset(retbuf, 0, sizeof(retbuf));
    sprintf(cmdstr, LSMOD_CMD);

    int status=systemWithRet(cmdstr, retbuf, sizeof(retbuf), USBSERIAL_CMD);
    if(status==1)
    {
        return 1;
    }
    return 0;
}

int YiYuan_EC20_4GAdapter::Dial()
{
    char cmdstr[50];
    memset(cmdstr, 0, sizeof(cmdstr));
    sprintf(cmdstr, DIAL4G_CMD);
    system(cmdstr);
    conn_status=EC20_CONN_STATUS_DIAL;
    return 0;
}

int YiYuan_EC20_4GAdapter::Dial3G()
{
    char cmdstr[50];
    memset(cmdstr, 0, sizeof(cmdstr));
    sprintf(cmdstr, DIAL3G_CMD);
    system(cmdstr);
    conn_status=EC20_CONN_STATUS_DIAL;
    return 0;
}

int YiYuan_EC20_4GAdapter::ChkPPP0pid()
{
    char FileName[40];
    memset(FileName, 0, sizeof(FileName));
    strcpy(FileName, "/var/run/ppp0.pid");
    //conn_status++;
    if(FileIsExist(FileName)==1) return 1;
    //conn_status=CONN_STATUS_PPPIDNONE;
    return 0;
}

int YiYuan_EC20_4GAdapter::Route()
{
    char cmdstr[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    //sprintf(cmdstr, ROUTE_CMD, gwAddr);
    if(strlen(aimIps)==0)sprintf(cmdstr, ROUTE_CMD_FOR_MULTINETWORKS, gwAddr);
    else sprintf(cmdstr, ROUTE_CMD_FOR_SPECIALIPS, aimIps, gwAddr);
    int status = system(cmdstr);
    if(status<0)
    {
        printf("cmd:%s\t error:%s", cmdstr, strerror(errno));
    }
    else
    {
        if(WIFEXITED(status))
        {
            printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); //取得cmdstring执行结果
        }
        else if(WIFSIGNALED(status))
        {
            printf("abnormal termination,signal number =%d\n", WTERMSIG(status));
        }
        else if(WIFSTOPPED(status))
        {
            printf("process stopped, signal number =%d\n", WSTOPSIG(status));
        }
        alive_status=1;
    }
    conn_status=EC20_CONN_STATUS_ROUTE;
    return 0;
}

int YiYuan_EC20_4GAdapter::Route2SpecialIps(char *ip)
{
    char cmdstr[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    //sprintf(cmdstr, ROUTE_CMD, gwAddr);
    sprintf(cmdstr, ROUTE_CMD_FOR_SPECIALIPS, ip, gwAddr);
    int status = system(cmdstr);
    std::cout << "route:" << cmdstr << std::endl;
    if(status<0)
    {
        printf("cmd:%s\t error:%s", cmdstr, strerror(errno));
    }
    else
    {
        if(WIFEXITED(status))
        {
            printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); //取得cmdstring执行结果
        }
        else if(WIFSIGNALED(status))
        {
            printf("abnormal termination,signal number =%d\n", WTERMSIG(status));
        }
        else if(WIFSTOPPED(status))
        {
            printf("process stopped, signal number =%d\n", WSTOPSIG(status));
        }
        alive_status=1;
    }
    conn_status=EC20_CONN_STATUS_ROUTE;
    return 0;
}

int YiYuan_EC20_4GAdapter::getIP()
{
    char ip[20],gw[20];
    char cmdstr[100], retbuf[200];
    memset(cmdstr, 0, sizeof(cmdstr));
    memset(retbuf, 0, sizeof(retbuf));
    memset(ipAddr, 0, sizeof(ipAddr));
    memset(gwAddr, 0, sizeof(gwAddr));

    sprintf(cmdstr, INET_ADDR_CMD);
    conn_status=EC20_CONN_STATUS_IPCHK;
    int status=systemWithRet(cmdstr, retbuf, sizeof(retbuf), PPP0_CMD);
    if(status==1)
    {
        //conn_status++;
        //printf("ipbuf=%s\r\n", retbuf);
        if(findIP(retbuf, "inet", ip, sizeof(ip))==1)
        {
            //conn_status++;
            strcpy(ipAddr, ip);
            if(strlen(ip)<5) return 0;
            if(findIP(retbuf, "peer", gw, sizeof(gw))==1)
            {
                strcpy(gwAddr, gw);
                //printf("ip:%s,gw:%s\r\n", ip, gw);
                return 1;
            }
        }
    }
    return 0;
}

int YiYuan_EC20_4GAdapter::findIP(char *source, const char *begin, char *ips, int len)
{
    int ret=0;
    memset(ips, 0, len);
    if(len<16) return 0;
    char* p=strstr(source, begin);
    if(p==NULL) return 0;
    int index=0;
    for(int i=0;i<(int)strlen(p);i++)
    {
        if(index>0)
        {
            if(p[i]==0x20 || p[i]=='/')
            {
                ret=1;
                break;	// end
            }
            else
            {
                ips[index]=p[i];
                index++;
            }
        }
        else
        {
            if(p[i]==0x20)	// begin
            {
                ips[index]=p[i];
                index++;
            }
        }
    }
    return ret;
}

int YiYuan_EC20_4GAdapter::systemWithRet(char *cmdstring, char *buf, int len, const char *chkstr)
{
    int ret=0;
    FILE *fstream=NULL;
    if(NULL==(fstream=popen(cmdstring, "r")))
    {
        printf("execute command failed:%s\r\n", cmdstring);
        return -1;
    }
    while(NULL!=fgets(buf, len, fstream))
    {
        printf("%s", buf);
        if(strstr(buf, chkstr)!=NULL)
        {
            ret=1;
            break;
        }
    }
    pclose(fstream);
    return ret;
}

int YiYuan_EC20_4GAdapter::FileIsExist(char *FileName)
{
    FILE *fp;
    fp=fopen(FileName, "rt");
    if(fp!=NULL)
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

int YiYuan_EC20_4GAdapter::checkNetwork()
{
    if(flow_DialChk(2)==0) return 0;
    if(getIP()==1)
    {
        //Route();
        if(strlen(aimIps)>0)Route2SpecialIps(aimIps);
        if(strlen(aimIps2)>0)Route2SpecialIps(aimIps2);
        if(strlen(aimIpNtp)>0)Route2SpecialIps(aimIpNtp);
        return 1;
    }
    return 0;
}

int YiYuan_EC20_4GAdapter::checkConnection()
{
    if(flow_DrvChk(2)==0)
    {
        LoadDrv();sleep(1);
        if(flow_DrvChk(3)==0) return 0;
    }
    printf("Dial 4G\r\n");
    Dial();
    if(flow_DialChk(20)==0) return 0;
    sleep(10);	// wait 10s to build the connection
    return checkNetwork();
}

int YiYuan_EC20_4GAdapter::checkModule()
{
    //if(powerResetMe909()==0) return 0;
    //sleep(10);
    return checkConnection();
}

int YiYuan_EC20_4GAdapter::flow_DrvChk(int retrynum)
{
    int ret=0;
    while(retrynum--)
    {
        if(ChkDrv()==1)
        {
            ret=1;
            break;
        }
        sleep(1);
    }
    return ret;
}

int YiYuan_EC20_4GAdapter::flow_DialChk(int retrynum)
{
    int ret=0;
    while(retrynum--)
    {
        if(ChkPPP0pid()==1)
        {
            printf("ppp0.pid finded\r\n");
            ret=1;
            break;
        }
        sleep(1);
    }
    return ret;
}

int YiYuan_EC20_4GAdapter::flow_Failed()
{
    conn_status=EC20_CONN_STATUS_FAILED;
    return 0;
}
