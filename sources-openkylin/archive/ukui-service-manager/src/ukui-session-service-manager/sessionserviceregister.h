#ifndef SESSIONSERVICEREGISTER_H
#define SESSIONSERVICEREGISTER_H

#include <QObject>

#include "serviceregister.h"

class SessionServiceRegister : public ServiceRegister
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.serviceManager.Register")
public:
    explicit SessionServiceRegister(QObject *parent = nullptr);

protected:
    bool registerServicesInternal(const QString &pluginPath, const QStringList &serviceNames, const QString &serviceSpace, bool resident, const QStringList &args = QStringList()) override;
};

#endif // SESSIONSERVICEREGISTER_H
