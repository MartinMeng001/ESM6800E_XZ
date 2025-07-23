#include <QCoreApplication>
#include <QMetaType>
#include <signal.h>
#include "Log/loggerworker.h"
#include "Protocol/protocolfor4gserver.h"
#include "Protocol/protocolannuniator4u.h"
#include "Protocol/protocol808platform.h"
#include "Configfile/userinfofile.h"
#include "Driver/yiyuan_ec20_4gadapter.h"
#include "Annuniator/tcpserver4annuniator.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "Platform4G/platformtcp.h"
#include "Platform4G/platformtcpv2.h"
#include "Platform808/platform808tcpv2.h"
#include "PlatformAsClient/tcpserver4platform.h"
#include "Worker/worker_fromannuniator.h"
#include "Worker/worker_fromplatform.h"
#include "Worker/worker_forgpio.h"
#include "Worker/worker_remotecontrol.h"
#include "Worker/worker_to808platform.h"
#include "Worker/worker_fromplatformclient.h"
#include "Worker/worker_annuniatorstatus.h"
#include "Worker/worker_4g.h"

// Version 2.2 - add the 808 network support
// Version 2.3 - add 4G, done
//             - fixed poweroff check function;
//             - fixed routed function
// Version 2.4 - add the network server mode support
// Version 2.4.1 - add time correct
// Version 2.5 - add flow usage sumarization
// Version 2.6 - add watchdog function, pending
// Version 2.7 - pre-release
// Version 2.7.1 - add 10 regions, add log day consumer to log file
// Version 2.7.2 - modify the regions default code
//         2.7.26- modify the default config params - server (117.158.154.17), port(8125)
//         2.7.27- fixed a battery always on problem when power off
//         2.7.28- fixed remote guard key5 don't report to platform
//         2.7.29- added 4G status manage
//         2.7.30- fixed a tcp client reconnect problem, achieve 4G status manage
//         2.7.33- disable common log try to improve the speed for remote guard
//         2.7.47- fixed remote guard loss occassionly problem, fixed 4G status manage problem
//         2.7.49- remove a unused log
//         2.7.59- added disabled ids function, maximum 15
//         2.7.63- added to 20 Remote Guard fields
// Version 2.8 - add 4G library support, use library to manage the 4G - give up the function
//         2.8.4 - send the 808 info after 4G connected, if 4G don't connect, don't send event and don't trigger heartbeat
// Version 2.9 - pending version, not necessary, used A0 to report all trouble, not used A1 again - give up, A0 isn't enough
// Version 2.10 - pending version, add extension protocol supporting, allow to execute action by external application
// Version 3.1 - remove 4G manage, used the emtronix 4G driver do 4G connection, don't process the route too
// Version 3.2 - New TcpClient version
// Version 3.2.08 - a trouble alarm will repeat 3 times
// Version 3.3.03 - No data from server over 3 minutes, reconnected to server4G
void handle_pipe(int sig){
    // do nothing
}
void ignoreSIGPIPE(){
    struct sigaction sa;
    sa.sa_handler = handle_pipe;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE,&sa,NULL);
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<RemoteControlDataItem>("RemoteControlDataItem");
    // environment modification
    ignoreSIGPIPE();
    //
    logworker.start();
    //logworker.addLogger("SB4U_Advance_Esm6800E_v2.8.16r", LOGTYPE_PRINT_RECORD);
    logworker.addLogger("SB4U_Advance_Esm6800E_v3.3.03_20240315_02 - 4G driver", LOGTYPE_PRINT_RECORD);
    //gUserInfoFile.testReadFunction();
    gUserInfoFile.initUserInfoData();
    //gUserInfoFile.testWritefunction();

    // initial protocol & Status params
    logworker.setDisableLogCommon(gUserInfoFile.getDisableLogCommon());
    //gProtocolFor4GServer.initProtocol4G(gUserInfoFile.getLocalIP());
    gProtocolFor4GServer.initProtocol4G("192.168.2.253");
    gProtocolAnnuniator4U.initProtocol4U();
    gProtocol808Platform.initSetPhoneNum(gUserInfoFile.get808PhoneId());
    gAnnuniatorStatus.initAnnuniatorStatusParams();
    // Connected to Annuniator
    annuniatorTcpServer.start(gUserInfoFile.getAnnuniatorServerPort());
    // Connected to 4G - remove these feature
//    gYiYuanEC20_4GAdapter.setRouteIps2(gUserInfoFile.get4GServerIP().toLatin1().data(), gUserInfoFile.get4GServerIP().length());
//    gYiYuanEC20_4GAdapter.setRouteIps(gUserInfoFile.get808ServerIP().toLatin1().data(), gUserInfoFile.get808ServerIP().length());
//    gYiYuanEC20_4GAdapter.setRouteIpNtp(gUserInfoFile.getNtpServer().toLatin1().data(), gUserInfoFile.getNtpServer().length());
//    gYiYuanEC20_4GAdapter.Connect4G();

    // Connected to Platform
    //gPlatformTcpv2.start(gUserInfoFile.get4GServerIP(), gUserInfoFile.get4GServerPort(), gUserInfoFile.getAnnuniatorIP());
    gPlatformTcpv2.start("192.168.2.137", 8125);
    //gPlatformTcpv2.start(gUserInfoFile.get4GServerIP(), gUserInfoFile.get4GServerPort());
    gPlatform808Tcpv2.start(gUserInfoFile.get808ServerIP(), gUserInfoFile.get808ServerPort());
    gtcpServer4Platfrom.start(gUserInfoFile.getLocalPort());

    // Start Worker
    controllerGPIO.startGPIOWorker();
    //controller4G.start4GWorker(); // reboot device when 4G don't connect
    annuniatorController.startSendWorker();
    controllerPlatform.startPlatformWorker();    
    controllerRemoteControl.startRemoteControlWorker();
    controller808Platform.startPlatformWorker();
    platformClientController.startSendWorker();
    //annuniatorControllerStatus.startStatusWorker(); // No function now

//    if(gYiYuanEC20_4GAdapter.getConnectStatus()==EC20_CONN_STATUS_FAILED){
//        QThread::sleep(10);
//        controllerGPIO.actionReboot4GModule();
//    }

    return a.exec();
}
