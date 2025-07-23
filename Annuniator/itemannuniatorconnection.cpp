#include <iostream>
#include <QHostAddress>
#include "Log/loggerworker.h"
#include "datamgr_fromannuniator.h"
#include "itemannuniatorconnection.h"
#include "Protocol/protocolannuniator4u.h"

ItemAnnuniatorConnection::ItemAnnuniatorConnection(QObject *parent) : QObject(parent)
{
    psocket=nullptr;
    annuniatorIP="";
    beExiting = false;
}

ItemAnnuniatorConnection::~ItemAnnuniatorConnection()
{
//    if(psocket!=nullptr){
//        disConnect();
//        logworker.addLogger("Annuniator Disconnected", LOGTYPE_PRINT);
//    }
    if(psocket!=nullptr){
        delete psocket;
        //disConnect();
    }
}

void ItemAnnuniatorConnection::initItem(QTcpSocket *socket_new)
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
            config();
            beValidConnection=true;
            //req_basicInfo();
            logworker.addLogger("AnnuniatorIP = "+annuniatorIP, LOGTYPE_PRINT_RECORD);
        }
    }
}

int ItemAnnuniatorConnection::SendData(QByteArray &data4sending)
{
    int size=0;
    if(psocket==nullptr)return 0;
    if(psocket->isValid()==false) return 0;
    if(psocket->isWritable()==false) return 0;
    size = psocket->write(data4sending);
    QString senddata = data4sending.toHex().toUpper();
    QString sendData = QString("Annuniator Send:%1").arg(senddata);
    logworker.addLogger(sendData, LOGTYPE_PRINT);
    return size;
}

void ItemAnnuniatorConnection::setAnnuniatorIP(const QString &value)
{
    annuniatorIP = value;
}

bool ItemAnnuniatorConnection::getBeValidConnection() const
{
    return beValidConnection;
}

void ItemAnnuniatorConnection::req_basicInfo()
{
    QByteArray dataarray;
    dataarray.resize(10);
    dataarray[0]=0x53;dataarray[1]=0x42;dataarray[2]=0x44;dataarray[3]=0x5A;
    dataarray[4]=0x00;dataarray[5]=0x80;dataarray[6]=0x01;dataarray[7]=0x81;
    dataarray[8]=0xAA;dataarray[9]=0x55;
    SendData(dataarray);
    timeMeasuring.start();
}

int ItemAnnuniatorConnection::getBasicInfoElapsed()
{
    return timeMeasuring.getElapsedTime();
}

void ItemAnnuniatorConnection::closeSocket()
{
    disConnect();
}

void ItemAnnuniatorConnection::processReadyRead()
{
    if(psocket->isValid()==false) return;
    QByteArray data=psocket->readAll();
    if(beExiting) return; // 如果退出，就不再处理数据
    if(data.size()==0) return;
    findItemPackets(data);
    QString recdata = data.toHex().toUpper();
    QString recData = QString("Annuniator Receive:%1").arg(recdata);
    logworker.addLogger(recData, LOGTYPE_PRINT);
}

void ItemAnnuniatorConnection::processReadyWrite(QByteArray &data)
{
    SendData(data);
}

void ItemAnnuniatorConnection::processDisconnected()
{
    //disConnect();
    std::cout << "get a disconnect signal" << std::endl;
    if(psocket==nullptr)return;
}

void ItemAnnuniatorConnection::disConnect()
{
//    beExiting = true;
//    if(psocket==nullptr)return;
//    psocket->close();
//    psocket=nullptr;
//    timeMeasuring.end(1);
//    logworker.addLogger("Annuniator Disconnected", LOGTYPE_PRINT);


    disconnect(psocket, &QTcpSocket::readyRead, this, &ItemAnnuniatorConnection::processReadyRead);
    disconnect(psocket, &QTcpSocket::disconnected, this, &ItemAnnuniatorConnection::processDisconnected);
    disconnect(this, &ItemAnnuniatorConnection::sigNewData, &gProtocolAnnuniator4U, &ProtocolAnnuniator4U::processAnnuniatorData);
//    disconnect(psocket, &QTcpSocket::readyRead, this, &ItemTcpConnection::processReadyRead);
//    disconnect(psocket, &QTcpSocket::disconnected, this, &ItemTcpConnection::processDisconnected);
    timeMeasuring.end(1);
    beExiting = true;
    if(psocket==nullptr)return;
    //emit delThisItem(annuniatorIP);
    psocket->close();
}

void ItemAnnuniatorConnection::config()
{
    if(psocket!=nullptr){
        connect(psocket, &QTcpSocket::readyRead, this, &ItemAnnuniatorConnection::processReadyRead);
        connect(psocket, &QTcpSocket::disconnected, this, &ItemAnnuniatorConnection::processDisconnected);
        connect(this, &ItemAnnuniatorConnection::sigNewData, &gProtocolAnnuniator4U, &ProtocolAnnuniator4U::processAnnuniatorData);
    }
}


void ItemAnnuniatorConnection::findItemPackets(QByteArray &data)
{
    if(parse_basicInfo(data)){
        if(beValidConnection == false) beValidConnection = true;
    }else{
        // add data to send buffer
        dataItem_Annuniator *dataitem=new dataItem_Annuniator;
        dataitem->dataFromAnnuniator = data;
        dataitem->datatime=QTime::currentTime();
        dataMgrFromAnnuniator.addAnnuniatorDataItem(dataitem);
        emit sigNewData(data);
    }

}

bool ItemAnnuniatorConnection::parse_basicInfo(QByteArray &data)
{
    if(data.size()<5) return false;
    if(data[0]==(char)0x80 && data[1]==(char)0xC2 && data[2]==(char)0x42 && data[3]==(char)0xAA && data[4]==(char)0x55) return true;
    return false;
}

void ItemAnnuniatorConnection::ip2Hex(QByteArray &data, int index, QString ip)
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
