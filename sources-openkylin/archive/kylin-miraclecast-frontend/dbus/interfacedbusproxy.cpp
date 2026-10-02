#include "interfacedbusproxy.h"

interfaceDBusProxy* interfaceDBusProxy::_instance = nullptr;

interfaceDBusProxy::interfaceDBusProxy():_interface(nullptr)
{
    _interface = new InteractionDBusInterface(getServiceName(), QDBusConnection::sessionBus());
}

interfaceDBusProxy::~interfaceDBusProxy(){
    if(_interface)
        delete  _interface;
}
