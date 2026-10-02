#include <QDebug>
#include <QIcon>
#include <QFile>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <qt5xdg/XdgIcon>
#else
#include <qt6xdg/XdgIcon>
#endif
#include "item-icon-provider.h"

QSize ItemIconProvider::m_defaultSize = QSize(56, 56);

ItemIconProvider::ItemIconProvider() : QQuickImageProvider(QQmlImageProviderBase::Pixmap)
{
}

QPixmap ItemIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return ItemIconProvider::getPixmap(id, size, requestedSize);
}

QPixmap ItemIconProvider::getPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    QPixmap pixmap;
    loadPixmap(id, pixmap);

    if (size) {
        QSize pixmapSize = pixmap.size();
        size->setWidth(pixmapSize.width());
        size->setHeight(pixmapSize.height());
    }
    return pixmap;
}

void ItemIconProvider::loadPixmap(const QString &id, QPixmap &pixmap)
{
    if (id.isEmpty()) {
        loadDefault(pixmap);
        return;
    }

    bool isOk;
    QUrl url(id);
    if (!url.scheme().isEmpty()) {
        isOk = loadPixmapFromUrl(url, pixmap);

    } else if (id.startsWith(QLatin1String("/")) || id.startsWith(QLatin1String(":/"))) {
        //qrc path: :/xxx/xxx.png
        isOk = loadPixmapFromPath(id, pixmap);

    } else {
        isOk = loadPixmapFromTheme(id, pixmap);
        if (!isOk) {
            isOk = loadPixmapFromXdg(id, pixmap);
        }
    }

    if (!isOk) {
        loadDefault(pixmap);
    }
}

void ItemIconProvider::loadDefault(QPixmap &pixmap)
{
    if (!loadPixmapFromTheme("application-x-desktop", pixmap)) {
        loadPixmapFromPath(":/test/icon/application-x-desktop.png", pixmap);
    }
}

bool ItemIconProvider::loadPixmapFromUrl(const QUrl &url, QPixmap &pixmap)
{
    QString path = url.path();
    if (path.isEmpty()) {
        qWarning() << "Error: loadPixmapFromUrl, path is empty";
        return false;
    }

    if (url.scheme() == QLatin1String("qrc")) {
        path.prepend(QLatin1String(":"));
        return loadPixmapFromPath(path, pixmap);
    }
    return false;
}

bool ItemIconProvider::loadPixmapFromPath(const QString &path, QPixmap &pixmap)
{
    if (!QFile::exists(path)) {
        qWarning() << "Error: loadPixmapFromPath, File dose not exists" << path;
        return false;
    }

    QPixmap filePixmap;
    bool isok = filePixmap.load(path);
    if (!isok) {
        return false;
    }

    pixmap.swap(filePixmap);
    return true;
}

bool ItemIconProvider::loadPixmapFromTheme(const QString &name, QPixmap &pixmap)
{
    if (!QIcon::hasThemeIcon(name)) {
        return false;
    }
    QIcon icon = QIcon::fromTheme(name);
    pixmap = icon.pixmap(ItemIconProvider::m_defaultSize);
    return true;
}

bool ItemIconProvider::loadPixmapFromXdg(const QString &name, QPixmap &pixmap)
{
    QIcon icon = XdgIcon::fromTheme(name);
    if (icon.isNull()) {
        qWarning() << "Error: loadPixmapFromXdg, icon dose not exists. name:" << name;
        return false;
    }

    pixmap = icon.pixmap(ItemIconProvider::m_defaultSize);
    return true;
}
