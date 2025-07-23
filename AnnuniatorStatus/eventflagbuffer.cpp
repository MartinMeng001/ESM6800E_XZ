#include "Log/loggerworker.h"
#include "eventflagbuffer.h"

//EventFlagBuffer eventFlagBuffer;
EventFlagBuffer::EventFlagBuffer(QObject *parent) : QObject(parent)
{
    eventBuffer.clear();
}
EventFlagBuffer::~EventFlagBuffer()
{
    clearAllEventFlagBuffer();
    logworker.addLogger("EventFlagBuffer Clear", LOGTYPE_PRINT);
}
void EventFlagBuffer::setBufferByPowerOffStatus(QString ip, int poweroffFlag, int acpoweroffFlag)
{
    EventFlagItem* item = findSocketByAnnuIP(ip);
    if(item==nullptr){
        item = new struct EventFlagItem;
        logworker.addLogger("EventFlagBuffer new struct EventFlagItem", LOGTYPE_PRINT_RECORD);
        item->poweroffFlag=poweroffFlag;
        item->acpoweroffFlag=acpoweroffFlag;
        eventBuffer.insert(ip, item);
        return;
    }
    if(item->poweroffFlag!=poweroffFlag || item->acpoweroffFlag!=acpoweroffFlag){
        item->poweroffFlag=poweroffFlag;
        item->acpoweroffFlag=acpoweroffFlag;
        //qDebug() << "setBufferByPowerOffStatus (power-ac)" << poweroffFlag << "-" << acpoweroffFlag << " *** " << ip;
    }
}
EventFlagItem *EventFlagBuffer::getEventFlagItemByIP(QString ip)
{
    return findSocketByAnnuIP(ip);
}
void EventFlagBuffer::clearAllEventFlagBuffer()
{
    if(eventBuffer.size()==0) return;
    QMap<QString, EventFlagItem*>::iterator it;
    for(it=eventBuffer.begin();it!=eventBuffer.end();){
        logworker.addLogger("EventFlagBuffer delete struct EventFlagItem", LOGTYPE_PRINT_RECORD);
        delete it.value();
        it = eventBuffer.erase(it);
    }
}
EventFlagItem *EventFlagBuffer::findSocketByAnnuIP(QString annuip)
{
    if(eventBuffer.isEmpty()) return nullptr;
    if(eventBuffer.contains(annuip)){
        return eventBuffer[annuip];
    }
    return nullptr;
}
