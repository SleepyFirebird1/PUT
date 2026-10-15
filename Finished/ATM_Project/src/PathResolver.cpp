#include "PathResolver.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <string>

using namespace std;

string resolvePath(const string& relativePath) {
    if (QFile::exists(QString::fromStdString(relativePath))) {
        return relativePath;
    }

    QString execPath = QCoreApplication::applicationDirPath();
    QString shadowPath = QDir(execPath).filePath("../../../../" + QString::fromStdString(relativePath));
    if (QFile::exists(shadowPath)) {
         return shadowPath.toStdString();
    }
    
    QString bundlePath = QDir(execPath).filePath(QString::fromStdString(relativePath));
    if (QFile::exists(bundlePath)) {
        return bundlePath.toStdString();
    }

    return relativePath; 
}
