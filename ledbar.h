#ifndef LEDBAR_H
#define LEDBAR_H

#include <QObject>
#include <QThread>
#include <tcpclient.h>

class ledBar : public QObject
{
    Q_OBJECT
public:
    explicit ledBar(QObject *parent = 0);
    ~ledBar();

    void connection(QString _ip, int port);
    void closeConnection();
    bool isConnected();

    void freeze();
    void reboot();
    void run();
    void clear();
    void authorize(int bank);
    void authorizeAll();
    void blank();
    void brightness(int value);
    void clearBank(int bank);
    void clearSchedule(int bank);
    void addSchedule(int bank, QString plage);
    void delSchedule(int bank, QString plage);
    void setZones(QString zoneDef);
    void setIpAddress(QString ip, QString mask, QString gateway);

    QString getStatus();
    QString getCurrentMsgNumber();
    QString getStoredMessage(int bank);

    void sendMessage(QString msg);
    QString sendQuery(QString msg);

signals:
    void sigLedConnected();
    void sigLedDisconnected();

public slots:
    void led_connected();
    void led_disconnected();

private:
    TCPClient *eplClient;
    QThread   *tcpThread;
};

#endif // LEDBAR_H
