#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include <QtPlugin>

#define UKUIServicePluginInterface_iid "org.ukui.serviceManager.ServiceInterface"

namespace UKUI {

class ServiceObject;

class ServiceInterface
{
public:
    virtual ~ServiceInterface() {}

    virtual QList<ServiceObject *> getServiceObjects() = 0;
};
}

Q_DECLARE_INTERFACE (UKUI::ServiceInterface, UKUIServicePluginInterface_iid)

#endif // SERVICEINTERFACE_H
