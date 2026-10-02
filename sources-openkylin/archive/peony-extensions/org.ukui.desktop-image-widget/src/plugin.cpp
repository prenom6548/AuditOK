#include "plugin.h"

#include "imagebackend.h"

#include <QtQml/qqml.h>

void DesktopImageBackendPlugin::registerTypes(const char *uri)
{
    Q_INIT_RESOURCE(desktopimage);

    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.desktopimage"));
    qmlRegisterType<ImageBackend>(uri, 1, 0, "ImageBackend");
}

