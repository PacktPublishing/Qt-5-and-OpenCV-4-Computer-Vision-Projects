#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <QString>

class Utilities
{
 public:
    static QString getDataPath();
    static QString newPhotoName();
    static QString getPhotoPath(QString name, QString postfix);
};

#endif
