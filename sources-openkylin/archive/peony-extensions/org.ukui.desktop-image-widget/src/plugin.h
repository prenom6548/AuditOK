#ifndef DESKTOPIMAGEBACKEND_PLUGIN_H
#define DESKTOPIMAGEBACKEND_PLUGIN_H

#include <QQmlExtensionPlugin>

class DesktopImageBackendPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif // DESKTOPIMAGEBACKEND_PLUGIN_H

