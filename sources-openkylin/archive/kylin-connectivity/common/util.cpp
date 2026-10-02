//
// Created by sqp on 2022/5/11.
//

#include "util.h"
#include <QFileInfo>
#include <unistd.h>
#include <QProcess>

namespace KCommon
{
QString getUserName()
{
    QStringList envVariables;
    envVariables << "USERNAME.*"
                 << "USER.*"
                 << "USERDOMAIN.*"
                 << "HOSTNAME.*"
                 << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    for (QString string : envVariables) {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2) {
                return stringList.at(1);
            }
        }
    }
    return "";
}

QString getHostName()
{
    char hostName[100] = {};
    gethostname(hostName, sizeof(hostName));
    return hostName;
}
} // namespace KCommon
