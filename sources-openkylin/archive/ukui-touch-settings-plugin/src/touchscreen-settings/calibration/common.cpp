#include "common.h"
#include <QtGlobal>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace {

QString sessionType()
{
    return qgetenv("XDG_SESSION_TYPE");
}

}

bool env::isWayland()
{
    return sessionType() == "wayland";
}

bool env::isTabletMode()
{
    QDBusInterface interface("com.kylin.statusmanager.interface",
                             "/",
                             "com.kylin.statusmanager.interface",
                             QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qWarning() << "Can not create interface for dbus com.kylin.statusmanager.interface";
        return false;
    }

    QDBusReply<bool> result = interface.call("get_current_tabletmode");
    return result.value();
}
