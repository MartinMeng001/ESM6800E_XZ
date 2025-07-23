#ifndef USAGE4GNETWORK_H
#define USAGE4GNETWORK_H

#include <QObject>
#include <QMutex>
#include <QDateTime>
#define SUPPORT_4GTYPE_PLATFORM4G       1
#define SUPPORT_4GTYPE_808PLATFORM      2
#define SUPPORT_4GTYPE_PLATFORCLIENT    3
#define SUPPORT_4GTYPE_TOTAL            3
struct DataItemFor4GUsage{
    int useddatasize;
};

class Usage4GNetwork : public QObject
{
    Q_OBJECT
public:
    explicit Usage4GNetwork(QObject *parent = nullptr);
    void set4GUsageData(int type, int datasize);
    void print();
signals:

protected:
    bool checkNewDay();
    void resetData();
    void resetDataItem(int index);
    void setDataOnce(int type, int datasize);
    void setDataItem(int index, int datasize);
    void logCurrentData();
    void logCurrentData2();
    int getTotalSize();
    int getDataSizeByIndex(int index);

private:
    DataItemFor4GUsage allData4GTypes[SUPPORT_4GTYPE_TOTAL];
    QMutex mutex;
    QDateTime lastUpdateTime;
};

#endif // USAGE4GNETWORK_H
