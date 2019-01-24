#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <QString>

class Utilities
{
 public:
    static QString getDataPath();
    static QString newSavedVideoName();
    static QString getSavedVideoPath(QString name, QString postfix);
    static void notifyMobile(int cameraID);
};

#endif
