#ifndef EVENTFLAGBUFFER_H
#define EVENTFLAGBUFFER_H

#include <QObject>
#include <QString>
#include <QMap>

// 本部分设计的目的是在tcpclient断开连接被删除的情况下，保存一些必要的flag状态，在连接恢复时能够以正确的状态初始化
// 因为本部分内容以查询为主，所以还是使用map作为主要数据结构
struct EventFlagItem{
    int poweroffFlag;
    int acpoweroffFlag;
};

class EventFlagBuffer : public QObject
{
    Q_OBJECT
public:
    explicit EventFlagBuffer(QObject *parent = nullptr);
    ~EventFlagBuffer();
    void setBufferByPowerOffStatus(QString ip, int poweroffFlag, int acpoweroffFlag);
    EventFlagItem *getEventFlagItemByIP(QString ip);
protected:
    void clearAllEventFlagBuffer();
    EventFlagItem *findSocketByAnnuIP(QString annuip);
private:
    QMap<QString, EventFlagItem*> eventBuffer;
};
//extern EventFlagBuffer eventFlagBuffer;
#endif // EVENTFLAGBUFFER_H
