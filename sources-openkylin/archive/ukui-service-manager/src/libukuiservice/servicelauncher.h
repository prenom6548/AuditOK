#ifndef SERVICELAUNCHER_H
#define SERVICELAUNCHER_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QHash>

class ServiceLauncher : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.serviceManager.Launcher")
public:
    explicit ServiceLauncher(QObject *parent = nullptr);

signals:

public slots:
    virtual bool launchServices(const QString &pluginPath, const QStringList &servicesName, bool resident, const QStringList &args = QStringList());

protected:
    QHash<QString, QDBusConnection> connections;
};

#endif // SERVICELAUNCHER_H
