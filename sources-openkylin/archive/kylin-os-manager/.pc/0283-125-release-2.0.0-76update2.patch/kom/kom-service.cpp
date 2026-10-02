#include <QDBusConnectionInterface>
#include <QDebug>
#include <QFileInfo>
#include "kom-service.h"

namespace kom
{

const QStringList KomService::programWhiteList = {
#ifdef DEBUG_MODE
    "/usr/bin/python3.8",
#endif

    "/usr/bin/kylin-os-manager",
    "/usr/bin/hedron-client",
    "/usr/share/kylin-os-manager/kylin-os-manager-session-service/kylin-os-manager-session-service",
    "/usr/share/kylin-os-manager/treasure-chest/kylin-source-update-tool/kylin-source-update-tool",
    "/usr/share/kylin-os-manager/treasure-chest/kylin-icbc-tool/kylin-icbc-tool"
};

bool KomService::checkEnviron(int pid)
{

    QString filePath = QString("/proc/%1/environ").arg(pid);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "file.open failed" << filePath;
        return false;
    }
    QTextStream in(&file);
    QString data = in.readAll();
    file.close();
    data.replace('\0', '\n');
    QStringList environVars = data.split('\n');
    for (const QString &var : environVars) {
        if (!var.isEmpty()) {
            int pos = var.indexOf('=');
            if (pos != -1) {
                QString key = var.left(pos);
                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT") {
                    return false;
                }
            }
        }
    }
    return true;
}


} // namespace kom
