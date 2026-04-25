#ifndef ORFEOCONFIG_H
#define ORFEOCONFIG_H

#include <QString>
#include <QList>

struct PanelOrfeoConfig {
    QString panelName;
    QString panelIp;
    int     bank          = 1;
    bool    showTitle     = true;
    bool    showDate      = true;
    bool    showTime      = true;
    bool    showSalle     = false;
    bool    showPersonnel = false;
    int     intervalMin   = 30;
    bool    active        = false;
    QString salleFilter;
};

namespace OrfeoConfig {
    QList<PanelOrfeoConfig> load();
    void                    save(const QList<PanelOrfeoConfig> &configs);
    QString                 configPath();
    QString                 apiUrl();
    QString                 apiKey();
    void                    saveApiSettings(const QString &url, const QString &key);
}

#endif
