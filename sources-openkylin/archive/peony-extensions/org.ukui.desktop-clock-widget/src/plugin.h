#ifndef DESKTOPCLOCKBACKEND_PLUGIN_H
#define DESKTOPCLOCKBACKEND_PLUGIN_H

#include <QQmlExtensionPlugin>

class DesktopClockBackendPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif // DESKTOPCLOCKBACKEND_PLUGIN_H

