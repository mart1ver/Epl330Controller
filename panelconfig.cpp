#include "panelconfig.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

QString PanelConfig::configPath()
{
    return QCoreApplication::applicationDirPath() + "/panels.json";
}

QList<Panel> PanelConfig::load()
{
    QList<Panel> panels;

    QFile f(configPath());
    if (!f.open(QIODevice::ReadOnly))
        return panels;
    for (const QJsonValue &v : QJsonDocument::fromJson(f.readAll()).array()) {
        QJsonObject o = v.toObject();
        panels.append({o["name"].toString(), o["ip"].toString()});
    }
    return panels;
}

void PanelConfig::save(const QList<Panel> &panels)
{
    QJsonArray arr;
    for (const Panel &p : panels) {
        QJsonObject o;
        o["name"] = p.name;
        o["ip"]   = p.ip;
        arr.append(o);
    }
    QFile f(configPath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(arr).toJson());
}
