#ifndef PANELCONFIG_H
#define PANELCONFIG_H

#include <QString>
#include <QList>

struct Panel {
    QString name;
    QString ip;
};

namespace PanelConfig {
    QList<Panel> load();
    void         save(const QList<Panel> &panels);
    QString      configPath();
}

#endif
