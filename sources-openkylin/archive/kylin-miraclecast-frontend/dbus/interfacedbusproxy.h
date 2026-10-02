#ifndef INTERFACEDBUSPROXY_H
#define INTERFACEDBUSPROXY_H

#include "interactiondbusinterface.h"

/*
 * brief dbus接口访问代理
*/
class interfaceDBusProxy
{
public:
    ~interfaceDBusProxy();

    inline static interfaceDBusProxy* instance(){
        if(!_instance)
            _instance = new interfaceDBusProxy();
        return _instance;
    }

    inline const char *getServiceName() const{
        return "org.freedesktop.miracleagent";
    }

    InteractionDBusInterface* _interface;

private:
    static interfaceDBusProxy* _instance;
    interfaceDBusProxy();
};

#define INTERACTIONDBUS interfaceDBusProxy::instance()->_interface

#endif // INTERFACEDBUSPROXY_H
