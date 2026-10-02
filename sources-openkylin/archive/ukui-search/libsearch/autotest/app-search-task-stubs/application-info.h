#ifndef APPLICATION_INFO_H
#define APPLICATION_INFO_H

#include <QObject>

#include "appdata/application-property.h"

namespace UkuiSearch {

class ApplicationInfo
{
public:
    explicit ApplicationInfo(QObject *parent = nullptr)
    {
        Q_UNUSED(parent)
    }

    QList<ApplicationInfoMap> searchAppWithOrder(ApplicationProperties properties,
                                                 const QStringList &keywords,
                                                 ApplicationPropertyMap restrictions)
    {
        Q_UNUSED(keywords)
        Q_UNUSED(restrictions)

        ApplicationPropertyMap first;
        if (properties.contains(ApplicationProperty::DesktopFilePath)) {
            first.insert(ApplicationProperty::DesktopFilePath, QStringLiteral("/usr/share/applications/demo.desktop"));
        }
        if (properties.contains(ApplicationProperty::LocalName)) {
            first.insert(ApplicationProperty::LocalName, QStringLiteral("Demo App"));
        }
        if (properties.contains(ApplicationProperty::Icon)) {
            first.insert(ApplicationProperty::Icon, QStringLiteral("demo-icon"));
        }

        ApplicationPropertyMap second;
        second.insert(ApplicationProperty::DesktopFilePath, QStringLiteral("/usr/share/applications/second.desktop"));
        second.insert(ApplicationProperty::LocalName, QStringLiteral("Second App"));
        second.insert(ApplicationProperty::Icon, QStringLiteral("second-icon"));

        return QList<ApplicationInfoMap>{
            ApplicationInfoMap{{QStringLiteral("/usr/share/applications/demo.desktop"), first}},
            ApplicationInfoMap{{QStringLiteral("/usr/share/applications/second.desktop"), second}}
        };
    }
};

} // namespace UkuiSearch

#endif // APPLICATION_INFO_H
