#include "orfeoconfig.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

QString OrfeoConfig::configPath()
{
    return QCoreApplication::applicationDirPath() + "/orfeo.json";
}

static QJsonObject readRoot()
{
    QFile f(OrfeoConfig::configPath());
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}

static void writeRoot(const QJsonObject &root)
{
    QFile f(OrfeoConfig::configPath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(root).toJson());
}

QString OrfeoConfig::apiUrl()
{
    return readRoot()["apiUrl"].toString();
}

QString OrfeoConfig::apiKey()
{
    return readRoot()["apiKey"].toString();
}

void OrfeoConfig::saveApiSettings(const QString &url, const QString &key)
{
    QJsonObject root = readRoot();
    root["apiUrl"] = url;
    root["apiKey"] = key;
    writeRoot(root);
}

QList<PanelOrfeoConfig> OrfeoConfig::load()
{
    QList<PanelOrfeoConfig> list;
    for (const QJsonValue &v : readRoot()["panels"].toArray()) {
        QJsonObject o = v.toObject();
        PanelOrfeoConfig c;
        c.panelName      = o["panelName"].toString();
        c.panelIp        = o["panelIp"].toString();
        c.bank           = o["bank"].toInt(1);
        c.showTitle      = o["showTitle"].toBool(true);
        c.showDate       = o["showDate"].toBool(true);
        c.showTime       = o["showTime"].toBool(true);
        c.showSalle      = o["showSalle"].toBool(false);
        c.showPersonnel  = o["showPersonnel"].toBool(false);
        c.intervalMin    = o["intervalMin"].toInt(30);
        c.active         = o["active"].toBool(false);
        c.salleFilter    = o["salleFilter"].toString();
        list.append(c);
    }
    return list;
}

void OrfeoConfig::save(const QList<PanelOrfeoConfig> &configs)
{
    QJsonObject root = readRoot();
    QJsonArray arr;
    for (const PanelOrfeoConfig &c : configs) {
        QJsonObject o;
        o["panelName"]     = c.panelName;
        o["panelIp"]       = c.panelIp;
        o["bank"]          = c.bank;
        o["showTitle"]     = c.showTitle;
        o["showDate"]      = c.showDate;
        o["showTime"]      = c.showTime;
        o["showSalle"]     = c.showSalle;
        o["showPersonnel"] = c.showPersonnel;
        o["intervalMin"]   = c.intervalMin;
        o["active"]        = c.active;
        o["salleFilter"]   = c.salleFilter;
        arr.append(o);
    }
    root["panels"] = arr;
    writeRoot(root);
}
