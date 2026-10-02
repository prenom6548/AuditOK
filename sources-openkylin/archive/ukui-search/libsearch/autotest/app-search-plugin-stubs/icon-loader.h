#ifndef ICON_LOADER_STUB_H
#define ICON_LOADER_STUB_H

#include <QIcon>
#include <QPixmap>

namespace UkuiSearch {

class IconLoader
{
public:
    static QIcon loadIconQt(const QString &name, const QIcon &fallback = {})
    {
        Q_UNUSED(name)
        if (!fallback.isNull()) {
            return fallback;
        }
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::blue);
        return QIcon(pixmap);
    }

    static QIcon loadIconXdg(const QString &name, const QIcon &fallback = {})
    {
        Q_UNUSED(name)
        return loadIconQt({}, fallback);
    }
};

}

#endif
