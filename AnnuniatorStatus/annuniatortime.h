#ifndef ANNUNIATORTIME_H
#define ANNUNIATORTIME_H

#include <QObject>
#include <QDateTime>

class AnnuniatorTime : public QObject
{
    Q_OBJECT
public:
    explicit AnnuniatorTime(QObject *parent = nullptr);

    int setCurrentAnnuniatorTime(QDateTime datetime);
    QDateTime getAnnuniatorTime();

signals:
    void updateAnnuniatorTime(QByteArray data);
protected:
    bool checkDeviceTimeValid();
    int getTimeDiff(QDateTime datetime);
    bool checkTimeDiff();
private:
    int timediff;   // correct time = now + timediff
    AnnuniatorTime* pObj = nullptr;
};

#endif // ANNUNIATORTIME_H
