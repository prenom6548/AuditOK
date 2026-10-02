#ifndef UKCCSESSIONSERVICE_H
#define UKCCSESSIONSERVICE_H

#include <QObject>
#include <ukui-service-interface/serviceinterface.h>

class ukccSessionService : public QObject, public UKUI::ServiceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUIServicePluginInterface_iid FILE "metadata.json")
    Q_INTERFACES(UKUI::ServiceInterface)

public:
    explicit ukccSessionService(QObject *parent = nullptr);
    QList<UKUI::ServiceObject *> getServiceObjects();

signals:

};

#endif // UKCCSESSIONSERVICE_H
