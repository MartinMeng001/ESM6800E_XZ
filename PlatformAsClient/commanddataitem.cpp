#include "commanddataitem.h"

CommandDataItem::CommandDataItem(QObject *parent) : QObject(parent)
{

}

bool CommandDataItem::getDataValid()
{
    if(checkTimeValid()==false)return false;
    return updateCount();
}

void CommandDataItem::setCount()
{
    count++;
    lastUpdateTime = QTime::currentTime();
}

bool CommandDataItem::updateCount()
{
    count--;
    if(count<0) return false;
    return true;
}

bool CommandDataItem::checkTimeValid()
{
    if(lastUpdateTime.secsTo(QTime::currentTime())>=allowInterval)return false;
    return true;
}
