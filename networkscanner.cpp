#include "networkscanner.h"
#include <QNetworkInterface>

NetworkScanner::NetworkScanner(QObject *parent) : QObject(parent) {}

QString NetworkScanner::detectSubnet()
{
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags() & QNetworkInterface::IsLoopBack)
            continue;
        if (!(iface.flags() & QNetworkInterface::IsUp))
            continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QStringList parts = entry.ip().toString().split('.');
                if (parts.size() == 4)
                    return parts[0] + "." + parts[1] + "." + parts[2];
            }
        }
    }
    return "192.168.1";
}

void NetworkScanner::scan(const QString &subnet, int port, int timeoutMs)
{
    stop();
    _port      = port;
    _timeoutMs = timeoutMs;
    _done      = 0;

    for (int i = 1; i <= 254; ++i)
        _pending << (subnet + "." + QString::number(i));
    _total = _pending.size();

    while (!_pending.isEmpty() && _active.size() < _maxConcurrent)
        startJob(_pending.takeFirst());
}

void NetworkScanner::stop()
{
    for (Job *j : _active) {
        j->socket->abort();
        j->socket->deleteLater();
        j->timer->deleteLater();
        delete j;
    }
    _active.clear();
    _pending.clear();
    _done  = 0;
    _total = 0;
}

void NetworkScanner::startJob(const QString &ip)
{
    Job *job    = new Job;
    job->socket = new QTcpSocket(this);
    job->timer  = new QTimer(this);
    job->timer->setSingleShot(true);

    connect(job->socket, &QTcpSocket::connected, this, [this, s = job->socket]() {
        finishJob(s, true);
    });
    connect(job->socket, &QAbstractSocket::errorOccurred, this, [this, s = job->socket](QAbstractSocket::SocketError) {
        finishJob(s, false);
    });
    connect(job->timer, &QTimer::timeout, this, [this, s = job->socket]() {
        finishJob(s, false);
    });

    _active.append(job);
    job->socket->connectToHost(ip, _port);
    job->timer->start(_timeoutMs);
}

NetworkScanner::Job *NetworkScanner::findJob(QTcpSocket *socket) const
{
    for (Job *j : _active)
        if (j->socket == socket)
            return j;
    return nullptr;
}

void NetworkScanner::finishJob(QTcpSocket *socket, bool found)
{
    Job *job = findJob(socket);
    if (!job)
        return;

    if (found)
        emit deviceFound(socket->peerAddress().toString());

    _active.removeOne(job);
    job->socket->abort();
    job->socket->deleteLater();
    job->timer->stop();
    job->timer->deleteLater();
    delete job;

    emit progress(++_done, _total);

    if (!_pending.isEmpty())
        startJob(_pending.takeFirst());
    else if (_active.isEmpty())
        emit finished();
}
