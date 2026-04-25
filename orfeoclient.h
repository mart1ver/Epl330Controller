#ifndef ORFEOCLIENT_H
#define ORFEOCLIENT_H

#include <QObject>
#include <QList>
#include "orfeoevent.h"

class QNetworkAccessManager;
class QNetworkReply;

class OrfeoClient : public QObject
{
    Q_OBJECT
public:
    explicit OrfeoClient(QObject *parent = nullptr);

    void setApiUrl(const QString &url);
    void setApiKey(const QString &key);
    void fetchEvents();

signals:
    void eventsReceived(QList<OrfeoEvent> events);
    void errorOccurred(QString message);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *_nam;
    QString _apiUrl;
    QString _apiKey;

    QList<OrfeoEvent> parseResponse(const QByteArray &data);
};

#endif
