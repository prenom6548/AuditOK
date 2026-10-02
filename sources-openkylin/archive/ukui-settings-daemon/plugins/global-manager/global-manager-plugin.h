#ifndef GLOBALMANAGERPLUGIN_H
#define GLOBALMANAGERPLUGIN_H

#include "plugin-interface.h"
#include "global-manager.h"

class GlobalManagerPlugin: public PluginInterface
{
public:
    ~GlobalManagerPlugin();
    static PluginInterface *getInstance();
    virtual void activate();
    virtual void deactivate();

private:
    GlobalManagerPlugin();
    GlobalManagerPlugin(GlobalManagerPlugin&)=delete;

    static GlobalManager *m_pManager;
    static PluginInterface *m_Instance;
};

extern "C" Q_DECL_EXPORT PluginInterface* createSettingsPlugin();
#endif // GLOBALMANAGERPLUGIN_H
