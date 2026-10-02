#ifndef SYSTEMSERVICEREGISTER_H
#define SYSTEMSERVICEREGISTER_H

#include "serviceregister.h"

class SystemServiceRegister : public ServiceRegister
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.serviceManager.Register")
public:
    explicit SystemServiceRegister(QObject *parent = nullptr);

protected:
    bool registerServicesInternal(const QString &pluginPath, const QStringList &serviceNames, const QString &serviceSpace, bool resident, const QStringList &args = QStringList()) override;
};

#endif // SYSTEMSERVICEREGISTER_H
