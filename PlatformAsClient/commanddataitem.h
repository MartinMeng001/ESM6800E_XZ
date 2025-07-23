#ifndef COMMANDDATAITEM_H
#define COMMANDDATAITEM_H

#include <QObject>
#include <QTime>

class CommandDataItem : public QObject
{
    Q_OBJECT
public:
    explicit CommandDataItem(QObject *parent = nullptr);

    bool getDataValid();
    void setCount();
    int getCount(){ return count; }
signals:

protected:
    bool updateCount();
    bool checkTimeValid();
private:
    int count;
    QTime lastUpdateTime;
    const int allowInterval = 60;
};

#endif // COMMANDDATAITEM_H
