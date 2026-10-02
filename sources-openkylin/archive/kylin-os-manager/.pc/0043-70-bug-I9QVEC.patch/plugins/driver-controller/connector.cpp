#include "plugin.h"

extern "C" bool kyconnect(kyplugin::Host &host)
{
    host.add(new DriverControllerPlugin::DriverControllerPluginProvider());

    return true;
}
