#include "orfeoclient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>

OrfeoClient::OrfeoClient(QObject *parent) : QObject(parent)
{
    _nam = new QNetworkAccessManager(this);
    connect(_nam, &QNetworkAccessManager::finished, this, &OrfeoClient::onReplyFinished);
}

void OrfeoClient::setApiUrl(const QString &url) { _apiUrl = url; }
void OrfeoClient::setApiKey(const QString &key) { _apiKey = key; }

void OrfeoClient::fetchEvents()
{
    if (_apiUrl.isEmpty()) {
        emit errorOccurred("URL API non configurée");
        return;
    }

    // TODO: replace with the actual Orfeo API endpoint once documentation is obtained
    QNetworkRequest req(QUrl(_apiUrl + "/api/evenements"));
    // TODO: check with Orfeo whether auth uses Bearer token, Basic, or a custom header
    req.setRawHeader("Authorization", ("Bearer " + _apiKey).toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _nam->get(req);
}

void OrfeoClient::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }
    emit eventsReceived(parseResponse(reply->readAll()));
}

QList<OrfeoEvent> OrfeoClient::parseResponse(const QByteArray &data)
{
    QList<OrfeoEvent> events;

    // TODO: update field names once the actual Orfeo API response format is documented
    // Current assumption: JSON array of objects with these keys:
    //   id, title (titre), date (YYYY-MM-DD), time (HH:MM), salle, personnel
    QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        OrfeoEvent e;
        e.id        = o["id"].toString();
        e.title     = o["titre"].toString();       // TODO: confirm exact key name
        e.date      = o["date"].toString();
        e.time      = o["heure"].toString();        // TODO: confirm exact key name
        e.salle     = o["salle"].toString();
        e.personnel = o["personnel"].toString();
        events.append(e);
    }
    return events;
}
