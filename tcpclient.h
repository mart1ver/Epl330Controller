#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
//#include <QTimer>

class TCPClient : public QObject
{
    Q_OBJECT
public:
    explicit TCPClient(QObject *parent = 0);
    ~TCPClient();

    QString addr;
    int port;

    Q_INVOKABLE bool connection(QString _addr, int _port);
    Q_INVOKABLE void closeConnection();

    Q_INVOKABLE bool sendCommand(QString command);
    Q_INVOKABLE QString sendQuery(QString query);

    Q_INVOKABLE bool isConnected();
    
signals:
    void sigConnected();
    void sigDisconnected();

public slots:
    void init();
    void connected();
    void disconnected();

private:
    QTcpSocket *socket;
};

#endif // TCPCLIENT_H
