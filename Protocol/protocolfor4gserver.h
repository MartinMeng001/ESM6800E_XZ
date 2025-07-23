#ifndef PROTOCOLFOR4GSERVER_H
#define PROTOCOLFOR4GSERVER_H

#include <QObject>
#include <QByteArray>

class ProtocolFor4GServer : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolFor4GServer(QObject *parent = nullptr);
    void setPowerAlarm();
    void clearPowerAlarm();
    void initProtocol4G(QString ip);
    // *#A>4G require IP
    int checkProtocolData(QByteArray& indata, QByteArray& outdata);
    //4G;IP;IP;alarm+type;
    int makeProtocolData(QByteArray& indata, QByteArray& outdata);
    QByteArray makeHearbeat();
signals:

protected:
    QByteArray makeHearbeatInfo();
    char makeTypeByte();
private:
    QString deviceip;
    int poweroffStatus; //1=off, 0=normal
};
extern ProtocolFor4GServer gProtocolFor4GServer;
#endif // PROTOCOLFOR4GSERVER_H
