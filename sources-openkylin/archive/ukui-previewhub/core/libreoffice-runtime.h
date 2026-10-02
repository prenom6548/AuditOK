#ifndef LIBREOFFICERUNTIME_H
#define LIBREOFFICERUNTIME_H

#include <QString>
#include <QStringList>

namespace LibreOfficeRuntime
{

struct Command
{
    QString program;
    QStringList arguments;
};

bool isAvailable();
Command convertToPdfCommand(const QString &outDir, const QString &filePath);

}

#endif // LIBREOFFICERUNTIME_H
