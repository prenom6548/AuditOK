#ifndef DEMOSERVICEPLUGIN_H
#define DEMOSERVICEPLUGIN_H

#include <QGenericPlugin>

#include "../ukuiserviceinterface/serviceinterface.h"

class DemoServicePlugin : public QObject, public UKUI::ServiceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUIServicePluginInterface_iid FILE "service-plugin-demo.json")
    Q_INTERFACES(UKUI::ServiceInterface)

public:
    explicit DemoServicePlugin(QObject *parent = nullptr);

    QList<UKUI::ServiceObject *> getServiceObjects() override;
};

#endif // DEMOSERVICEPLUGIN_H
