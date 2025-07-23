#ifndef ITEMPLATFORMCONNECTION_H
#define ITEMPLATFORMCONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QTcpSocket>
#include "comanddatamanager.h"

class TimeMeauring4Platform : public QObject
{
    Q_OBJECT
public:
    explicit TimeMeauring4Platform(QObject *parent = nullptr){
        elapsed=0;
        flagEnd=false;
        flagInvalid=true;
    }
    ~TimeMeauring4Platform(){

    }
    void start(){
        time4measuring.restart();
        flagEnd=false;
        flagInvalid=false;
    }
    void end(int flag){
        elapsed = time4measuring.elapsed();
        if(flag==0) flagInvalid=false;
        else flagInvalid=true;
        flagEnd=true;
    }
    int getElapsedTime(){
        if(flagEnd==false) return -2;
        if(flagInvalid==true) return -1;
        return elapsed;
    }
private:
    QTime time4measuring;
    int elapsed;    // 消耗的时间
    bool flagInvalid;
    bool flagEnd;
};

class ItemPlatformConnection : public QObject
{
    Q_OBJECT
public:
    explicit ItemPlatformConnection(QObject *parent = nullptr);
    ~ItemPlatformConnection();

    void initItem(QTcpSocket* socket_new);
    QTcpSocket *getTcpSocket(){return psocket;}
    int SendData(QByteArray& data4sending);

    QString getAnnuniatorIP(){return annuniatorIP;}
    void setAnnuniatorIP(const QString &value);
    bool getBeValidConnection() const;

    void req_basicInfo();
    int getBasicInfoElapsed();
    void closeSocket();
signals:
    void sigNewData(QByteArray data);
    void sig4GNetworkDataChanging(int type, int datasize);
public slots:
    void processReadyRead();
    void processReadyWrite(QByteArray& data);
    void processDisconnected();

protected:
    void disConnect();
    void config();

    void findItemPackets(QByteArray& data);
    bool parse_basicInfo(QByteArray& data);
    void ip2Hex(QByteArray& data, int index, QString ip);
private:
    QString annuniatorIP;
    bool beValidConnection, beExiting = false;
    QTcpSocket *psocket;    // connection for the item
    TimeMeauring4Platform timeMeasuring;
    ComandDataManager dataValidCheck;
};

#endif // ITEMPLATFORMCONNECTION_H
