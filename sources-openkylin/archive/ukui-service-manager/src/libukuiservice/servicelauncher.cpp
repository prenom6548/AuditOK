#include "servicelauncher.h"

ServiceLauncher::ServiceLauncher(QObject *parent)
    : QObject{parent}
{

}

bool ServiceLauncher::launchServices(const QString &pluginPath, const QStringList &servicesName, bool resident, const QStringList &args)
{
    return false;
}
