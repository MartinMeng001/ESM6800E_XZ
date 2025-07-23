#include "Log/loggerworker.h"
#include "regionmanager.h"

RegionManager regionManager;
RegionManager::RegionManager(QObject *parent) : QObject(parent)
{
    regions.clear();
    disabledGuardIds.clear();
}

void RegionManager::setAllRegions(QStringList regionparams)
{
    if(regionparams.length()!=ALLOW_REGION_ITEMS)return;
    regions.clear();
    for(int i=0;i<ALLOW_REGION_ITEMS;i++){
        regions.append(regionparams.at(i));
    }
}

void RegionManager::setAllDisabledIds(QStringList disableids)
{
    if(disableids.length()>ALLOW_DISABLEID_ITEMS)return;
    disabledGuardIds.clear();
    foreach (auto item, disableids) {
        if(item=="FFFFFFFF")continue;
        disabledGuardIds.append(item);
    }
//    QString invalidinfo = "Invalid GuardId="+disabledGuardIds.at(0);
//    logworker.addLogger(invalidinfo, LOGTYPE_PRINT);
}

bool RegionManager::checkRegion(QString &item)
{
    if(checkRegionSizeValid()==false) return false;
    for(int i=0;i<ALLOW_REGION_ITEMS;i++){
        if(checkRegionItemValid(regions.at(i))==false)continue;
        if(checkRegionItem(regions.at(i), item))return true;
    }
    return false;
}
// return false to disable
bool RegionManager::checkIdAllowed(QString &item)
{
//    QString invalidinfo = "checkIdAllowed GuardId="+item;
//    logworker.addLogger(invalidinfo, LOGTYPE_PRINT);
    if(checkDisableIdsValid()==false) return true;  // diable list don't existed, don't disable
    foreach(auto item1, disabledGuardIds){
        if(checkRegionItem(item1, item)) return false;  // find disable id, disabled it
    }
    return true;
}

bool RegionManager::checkRegionSizeValid()
{
    if(regions.length()== ALLOW_REGION_ITEMS) return true;
    return false;
}

bool RegionManager::checkRegionItemValid(QString paramsitem)
{
    if(paramsitem=="FFF")return false;
    return true;
}

bool RegionManager::checkRegionItem(QString paramsitem, QString &item)
{
    if(paramsitem==item) return true;
    return false;
}

bool RegionManager::checkDisableIdsValid()
{
    if(disabledGuardIds.size()>0) return true;
    return false;
}

bool RegionManager::checkDisableIdsItemValid(QString iditem)
{
    return true;
}

bool RegionManager::checkDisableIdItem(QString paramsitem, QString &item)
{
    if(paramsitem=="FFFFFFFF") return false;
    if(paramsitem==item) return true;
    return false;
}
