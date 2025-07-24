#include <QCoreApplication>
#include <QMetaType>
#include <signal.h>
#include <QNetworkInterface>
#include "Log/loggerworker.h"
#include "Protocol/protocolfor4gserver.h"
#include "Protocol/protocolannuniator4u.h"
#include "Protocol/protocol808platform.h"
#include "Configfile/userinfofile.h"
#include "Driver/yiyuan_ec20_4gadapter.h"
#include "Annuniator/tcpserver4annuniator.h"
#include "Annuniator/manageannuniatorconnections.h"
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
// Version 3.10.01 - add reconnecting and reboot every 15 minutes if disconnected
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
void diagnoseTcpServerIssues()
{
    logworker.addLogger("=== TCP Server Diagnostic Start ===", LOGTYPE_PRINT);

    // 1. 检查服务器状态
    int serverStatus = gtcpServer4Platfrom->getStatus();
    logworker.addLogger(QString("Server Status: %1").arg(serverStatus), LOGTYPE_PRINT);

    // 2. 检查是否停止新连接
    bool stopNew = gtcpServer4Platfrom->isStopNewEnabled();
    logworker.addLogger(QString("Stop New Connections: %1").arg(stopNew ? "YES" : "NO"), LOGTYPE_PRINT);

    // 3. 检查连接数
    int connectionCount = gtcpServer4Platfrom->getConnectionCount();
    logworker.addLogger(QString("Active Connections: %1").arg(connectionCount), LOGTYPE_PRINT);

    // 4. 记录网络接口
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            logworker.addLogger(QString("Network Interface: %1").arg(address.toString()), LOGTYPE_PRINT);
        }
    }

    // 5. 强制启用新连接
    gtcpServer4Platfrom->allowNewConnection();

    logworker.addLogger("=== TCP Server Diagnostic End ===", LOGTYPE_PRINT);
}

// 定期调用诊断函数
void setupDiagnosticTimer()
{
    QTimer *diagnosticTimer = new QTimer();
    QObject::connect(diagnosticTimer, &QTimer::timeout, diagnoseTcpServerIssues);
    diagnosticTimer->start(60000);  // 每分钟诊断一次
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<RemoteControlDataItem>("RemoteControlDataItem");
    // environment modification
    ignoreSIGPIPE();

    if(manager_AnnuniatorConnections==nullptr){
        manager_AnnuniatorConnections = new ManageAnnuniatorConnections(&a);
        manager_AnnuniatorConnections->initialize();
    }
    if(annuniatorTcpServer==nullptr) {
        annuniatorTcpServer = new TcpServer4Annuniator(&a);
        annuniatorTcpServer->initialize();
    }
    if(gtcpServer4Platfrom==nullptr) {
        gtcpServer4Platfrom = new TcpServer4Platform(&a);
        gtcpServer4Platfrom->initialize();
    }

    //
    logworker.start();
    //logworker.addLogger("SB4U_Advance_Esm6800E_v2.8.16r", LOGTYPE_PRINT_RECORD);
    logworker.addLogger("SB4U_Advance_Esm6800E_v3.10.03_20250724 - RECONNECTING", LOGTYPE_PRINT_RECORD);
    //gUserInfoFile.testReadFunction();
    gUserInfoFile.initUserInfoData();
    //gUserInfoFile.testWritefunction();

    // initial protocol & Status params
    logworker.setDisableLogCommon(gUserInfoFile.getDisableLogCommon());
    gProtocolFor4GServer.initProtocol4G(gUserInfoFile.getLocalIP());
    //gProtocolFor4GServer.initProtocol4G("192.168.2.253");
    gProtocolAnnuniator4U.initProtocol4U();
    gProtocol808Platform.initSetPhoneNum(gUserInfoFile.get808PhoneId());
    gAnnuniatorStatus.initAnnuniatorStatusParams();
    // Connected to Annuniator

    // 改进的服务器启动代码
    logworker.addLogger("Starting Annuniator TCP Server...", LOGTYPE_PRINT);

    // 连接服务器状态变化信号，用于监控
    QObject::connect(annuniatorTcpServer, &TcpServer4Annuniator::serverStatusChanged,
                     [](int status) {
        if (status == 1) {
            logworker.addLogger("Annuniator TCP Server started successfully", LOGTYPE_PRINT);
        } else {
            logworker.addLogger("Annuniator TCP Server stopped or failed", LOGTYPE_PRINT);
        }
    });

    // 连接新连接建立信号
    QObject::connect(annuniatorTcpServer, &TcpServer4Annuniator::newConnectionEstablished,
                     [](const QString& clientIP) {
        logworker.addLogger(QString("New Annuniator client connected: %1").arg(clientIP), LOGTYPE_PRINT);
    });

    // 连接验证失败信号
    QObject::connect(annuniatorTcpServer, &TcpServer4Annuniator::connectionValidationFailed,
                     [](const QString& clientIP) {
        logworker.addLogger(QString("Connection validation failed for: %1").arg(clientIP), LOGTYPE_PRINT);
    });

    // 连接管理器状态监控
    QObject::connect(manager_AnnuniatorConnections, &ManageAnnuniatorConnections::connectionCountChanged,
                     [](int count) {
        logworker.addLogger(QString("Active Annuniator connections: %1").arg(count), LOGTYPE_PRINT);
    });

    QObject::connect(manager_AnnuniatorConnections, &ManageAnnuniatorConnections::connectionAdded,
                     [](const QString& ip) {
        logworker.addLogger(QString("Annuniator connection added: %1").arg(ip), LOGTYPE_PRINT_RECORD);
    });

    QObject::connect(manager_AnnuniatorConnections, &ManageAnnuniatorConnections::connectionRemoved,
                     [](const QString& ip) {
        logworker.addLogger(QString("Annuniator connection removed: %1").arg(ip), LOGTYPE_PRINT_RECORD);
    });
    // 启动服务器 - 使用配置文件中的端口或默认端口
    int serverPort = gUserInfoFile.getAnnuniatorServerPort();
    if (serverPort <= 0) {
        serverPort = 5801;  // 默认端口
        logworker.addLogger("Using default Annuniator server port: 5801", LOGTYPE_PRINT);
    }

    int startResult = annuniatorTcpServer->start(serverPort);
    if (startResult == 1) {
        logworker.addLogger(QString("Annuniator TCP Server listening on port %1").arg(serverPort), LOGTYPE_PRINT);
    } else {
        logworker.addLogger(QString("Failed to start Annuniator TCP Server on port %1").arg(serverPort), LOGTYPE_PRINT);
        // 可以选择退出程序或尝试其他端口
    }

    // Connected to Platform
    //gPlatformTcpv2.start(gUserInfoFile.get4GServerIP(), gUserInfoFile.get4GServerPort(), gUserInfoFile.getAnnuniatorIP());
    //gPlatformTcpv2.start("192.168.2.137", 8125);
    gPlatformTcpv2.start(gUserInfoFile.get4GServerIP(), gUserInfoFile.get4GServerPort());
    gPlatform808Tcpv2.start(gUserInfoFile.get808ServerIP(), gUserInfoFile.get808ServerPort());
    gtcpServer4Platfrom->start(gUserInfoFile.getLocalPort());


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
    setupDiagnosticTimer();
    return a.exec();
}
