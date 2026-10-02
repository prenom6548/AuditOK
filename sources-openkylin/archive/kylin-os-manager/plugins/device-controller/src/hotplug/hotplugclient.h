#ifndef HOTPLUGCLIENT_H
#define HOTPLUGCLIENT_H

#include <QObject>
#include <QDBusContext>
#include "../hardwareinfo/hardwareinfo.h"

class HotplugClient : public QObject, protected QDBusContext {
Q_OBJECT

public:
    HotplugClient(QObject *parent = nullptr);

public slots:

    void plugInEvent(const QString jsonString);

    void plugoutEvent(const QString jsonString);

private:
    QList<HardWareInfo> convertJsonToHardWareList(const QString &jsonString);
};


#endif //HOTPLUGCLIENT_H
