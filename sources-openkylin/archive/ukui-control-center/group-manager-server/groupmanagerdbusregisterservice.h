#ifndef GROUPMANAGERDBUSREGISTERSERVICE_H
#define GROUPMANAGERDBUSREGISTERSERVICE_H

#include <QObject>
#include <ukui-service-interface/serviceinterface.h>

class GroupManagerdbusRegisterService : public QObject, public UKUI::ServiceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUIServicePluginInterface_iid FILE "metadata.json")
    Q_INTERFACES(UKUI::ServiceInterface)

public:
    explicit GroupManagerdbusRegisterService(QObject *parent = nullptr);
    QList<UKUI::ServiceObject *> getServiceObjects();
signals:

};

#endif // GROUPMANAGERDBUSREGISTERSERVICE_H
