#ifndef SERVICEREGISTER_H
#define SERVICEREGISTER_H

#include <QObject>
#include <QDBusContext>

class ServiceRegister : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.serviceManager.Register")
public:
    explicit ServiceRegister(QObject *parent = nullptr);

signals:
    void serviceRegistered(const QString &name, int returnCode, const QString &errorMessages);

public slots:
    bool registerServiceByPlugin(const QString &pluginPath, const QStringList &args = QStringList());

protected:
    virtual bool registerServicesInternal(const QString &pluginPath, const QStringList &serviceNames, const QString &serviceSpace, bool resident, const QStringList &args = QStringList());
};

#endif // SERVICEREGISTER_H
