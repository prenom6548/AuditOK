#ifndef SESSIONSERVICELAUNCHER_H
#define SESSIONSERVICELAUNCHER_H

#include "servicelauncher.h"

class SessionServiceLauncher : public ServiceLauncher
{
public:
    explicit SessionServiceLauncher(QObject *parent = nullptr);

    bool launchServices(const QString &pluginPath, const QStringList &serviceNames, bool resident, const QStringList &args = QStringList()) override;
};

#endif // SESSIONSERVICELAUNCHER_H
