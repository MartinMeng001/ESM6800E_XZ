#ifndef ITEMANNUNIATORCONNECTION_H
#define ITEMANNUNIATORCONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QTime>
#include <QTimer>
#include <QTcpSocket>
#include <QMutex>
#include <QAbstractSocket>

class TimeMeauring : public QObject
{
    Q_OBJECT
public:
    explicit TimeMeauring(QObject *parent = nullptr){
        elapsed = 0;
        flagEnd = false;
        flagInvalid = true;
    }
    ~TimeMeauring(){

    }
    void start(){
        time4measuring.restart();
        flagEnd = false;
        flagInvalid = false;
    }
    void end(int flag){
        elapsed = time4measuring.elapsed();
        if(flag == 0) flagInvalid = false;
        else flagInvalid = true;
        flagEnd = true;
    }
    int getElapsedTime(){
        if(flagEnd == false) return -2;
        if(flagInvalid == true) return -1;
        return elapsed;
    }
private:
    QTime time4measuring;
    int elapsed;    // 消耗的时间
    bool flagInvalid;
    bool flagEnd;
};

class ItemAnnuniatorConnection : public QObject
{
    Q_OBJECT
public:
    explicit ItemAnnuniatorConnection(QObject *parent = nullptr);
    ~ItemAnnuniatorConnection();

    void initItem(QTcpSocket* socket_new);
    QTcpSocket *getTcpSocket(){return psocket;}
    int SendData(QByteArray& data4sending);

    QString getAnnuniatorIP(){return annuniatorIP;}
    void setAnnuniatorIP(const QString &value);
    bool getBeValidConnection() const;

    // 新增：连接状态检查
    bool isSocketValid();
    QAbstractSocket::SocketState getSocketState();

    // 改进的基本信息请求
    void req_basicInfo();
    int getBasicInfoElapsed();
    void closeSocket();

    // 新增：连接健康检查
    bool isConnectionHealthy();
    void resetConnectionTimeout();

signals:
    void sigNewData(QByteArray data);
    void connectionValidated();  // 新增：连接验证成功信号
    void connectionFailed();     // 新增：连接失败信号

public slots:
    void processReadyRead();
    void processReadyWrite(QByteArray& data);
    void processDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);  // 新增：错误处理
    void onValidationTimeout();  // 新增：验证超时处理

protected:
    void disConnect();
    bool config();

    // 改进的数据包处理
    void findItemPackets(QByteArray& data);
    bool parse_basicInfo(QByteArray& data);
    void ip2Hex(QByteArray& data, int index, QString ip);

    // 新增：连接验证相关
    void startValidationProcess();
    void stopValidationProcess();

private:
    QString annuniatorIP;
    bool beValidConnection;
    bool beExiting;
    QTcpSocket *psocket;    // connection for the item
    TimeMeauring timeMeasuring;

    // 新增：连接管理相关
    QTimer* validationTimer;   // 验证超时定时器
    QTimer* heartbeatTimer;    // 心跳定时器
    QTime lastDataTime;        // 最后数据接收时间
    QMutex connectionMutex;    // 连接状态保护

    // 新增：连接配置参数
    static const int VALIDATION_TIMEOUT = 10000;  // 10秒验证超时
    static const int HEARTBEAT_INTERVAL = 30000;  // 30秒心跳间隔
    static const int CONNECTION_TIMEOUT = 60000;  // 60秒连接超时

    bool validationInProgress;
    int validationAttempts;
    static const int MAX_VALIDATION_ATTEMPTS = 3;
};

#endif // ITEMANNUNIATORCONNECTION_H
