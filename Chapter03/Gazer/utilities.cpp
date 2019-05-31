#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostInfo>
#include <QDebug>

#include "utilities.h"

QString Utilities::getDataPath()
{
    QString user_movie_path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0];
    QDir movie_dir(user_movie_path);
    movie_dir.mkpath("Gazer");
    return movie_dir.absoluteFilePath("Gazer");
}

QString Utilities::newSavedVideoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getSavedVideoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}

void Utilities::notifyMobile(int cameraID)
{
    QString endpoint = "https://maker.ifttt.com/trigger/...";
    // CHANGE endpoint TO YOURS HERE:
    // https://maker.ifttt.com/trigger/Motion-Detected-by-Gazer/with/key/-YOUR_KEY
    // QString endpoint = QUrl("https://maker.ifttt.com/YOUR_END PIOIN");
    QNetworkRequest request = QNetworkRequest(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject json;
    json.insert("value1", QString("%1").arg(cameraID));
    json.insert("value2", QHostInfo::localHostName());
    QNetworkAccessManager nam;
    QNetworkReply *rep = nam.post(request, QJsonDocument(json).toJson());
    while(!rep->isFinished()) {
        QApplication::processEvents();
    }
    // QString strReply = (QString)rep->readAll();
    // qDebug()<<"Test: "<<strReply;
    rep->deleteLater();
}
