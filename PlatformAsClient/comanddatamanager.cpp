#include <QMutexLocker>
#include "Log/loggerworker.h"
#include "comanddatamanager.h"

ComandDataManager::ComandDataManager(QObject *parent) : QObject(parent)
{
    commandMap.clear();
}

bool ComandDataManager::newDataFromAnnuniator(QByteArray &data)
{
    QMutexLocker locker(&mutex);
    int command = (int)data[0];
    if(checkDataExisted(command)){
        return checkItem(command);
    }
    return false;
}

void ComandDataManager::newDataFromPlatform(QByteArray &data)
{
    QMutexLocker locker(&mutex);
    int command = (int)data[5];
    if(checkDataExisted(command)){
        updateItem(command);
    }else{
        addNewItem(command);
    }
}

bool ComandDataManager::checkDataExisted(int command)
{
    return commandMap.contains(command);
}

void ComandDataManager::addNewItem(int command)
{
    CommandDataItem* pItem = new CommandDataItem();
    logworker.addLogger("ComandDataManager::addNewItem - new CommandDataItem", LOGTYPE_PRINT_RECORD);
    pItem->setCount();
    commandMap.insert(command, pItem);
}

void ComandDataManager::updateItem(int command)
{
    commandMap[command]->setCount();
}

bool ComandDataManager::checkItem(int command)
{
    if(commandMap[command]->getDataValid()==false){
        CommandDataItem* pItem = commandMap.take(command);
        logworker.addLogger("ComandDataManager::checkItem Invalid false- delete CommandDataItem", LOGTYPE_PRINT_RECORD);
        delete pItem;
        return false;
    }else{
        if(commandMap[command]->getCount()<=0){
            CommandDataItem* pItem = commandMap.take(command);
            logworker.addLogger("ComandDataManager::checkItem Command over count- delete CommandDataItem", LOGTYPE_PRINT_RECORD);
            delete pItem;
        }
        return true;
    }
}
