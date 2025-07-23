#ifndef WORKER_FORGPIO_H
#define WORKER_FORGPIO_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QString>
#include "Driver/driver_gpio.h"

#define POWERSTATUS_PIN         GPIO0
#define POWERCONTROL_PIN        GPIO1
#define REMOTE315MSTATUS_PIN    GPIO12
#define POWER4GCONTROL_PIN      GPIO23
// All Operation to GPIO must be set in this class

class Worker_ForGPIO : public QObject
{
    Q_OBJECT
public:
    explicit Worker_ForGPIO(QObject *parent = nullptr);

    void initParams4GPIOWorker();
signals:
    // signal for power status
    void sig_PowerOn();
    void sig_PowerOff();
    void sig_4gReseted();
    // signal for remote 315M
public slots:
    void initWorker();
    // contorl 4g power
    void open_4gPower();
    void reset_4gPower();
    // control battery power
    void action_BatteryOn();
    void action_BatteryOff();
    void action_RebootDevice();
protected:
    void initGPIO();
    void config();
    int close_batteryPower();
    int open_batteryPower();
    int close_4gPower();

    int reset_4gpower();
    void checkPowerStatus(unsigned int inputStatus);

    //protected:
    int read_gpio(int pin);
    int write_gpio(int pin, int state);

    int set2Output(int pin);
    int set2Input(int pin);

    int GPIO_OutSet(int fd, unsigned int dwSetBits);
    int GPIO_OutClear(int fd, unsigned int dwClearBits);
    int GPIO_OutDisable(int fd, unsigned int dwDisBits);
    int GPIO_PinState(int fd, unsigned int* pPinState);

    int openWatchDog(int secs);
    void feedWatchDog();
    int closeWatchDog();

    void Delay_MSec(int msec);
private:
    bool beValid, beReboot, watchdogDisabled;
    bool inProcessing_ResetPower4G;
    int gpio_fd;
    int fd_watchdog_global=0;
};

class Controller_ForGPIO : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller_ForGPIO();
    ~Controller_ForGPIO();

    void startGPIOWorker();
    void stopWorker();
    void actionBatteryOff();
    void actionRebootDev();
    void actionReboot4GModule();
public slots:

signals:
    void startWorker();
    void sigBatteryOff();
private:
    Worker_ForGPIO* worker=nullptr;

};
extern Controller_ForGPIO controllerGPIO;

#endif // WORKER_FORGPIO_H
