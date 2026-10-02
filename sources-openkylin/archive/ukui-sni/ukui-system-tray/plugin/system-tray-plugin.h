#ifndef UKUI_PANEL_START_MENU_PLUGIN_H
#define UKUI_PANEL_START_MENU_PLUGIN_H

#include <QQmlExtensionPlugin>

class SystemTrayPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override;
};


#endif //UKUI_PANEL_START_MENU_PLUGIN_H
