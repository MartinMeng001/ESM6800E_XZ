#include <iostream>
#include <QHostAddress>
#include "Log/loggerworker.h"
#include "dataitem_platformclient.h"
#include "AnnuniatorStatus/usage4gnetwork.h"
#include "AnnuniatorStatus/annuniatorstatus.h"
#include "itemplatformconnection.h"

ItemPlatformConnection::ItemPlatformConnection(QObject *parent) : QObject(parent)
{
    psocket = nullptr;
    annuniatorIP="";
    beExiting = false;
}

ItemPlatformConnection::~ItemPlatformConnection()
{
    if(psocket!=nullptr){
        disConnect();
        logworker.addLogger("PlatformClient Disconnected", LOGTYPE_PRINT);
    }
}

void ItemPlatformConnection::initItem(QTcpSocket *socket_new)
{
    if(psocket==nullptr){
        if(socket_new!=nullptr){
            psocket=socket_new;
            QString ipnow = psocket->peerAddress().toString();
            if(ipnow.startsWith("::ffff:")){
                annuniatorIP = ipnow.mid(QString("::ffff:").size());
            }else{
                annuniatorIP = ipnow;
            }
            //annuniatorIP = psocket->peerAddress().toString().s;
            beValidConnection = true;
            config();
            logworker.addLogger("PlatformClient = "+annuniatorIP, LOGTYPE_PRINT_RECORD);
        }
    }
}

int ItemPlatformConnection::SendData(QByteArray &data4sending)
{
    if(dataValidCheck.newDataFromAnnuniator(data4sending)==false) return 0;
    int size=0;
    if(psocket==nullptr)return 0;
    if(psocket->isValid()==false) return 0;
    if(psocket->isWritable()==false) return 0;
    size = psocket->write(data4sending);
    emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_PLATFORCLIENT, size);
    QString senddata = data4sending.toHex().toUpper();
    QString sendData = QString("PlatformClient Send:%1").arg(senddata);
    logworker.addLogger(sendData, LOGTYPE_PRINT);
    return size;
}

void ItemPlatformConnection::setAnnuniatorIP(const QString &value)
{
    annuniatorIP = value;
}

bool ItemPlatformConnection::getBeValidConnection() const
{
    return beValidConnection;
}

void ItemPlatformConnection::req_basicInfo()
{
    QByteArray dataarray;
    dataarray.resize(10);
    dataarray[0]=0x53;dataarray[1]=0x42;dataarray[2]=0x44;dataarray[3]=0x5A;
    dataarray[4]=0x00;dataarray[5]=0x80;dataarray[6]=0x01;dataarray[7]=0x81;
    dataarray[8]=0xAA;dataarray[9]=0x55;
    SendData(dataarray);
    timeMeasuring.start();
}

int ItemPlatformConnection::getBasicInfoElapsed()
{
    return timeMeasuring.getElapsedTime();
}

void ItemPlatformConnection::closeSocket()
{
    disConnect();
}

void ItemPlatformConnection::processReadyRead()
{
    if(psocket->isValid()==false) return;
    QByteArray data=psocket->readAll();   
    if(data.size()==0) return;
    emit sig4GNetworkDataChanging(SUPPORT_4GTYPE_PLATFORCLIENT, data.size());
    if(beExiting) return; // 如果退出，就不再处理数据
    findItemPackets(data);
    QString recdata = data.toHex().toUpper();
    QString recData = QString("PlatformclientServer Receive:%1").arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
}

void ItemPlatformConnection::processReadyWrite(QByteArray &data)
{
    SendData(data);
}

void ItemPlatformConnection::processDisconnected()
{
    disConnect();
}

void ItemPlatformConnection::disConnect()
{
    beExiting = true;
    if(psocket==nullptr)return;
    psocket->close();
    psocket=nullptr;
    timeMeasuring.end(1);
    logworker.addLogger("PlatformClient Disconnected", LOGTYPE_PRINT);
}

void ItemPlatformConnection::config()
{
    if(psocket!=nullptr){
        connect(psocket, &QTcpSocket::readyRead, this, &ItemPlatformConnection::processReadyRead);
        connect(psocket, &QTcpSocket::disconnected, this, &ItemPlatformConnection::processDisconnected);
        connect(this, &ItemPlatformConnection::sig4GNetworkDataChanging, &gAnnuniatorStatus, &AnnuniatorStatus::Status_4GNetworkUsage);
    }
}


void ItemPlatformConnection::findItemPackets(QByteArray &data)
{
//    if(parse_basicInfo(data)){
//        if(beValidConnection == false) beValidConnection = true;
//    }else{
        // add data to send buffer
        dataItem_PlatformClient *dataitem=new dataItem_PlatformClient;
        dataitem->dataFromAnnuniator = data;
        dataitem->datatime=QTime::currentTime();
        dataMgrPlatformClient.addAnnuniatorDataItem(dataitem);
        dataValidCheck.newDataFromPlatform(data);
        emit sigNewData(data);
//    }

}

bool ItemPlatformConnection::parse_basicInfo(QByteArray &data)
{
    if(data.size()<5) return false;
    if(data[0]==(char)0x80 && data[1]==(char)0xC2 && data[2]==(char)0x42 && data[3]==(char)0xAA && data[4]==(char)0x55) return true;
    return false;
}

void ItemPlatformConnection::ip2Hex(QByteArray &data, int index, QString ip)
{
    QStringList ips = ip.split(".");
    if(ips.size()==4){
        data[index] = ips.at(0).toInt();
        data[index+1] = ips.at(1).toInt();
        data[index+2] = ips.at(2).toInt();
        data[index+3] = ips.at(3).toInt();
    }else{
        data[index] = 0;
        data[index+1] = 0;
        data[index+2] = 0;
        data[index+3] = 0;
    }
}
