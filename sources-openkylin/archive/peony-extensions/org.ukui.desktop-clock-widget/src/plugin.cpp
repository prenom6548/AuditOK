#include "plugin.h"

#include "clockbackend.h"

#include <QtQml/qqml.h>

void DesktopClockBackendPlugin::registerTypes(const char *uri)
{
    Q_INIT_RESOURCE(desktopclock);

    // 与 qmldir 中的 module 必须一致
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.desktopclock"));

    qmlRegisterType<ClockBackend>(uri, 1, 0, "ClockBackend");
}

