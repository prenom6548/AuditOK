#ifndef SYSTEMSERVICELAUNCHER_H
#define SYSTEMSERVICELAUNCHER_H

#include "servicelauncher.h"

class SystemServiceLauncher : public ServiceLauncher
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.serviceManager.Launcher")
public:
    explicit SystemServiceLauncher(QObject *parent = nullptr);

    bool launchServices(const QString &pluginPath, const QStringList &serviceNames, bool resident, const QStringList &args = QStringList()) override;
};

#endif // SYSTEMSERVICELAUNCHER_H
