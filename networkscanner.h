#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QStringList>

class NetworkScanner : public QObject
{
    Q_OBJECT
public:
    explicit NetworkScanner(QObject *parent = nullptr);
    void scan(const QString &subnet, int port, int timeoutMs = 500);
    void stop();
    static QString detectSubnet();

signals:
    void deviceFound(const QString &ip);
    void progress(int done, int total);
    void finished();

private:
    struct Job {
        QTcpSocket *socket;
        QTimer     *timer;
    };

    void  startJob(const QString &ip);
    void  finishJob(QTcpSocket *socket, bool found);
    Job  *findJob(QTcpSocket *socket) const;

    int         _port          = 23;
    int         _timeoutMs     = 500;
    const int   _maxConcurrent = 50;
    QStringList _pending;
    QList<Job*> _active;
    int         _done  = 0;
    int         _total = 0;
};

#endif
