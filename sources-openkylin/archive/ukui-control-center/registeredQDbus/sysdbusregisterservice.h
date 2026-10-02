#ifndef SYSDBUSREGISTERSERVICE_H
#define SYSDBUSREGISTERSERVICE_H

#include <QObject>
#include <ukui-service-interface/serviceinterface.h>

class SysdbusRegisterService : public QObject, public UKUI::ServiceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUIServicePluginInterface_iid FILE "metadata.json")
    Q_INTERFACES(UKUI::ServiceInterface)

public:
    explicit SysdbusRegisterService(QObject *parent = nullptr);
    QList<UKUI::ServiceObject *> getServiceObjects();
signals:

};

#endif // SYSDBUSREGISTERSERVICE_H
