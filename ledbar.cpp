#include "ledbar.h"

ledBar::ledBar(QObject *parent) :
    QObject(parent)
{
    tcpThread = new QThread(this);
    eplClient = new TCPClient;
    eplClient->moveToThread(tcpThread);
    connect(tcpThread, &QThread::started, eplClient, &TCPClient::init); // socket créé dans le worker thread
    connect(eplClient, SIGNAL(sigConnected()), this, SLOT(led_connected()));
    connect(eplClient, SIGNAL(sigDisconnected()), this, SLOT(led_disconnected()));
    tcpThread->start();
}

ledBar::~ledBar()
{
    tcpThread->quit();
    tcpThread->wait();
    delete eplClient;
}

// ***************** METHODS ********************** //

QString ledBar::getStatus()
{
    return sendQuery("ETA");
}

QString ledBar::getCurrentMsgNumber()
{
    QString msgNum;
    QString key;
    key = "Texte en cours : ";
    msgNum = getStatus();
    msgNum.remove(0, msgNum.indexOf(key) + key.count());
    msgNum = msgNum.left(2);
    return msgNum;
}

void ledBar::connection(QString _ip, int port)
{
    QMetaObject::invokeMethod(eplClient, "connection",
        Qt::BlockingQueuedConnection,
        Q_ARG(QString, _ip),
        Q_ARG(int, port));
}

void ledBar::closeConnection()
{
    QMetaObject::invokeMethod(eplClient, "closeConnection",
        Qt::BlockingQueuedConnection);
}

bool ledBar::isConnected()
{
    bool result = false;
    QMetaObject::invokeMethod(eplClient, "isConnected",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, result));
    return result;
}

QString ledBar::getStoredMessage(int bank)
{
    QString query;
    if (bank < 10)
    {
        return sendQuery("COP0" + QString::number(bank));
    }
    else
    {
        return sendQuery("COP" + QString::number(bank));
    }
}

void ledBar::freeze()
{
    sendMessage("HLT");
}

void ledBar::reboot()
{
    sendMessage("DCH");
}

void ledBar::run()
{
    sendMessage("RUN");
}

void ledBar::clear()
{
    sendMessage("CLR");
}

void ledBar::authorizeAll()
{
    sendMessage("AUT");
}

void ledBar::blank()
{
    sendMessage("MSG00");
    authorize(0);
    freeze();
}

void ledBar::brightness(int value)
{
    QString brightness;
    brightness = "SET D=" + QString::number(value);
    sendMessage(brightness);
}

void ledBar::sendMessage(QString msg)
{
    QMetaObject::invokeMethod(eplClient, "sendCommand",
        Qt::BlockingQueuedConnection,
        Q_ARG(QString, msg));
}

QString ledBar::sendQuery(QString msg)
{
    QString result;
    QMetaObject::invokeMethod(eplClient, "sendQuery",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(QString, result),
        Q_ARG(QString, msg));
    return result;
}

void ledBar::authorize(int bank)
{
        sendMessage("AUT " + QString::number(bank));
}

void ledBar::clearBank(int bank)
{
    QString msg;
    if (bank < 10)
    {
        msg = "MSG0" + QString::number(bank);
    }
    else
    {
        msg = "MSG" + QString::number(bank);
    }
    sendMessage(msg);
}

void ledBar::clearSchedule(int bank)
{
    sendMessage((bank < 10 ? QString("PLE0") : QString("PLE")) + QString::number(bank));
}

void ledBar::addSchedule(int bank, QString plage)
{
    sendMessage((bank < 10 ? QString("PLA0") : QString("PLA")) + QString::number(bank) + " " + plage);
}

void ledBar::delSchedule(int bank, QString plage)
{
    sendMessage((bank < 10 ? QString("PLS0") : QString("PLS")) + QString::number(bank) + " " + plage);
}

void ledBar::setZones(QString zoneDef)
{
    sendMessage("ZON " + zoneDef);
}

void ledBar::setIpAddress(QString ip, QString mask, QString gateway)
{
    QString msg;
    msg = "! " + ip + mask + gateway;
     sendMessage(msg);
}



// ***************** SLOTS ********************** //

void ledBar::led_connected()
{
    emit sigLedConnected();
}

void ledBar::led_disconnected()
{
    emit sigLedDisconnected();
}
