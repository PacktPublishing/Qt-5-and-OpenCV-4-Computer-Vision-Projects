#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include "utilities.h"

QString Utilities::getDataPath()
{
    QString user_pictures_path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0];
    QDir pictures_dir(user_pictures_path);
    pictures_dir.mkpath("DiGauge");
    return pictures_dir.absoluteFilePath("DiGauge");
}

QString Utilities::newPhotoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getPhotoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}
