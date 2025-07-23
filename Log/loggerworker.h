#ifndef LOGGERWORKER_H
#define LOGGERWORKER_H

#include <QThread>
#include <QDateTime>
#include <QList>
#include <QMutex>
#define LOGTYPE_PRINT           0
#define LOGTYPE_RECORD          1
#define LOGTYPE_PRINT_RECORD    2
struct logInfoItem{
    QString info;
    QDateTime infotime;
    int type;   // type = 0, print to serial, type = 1, record to file, type = 2,
};

class LoggerWorker : public QThread
{
    Q_OBJECT
public:
    explicit LoggerWorker(QObject *parent = nullptr);
    void addLogger(QString info, int type);
    void stopWorker();
    void setDisableLogCommon(int flag);
    // QThread interface
protected:
    void run();

    void clearAllBuffer();
    void recordOneLogger(logInfoItem* item);
    void printOneLogger(logInfoItem* item);
    void clearLogFile();
    logInfoItem *takeItem(int size);
private:
    QList<logInfoItem*> allPendingLoggers;
    QMutex mutexObj;
    bool stopThread;
    bool allowCommonLog;
};
extern LoggerWorker logworker;

#endif // LOGGERWORKER_H
