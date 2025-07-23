#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
//#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <QTimer>
#include <QEventLoop>
#include "Configfile/userinfofile.h"
#include "RemoteControl/pulse315m.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "Log/loggerworker.h"
#include "worker_forgpio.h"

#define	WATCHDOG_IOCTL_BASE	'W'

#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT	_IOWR(WATCHDOG_IOCTL_BASE, 6, int)

Controller_ForGPIO controllerGPIO;
Worker_ForGPIO::Worker_ForGPIO(QObject *parent) : QObject(parent)
{
    watchdogDisabled = false;
    beReboot = false;
    beValid = false;
    inProcessing_ResetPower4G = false;
    gpio_fd = 0;
}

void Worker_ForGPIO::initParams4GPIOWorker()
{
    if(gUserInfoFile.getDisableWatchDog()==1)watchdogDisabled=true;
    else watchdogDisabled=false;
    QString info = QString("Disable WatchDog = %1").arg(watchdogDisabled);
    logworker.addLogger(info, LOGTYPE_PRINT);
}
int Worker_ForGPIO::openWatchDog(int secs)
{
    if(watchdogDisabled) return 0;
    unsigned long timeout=secs;
    fd_watchdog_global=open("/dev/watchdog", O_RDONLY);
    if(fd_watchdog_global<0)
    {
        printf("Open Watch Dog failed\r\n");
        return 0;
    }
    ioctl(fd_watchdog_global, WDIOC_SETTIMEOUT, &timeout);
    return 1;
}
void Worker_ForGPIO::feedWatchDog()
{
    if(watchdogDisabled) return;
    if(fd_watchdog_global>0)
    {
        ioctl(fd_watchdog_global, WDIOC_KEEPALIVE, 0);
        //printf("feed watchdog\r\n");
    }
    else
    {
        printf("Watch Dog failed\r\n");
    }
}
int Worker_ForGPIO::closeWatchDog()
{
    if(watchdogDisabled) return 0;
    if(fd_watchdog_global>0) close(fd_watchdog_global);
    return 1;
}
void Worker_ForGPIO::initWorker()
{
    fd_set fdRead;
    int ret=0;
    unsigned int	dwPinState;
    struct timeval aTime;//访问时间：	文件中的数据库最后被访问的时间s
    prctl(PR_SET_NAME,"Worker_ForGPIO");
    //struct timeval usTime;//毫秒
    initGPIO();
    config();
    char onoff_315M=-1;
    bool changed = true;
    //long totaltime;
    while(1)
    {
        if(gUserInfoFile.beRebootTimer()){
            std::cout << "**** gUserInfoFile.beRebootTimer reboot=true" << std::endl;
            beReboot=true;
        }
        if(beReboot==false)feedWatchDog();
        if(gpio_fd==0){
            initGPIO();
            usleep(1000);
            logworker.addLogger("re-initGPIO", LOGTYPE_PRINT);
        }
        FD_ZERO(&fdRead);//&
        FD_SET(gpio_fd, &fdRead);
        // SET OVERTIME TIME
        //if(changed){
        aTime.tv_sec = 0;
        aTime.tv_usec = 200000;
        changed = false;
        //}
        ret = select(gpio_fd+1,&fdRead,NULL,NULL,&aTime);
        if (ret < 0 ){
            std::cout << "Worker_ForGPIO Remote select overtime" << std::endl;
            dwPinState = POWERSTATUS_PIN;
            int rc = GPIO_PinState(gpio_fd, &dwPinState);
            if(rc< 0){
                continue;
            }
            checkPowerStatus(dwPinState);
            //usleep(1000);
            continue;
        }
        else if (ret > 0){
            char onoff=0;
            //判断是否读事件
            if (FD_ISSET(gpio_fd,&fdRead)){
                dwPinState = REMOTE315MSTATUS_PIN|POWERSTATUS_PIN;
                int rc = GPIO_PinState(gpio_fd, &dwPinState);
                if(rc< 0){
                    //printf("GPIO_PinState::failed %d\n", rc);
                    continue;
                }
                //根据上升沿对脉冲计数
                if(dwPinState& REMOTE315MSTATUS_PIN){
                    onoff=1;
                    //std::cout << "1";
                }else{
                    //std::cout << "0";
                }
                gettimeofday(&aTime,NULL);
                checkPowerStatus(dwPinState);
//                pulse315M.setPulseItem2(onoff, aTime);
                if(onoff!=onoff_315M){
                    changed = true;
                    onoff_315M = onoff;
                    pulse315M.setPulseItem3(onoff_315M, aTime);
                }
            }
        }else{
            dwPinState = POWERSTATUS_PIN;
            int rc = GPIO_PinState(gpio_fd, &dwPinState);
            if(rc< 0){
                continue;
            }
            checkPowerStatus(dwPinState);
        }
        //usleep(1);
    }
}


void Worker_ForGPIO::initGPIO()
{
    gpio_fd = open("/dev/em_gpio", O_RDWR);

    // Set Input
    GPIO_OutDisable(gpio_fd, POWERSTATUS_PIN|REMOTE315MSTATUS_PIN);	//|GPIO10
    // Set Output
    set2Output(POWERCONTROL_PIN|POWER4GCONTROL_PIN);
    GPIO_OutClear(gpio_fd, POWERCONTROL_PIN);
    GPIO_OutSet(gpio_fd, POWER4GCONTROL_PIN);
    openWatchDog(30);
}

void Worker_ForGPIO::config()
{
    connect(this, &Worker_ForGPIO::sig_PowerOn, &gAnnuniatorStatus, &AnnuniatorStatus::Status_PowerOn);
    connect(this, &Worker_ForGPIO::sig_PowerOff, &gAnnuniatorStatus, &AnnuniatorStatus::Status_PowerOff);
    connect(this, &Worker_ForGPIO::sig_4gReseted, &gAnnuniatorStatus, &AnnuniatorStatus::Action_4GReseted);
    connect(&gAnnuniatorStatus, &AnnuniatorStatus::sig4GPowerReset, this, &Worker_ForGPIO::reset_4gPower);
    connect(&gAnnuniatorStatus, &AnnuniatorStatus::sigBatteryOn, this, &Worker_ForGPIO::action_BatteryOn);
    //connect(&gAnnuniatorStatus, &AnnuniatorStatus::sigBatteryOff, this, &Worker_ForGPIO::action_BatteryOff);
    action_BatteryOn();
}

int Worker_ForGPIO::open_batteryPower()
{
    if(gpio_fd==0)return 0;
    //set2Output(POWERCONTROL_PIN);
    GPIO_OutClear(gpio_fd, POWERCONTROL_PIN);
    return 1;
}

int Worker_ForGPIO::close_batteryPower()
{
    if(gpio_fd==0)return 0;
    logworker.addLogger("exec close_batteryPower", LOGTYPE_PRINT);
    //set2Output(POWERCONTROL_PIN);
    GPIO_OutSet(gpio_fd, POWERCONTROL_PIN);
    return 1;
}

int Worker_ForGPIO::close_4gPower()
{
    if(inProcessing_ResetPower4G)return 0;
    if(gpio_fd==0)return 0;
    GPIO_OutClear(gpio_fd, POWER4GCONTROL_PIN);
    return 1;
}

void Worker_ForGPIO::open_4gPower()
{
    inProcessing_ResetPower4G = false;
    if(gpio_fd==0)return;
    GPIO_OutSet(gpio_fd, POWER4GCONTROL_PIN);
    emit sig_4gReseted();
    logworker.addLogger("open_4gPower", LOGTYPE_PRINT);
}

void Worker_ForGPIO::reset_4gPower()
{
    logworker.addLogger("Worker_ForGPIO::reset_4gPower()", LOGTYPE_PRINT);
    reset_4gpower();
    //controllerGPIO.actionReboot4GModule();
}

void Worker_ForGPIO::action_BatteryOn()
{
    open_batteryPower();
    logworker.addLogger("exec action_BatteryON", LOGTYPE_PRINT);
}

void Worker_ForGPIO::action_BatteryOff()
{
    close_batteryPower();
    logworker.addLogger("exec action_BatteryOff", LOGTYPE_PRINT);
}

void Worker_ForGPIO::action_RebootDevice()
{
    beReboot = true;
}



int Worker_ForGPIO::reset_4gpower()
{
    if(close_4gPower()==0) return 0;
    logworker.addLogger("close_4gPower", LOGTYPE_PRINT);
    Delay_MSec(2000);
    open_4gPower();
    return 0;
}

void Worker_ForGPIO::checkPowerStatus(unsigned int inputStatus)
{
    if(inputStatus & POWERSTATUS_PIN){
        emit sig_PowerOn();
    }else{
        emit sig_PowerOff();
    }
}

int Worker_ForGPIO::read_gpio(int pin)
{
    int 				rc;
    struct double_pars	dpars;
    unsigned int dwPinState = 0xffffffff;
    dpars.par1 = ESM6800_GPIO_INPUT_STATE;	// 5
    dpars.par2 = dwPinState;

    rc = read(gpio_fd, &dpars, sizeof(struct double_pars));
    if(!rc)
    {
        dwPinState = dpars.par2;
        unsigned int pos=0x1;
        //if(((dwPinState>>pin) & pos)==0x1) return 1;
        if(((dwPinState & pin) & pos)==0x1) return 1;
        else return 0;
    }
    return -1;	// failed
}
int Worker_ForGPIO::write_gpio(int pin, int state)
{
    int 				rc;
    struct double_pars	dpars;
    unsigned int dwSetBits = 0x1<<pin;
    if(state==0)dwSetBits=0;
    dpars.par1 = ESM6800_GPIO_OUTPUT_SET;	// 2
    dpars.par2 = dwSetBits;

    rc = write(gpio_fd, &dpars, sizeof(struct double_pars));
    return rc;
}
int Worker_ForGPIO::set2Output(int pin)
{
    int 				rc;
    struct double_pars	dpars;
    //unsigned int dwEnBits=0x1<<pin;
    unsigned int dwEnBits=pin;
    dpars.par1 = ESM6800_GPIO_OUTPUT_ENABLE;		// 0
    dpars.par2 = dwEnBits;

    rc = write(gpio_fd, &dpars, sizeof(struct double_pars));
    return rc;
}
int Worker_ForGPIO::set2Input(int pin)
{
    int 				rc;
    struct double_pars	dpars;
    //unsigned int dwDisBits=0x1<<pin;
    unsigned int dwDisBits=pin;
    dpars.par1 = ESM6800_GPIO_OUTPUT_DISABLE;	// 1
    dpars.par2 = dwDisBits;

    rc = write(gpio_fd, &dpars, sizeof(struct double_pars));
    return rc;
}
int Worker_ForGPIO::GPIO_OutSet(int fd, unsigned int dwSetBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = ESM6800_GPIO_OUTPUT_SET;	// 2
    dpars.par2 = dwSetBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}

int Worker_ForGPIO::GPIO_OutClear(int fd, unsigned int dwClearBits)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = ESM6800_GPIO_OUTPUT_CLEAR;	// 3
    dpars.par2 = dwClearBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}
int Worker_ForGPIO::GPIO_OutDisable(int fd, unsigned int dwDisBits){
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = ESM6800_GPIO_OUTPUT_DISABLE;	// 1
    dpars.par2 = dwDisBits;

    rc = write(fd, &dpars, sizeof(struct double_pars));
    return rc;
}
int Worker_ForGPIO::GPIO_PinState(int fd, unsigned int* pPinState)
{
    int 				rc;
    struct double_pars	dpars;

    dpars.par1 = ESM6800_GPIO_INPUT_STATE;	// 5
    dpars.par2 = *pPinState;

    rc = read(fd, &dpars, sizeof(struct double_pars));
    if(!rc)
    {
        *pPinState = dpars.par2;
    }
    return rc;
}
void Worker_ForGPIO::Delay_MSec(int msec)
{
    if(inProcessing_ResetPower4G) return;
    inProcessing_ResetPower4G = true;
//    //QEventLoop loop;
//    QTimer::singleShot(msec, this, SLOT(open_4gPower()));
//    //loop.exec();
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
Controller_ForGPIO::Controller_ForGPIO()
{

}

Controller_ForGPIO::~Controller_ForGPIO()
{
    if(workerThread.isRunning()){
        workerThread.quit();
        //workerThread.wait();
    }
    logworker.addLogger("Controller_ForGPIO quit", LOGTYPE_PRINT);
}

void Controller_ForGPIO::startGPIOWorker()
{
    worker=new Worker_ForGPIO;
    worker->initParams4GPIOWorker();
    worker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(startWorker()), worker, SLOT(initWorker()));
    connect(this, SIGNAL(sigBatteryOff()), worker, SLOT(action_BatteryOff()));
    //connect(&gAnnuniatorStatus, &AnnuniatorStatus::sigBatteryOff, this, &Controller_ForGPIO::sigBatteryOff);
    //workerThread.setPriority(QThread::HighestPriority);
    workerThread.start();
    emit startWorker();
}

void Controller_ForGPIO::stopWorker()
{
    workerThread.exit();
}

void Controller_ForGPIO::actionBatteryOff()
{
    if(worker!=nullptr){
        worker->action_BatteryOff();
        //emit sigBatteryOff();
    }
}

void Controller_ForGPIO::actionRebootDev()
{
    if(worker!=nullptr){
        worker->action_RebootDevice();
        //emit sigBatteryOff();
    }
}

void Controller_ForGPIO::actionReboot4GModule()
{
    if(worker!=nullptr){
        worker->reset_4gPower();
        logworker.addLogger("reboot 4G module", LOGTYPE_PRINT);
        //emit sigBatteryOff();
    }
}
