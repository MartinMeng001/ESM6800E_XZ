#ifndef REGIONMANAGER_H
#define REGIONMANAGER_H

#include <QObject>
#include <QStringList>

#define ALLOW_REGION_ITEMS      20
#define ALLOW_DISABLEID_ITEMS   15
class RegionManager : public QObject
{
    Q_OBJECT
public:
    explicit RegionManager(QObject *parent = nullptr);
    void setAllRegions(QStringList regionparams);
    void setAllDisabledIds(QStringList disableids);
    bool checkRegion(QString &item);
    bool checkIdAllowed(QString &item);
signals:

protected:
    bool checkRegionSizeValid();
    bool checkRegionItemValid(QString paramsitem);
    bool checkRegionItem(QString paramsitem, QString &item);

    bool checkDisableIdsValid();
    bool checkDisableIdsItemValid(QString iditem);
    bool checkDisableIdItem(QString paramsitem, QString &item);
private:
    QStringList regions;
    QStringList disabledGuardIds;
};
extern RegionManager regionManager;

#endif // REGIONMANAGER_H
