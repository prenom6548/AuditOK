// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QRegularExpression>
#include <QTextStream>

#include "IconUtils.h"

#define ICON_PATHS "~/.icons:~/.local/share/icons:/usr/share/icons:" EXTRA_ICON_PATH
#define APP_PATHS                                                                                  \
    "~/.local/share/applications:/usr/local/share/applications:"                                   \
    "/usr/share/applications:/etc/xdg/autostart:" EXTRA_APPS_PATH
#define PIXMAP_PATHS "/usr/share/pixmaps:" EXTRA_PIXMAP_PATH

IconUtils &IconUtils::instance()
{
    static IconUtils loader;
    return loader;
}

QString IconUtils::expandPath(const QString &path)
{
    if (path.startsWith("~/")) {
        return QDir::homePath() + path.mid(1);
    }
    return path;
}

QStringList IconUtils::expandPaths(const QString &paths)
{
    static QHash<QString, QStringList> cache;
    auto it = cache.constFind(paths);
    if (it != cache.constEnd()) {
        return it.value();
    }

    QStringList result;
    const QStringList parts = paths.split(":", Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        QString expanded = expandPath(part);
        if (!expanded.isEmpty()) {
            result.append(expanded);
        }
    }
    result.removeDuplicates();
    cache.insert(paths, result);
    return result;
}

void IconUtils::forEachRecursive(const QString &path, int baseLen, const FileIterator &iterator)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QString subPath;
    if (path.length() > baseLen) {
        subPath = path.mid(baseLen + 1);
    }

    if (iterator(path, subPath, QString())) {
        return;
    }

    const QFileInfoList entries =
        dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QFileInfo &fi : entries) {
        QString fullPath = fi.filePath();

        if (fi.isDir()) {
            forEachRecursive(fullPath, baseLen, iterator);
        } else if (fi.isFile()) {
            if (iterator(fullPath, subPath, fi.fileName())) {
                return;
            }
        }
    }
}

void IconUtils::forEachInDirs(const QString &dirs, const FileIterator &iterator)
{
    const QStringList pathList = expandPaths(dirs);
    for (const QString &basePath : pathList) {
        forEachRecursive(basePath, basePath.length() + 1, iterator);
    }
}

IconUtils::IconType IconUtils::iconTypeFromFileName(const QString &fileName, QString *baseName)
{
    QString suffix = QFileInfo(fileName).suffix();
    if (suffix.length() != 3) {
        return IconTypeUnknown;
    }

    IconType type = IconTypeUnknown;

    if (suffix.compare("svg", Qt::CaseInsensitive) == 0) {
        type = IconTypeSvg;
    } else if (suffix.compare("png", Qt::CaseInsensitive) == 0) {
        type = IconTypePng;
    } else if (suffix.compare("xpm", Qt::CaseInsensitive) == 0) {
        type = IconTypeXpm;
    } else if (suffix.compare("ico", Qt::CaseInsensitive) == 0) {
        type = IconTypeIco;
    } else {
        return type;
    }

    if (baseName) {
        *baseName = fileName.startsWith("/")
                        ? fileName
                        : fileName.left(fileName.length() - suffix.length() - 1);
    }

    return type;
}

void IconUtils::addImageToIcon(IconData &icon, const QString &path, IconType type)
{
    if (type == IconTypeUnknown || (icon.types & type)) {
        return;
    }

    switch (type) {
    case IconTypeSvg:
        icon.svgPath = path;
        break;
    case IconTypePng:
        icon.pngPath = path;
        break;
    case IconTypeXpm:
        icon.xpmPath = path;
        break;
    case IconTypeIco:
        icon.icoPath = path;
        break;
    default:
        return;
    }

    icon.types |= type;
}

bool IconUtils::loadIconFile(const QString &fullPath, const QString &subPath,
                             const QString &fileName)
{
    if (!fileName.isEmpty()) {
        QString iconName;
        IconType type = iconTypeFromFileName(fileName, &iconName);
        if (type != IconTypeUnknown) {
            addImageToIcon(m_icons[iconName], fullPath, type);
        }
        return false;
    }

    // apps/categories/status/devices
    if (!subPath.isEmpty()) {
        static const QRegularExpression sizeDirRegex(QStringLiteral("^(\\d+)x(\\d+)(?:@(\\d+))?"));
        if (sizeDirRegex.match(subPath).hasMatch() || subPath.startsWith("scalable")) {
            int slashPos = subPath.indexOf('/');
            if (slashPos >= 0) {
                QString afterSlash = subPath.mid(slashPos);
                if (!afterSlash.contains("apps") && !afterSlash.contains("categories") &&
                    !afterSlash.contains("status") && !afterSlash.contains("devices")) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool IconUtils::loadDesktopFile(const QString &fullPath, const QString &fileName)
{
    if (!fileName.endsWith(".desktop")) {
        return false;
    }
    QString desktopName = fileName.left(fileName.length() - 8);
    if (desktopName.isEmpty()) {
        return false;
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString iconName, startupName, appName, execName;

    bool inDesktopEntry = false;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        if (line.startsWith('[')) {
            inDesktopEntry = (line == QStringLiteral("[Desktop Entry]"));
            continue;
        }
        if (!inDesktopEntry) {
            continue;
        }

        int eqPos = line.indexOf('=');
        if (eqPos < 0) {
            continue;
        }

        QString key = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 1).trimmed();

        if (key == QStringLiteral("Icon")) {
            iconName = value;
        } else if (key == QStringLiteral("StartupWMClass")) {
            startupName = value;
        } else if (key == QStringLiteral("Name")) {
            appName = value;
        } else if (key == QStringLiteral("Exec")) {
            int lastSlash = value.lastIndexOf('/');
            if (lastSlash >= 0) {
                value = value.mid(lastSlash + 1);
            }
            int spacePos = value.indexOf(' ');
            if (spacePos >= 0) {
                value = value.left(spacePos);
            }
            execName = value;
        }

        if (!iconName.isEmpty() && !startupName.isEmpty() && !appName.isEmpty() &&
            !execName.isEmpty()) {
            break;
        }
    }

    file.close();

    if (iconName.isEmpty()) {
        return false;
    }

    m_desktopIconMap.insert(desktopName.toLower(), iconName);
    if (!startupName.isEmpty()) {
        m_desktopIconMap.insert(startupName.toLower(), iconName);
    }
    if (!appName.isEmpty()) {
        m_desktopIconMap.insert(appName.toLower(), iconName);
    }
    if (!execName.isEmpty()) {
        if (!m_desktopIconMap.contains(execName.toLower())) {
            m_desktopIconMap.insert(execName.toLower(), iconName);
        }
    }

    return true;
}

const IconUtils::IconData *IconUtils::findIcon(const QString &name) const
{
    auto it = m_icons.constFind(name);
    return (it != m_icons.constEnd()) ? &it.value() : nullptr;
}

QString IconUtils::findIconNameFromDesktop(const QString &name) const
{
    auto it = m_desktopIconMap.constFind(name.toLower());
    if (it != m_desktopIconMap.constEnd()) {
        return it.value();
    }
    return QString();
}

QString IconUtils::resolveIconPath(const IconData *icon)
{
    if (!icon) {
        return QString();
    }
    if (icon->types & IconTypeSvg) {
        return icon->svgPath;
    }
    if (icon->types & IconTypePng) {
        return icon->pngPath;
    }
    if (icon->types & IconTypeIco) {
        return icon->icoPath;
    }
    if (icon->types & IconTypeXpm) {
        return icon->xpmPath;
    }
    return QString();
}

QString IconUtils::resolveIconPathByName(const QString &name) const
{
    if (name.startsWith("/")) {
        return QFile::exists(name) ? name : QString();
    }

    QString iconName = findIconNameFromDesktop(name);
    if (!iconName.isEmpty()) {
        if (iconName.startsWith("/")) {
            if (QFile::exists(iconName)) {
                return iconName;
            }
        } else {
            const IconData *data = findIcon(iconName);
            if (data) {
                return resolveIconPath(data);
            }
        }
    }

    const IconData *data = findIcon(name);
    if (data) {
        return resolveIconPath(data);
    }

    return QString();
}

QString IconUtils::pixmapToDataUrl(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return QString();
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return QStringLiteral("data:image/png;base64,%1").arg(QString(byteArray.toBase64()));
}

QString IconUtils::iconToDataUrl(const QIcon &icon)
{
    if (icon.isNull()) {
        return QString();
    }
    return pixmapToDataUrl(icon.pixmap(48, 48));
}

QString IconUtils::fileToDataUrl(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return QString();
    }

    if (filePath.endsWith(".svg")) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            return QStringLiteral("data:image/svg+xml;base64,%1")
                .arg(QString(file.readAll().toBase64()));
        }
        return QString();
    }

    return pixmapToDataUrl(QPixmap(filePath));
}

void IconUtils::loadAll()
{
    if (m_loaded) {
        return;
    }

    qDebug() << "IconUtils: loading from paths:"
             << "\n  APP_PATHS:" << APP_PATHS << "\n  PIXMAP_PATHS:" << PIXMAP_PATHS
             << "\n  ICON_PATHS:" << ICON_PATHS;

    {
        auto desktopCb = [this](const QString &fullPath, const QString &subPath,
                                const QString &fileName) -> bool {
            if (!fileName.isEmpty() && fileName.endsWith(".desktop")) {
                loadDesktopFile(fullPath, fileName);
            }
            return false;
        };
        forEachInDirs(APP_PATHS, desktopCb);
    }
    qDebug() << "IconUtils: loaded" << m_desktopIconMap.size() << "desktop mappings";

    {
        auto iconCb = [this](const QString &fullPath, const QString &subPath,
                             const QString &fileName) -> bool {
            return loadIconFile(fullPath, subPath, fileName);
        };
        forEachInDirs(ICON_PATHS, iconCb);
        forEachInDirs(PIXMAP_PATHS, iconCb);
    }
    qDebug() << "IconUtils: loaded" << m_icons.size() << "icons";

    m_loaded = true;
}

QString IconUtils::getIconUrl(const QString &name)
{
    if (!m_loaded) {
        loadAll();
    }

    if (name.isEmpty()) {
        return QString();
    }

    auto iconUrl = m_iconUrlPaire.constFind(name);
    if (iconUrl != m_iconUrlPaire.constEnd()) {
        return iconUrl.value();
    }

    QString dataUrl;

    {
        QIcon icon = QIcon::fromTheme(name);
        if (!icon.isNull()) {
            dataUrl = iconToDataUrl(icon);
        }
    }

    if (dataUrl.isEmpty()) {
        QString iconPath = resolveIconPathByName(name);
        if (!iconPath.isEmpty()) {
            dataUrl = fileToDataUrl(iconPath);
        }
    }

    if (dataUrl.isEmpty()) {
        QIcon fallback = QIcon::fromTheme("application-x-desktop");
        dataUrl = iconToDataUrl(fallback);
    }

    m_iconUrlPaire.insert(name, dataUrl);
    return dataUrl;
}
