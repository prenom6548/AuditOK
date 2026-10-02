#ifndef AI_SEARCH_PLUGIN_ICON_LOADER_STUB_H
#define AI_SEARCH_PLUGIN_ICON_LOADER_STUB_H

#include <QIcon>
#include <QString>

namespace UkuiSearch {

class IconLoader
{
public:
    static QIcon loadIconQt(const QString &name, const QIcon &fallback = {})
    {
        return name.isEmpty() ? fallback : QIcon::fromTheme(name, fallback);
    }

    static QIcon loadIconXdg(const QString &name, const QIcon &fallback = {})
    {
        return loadIconQt(name, fallback);
    }
};

}  // namespace UkuiSearch

#endif
