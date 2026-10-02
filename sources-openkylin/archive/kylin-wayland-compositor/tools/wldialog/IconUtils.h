// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <functional>

#include <QHash>
#include <QIcon>
#include <QString>
#include <QStringList>

class IconUtils
{
  public:
    enum IconType {
        IconTypeUnknown = 0,
        IconTypeSvg = 1 << 0,
        IconTypePng = 1 << 1,
        IconTypeXpm = 1 << 2,
        IconTypeIco = 1 << 3,
    };

    struct IconData {
        QString svgPath;
        QString pngPath;
        QString xpmPath;
        QString icoPath;
        quint32 types = 0;
    };

    static IconUtils &instance();

    void loadAll();
    QString getIconUrl(const QString &name);

  private:
    IconUtils() = default;
    ~IconUtils() = default;
    IconUtils(const IconUtils &) = delete;
    IconUtils &operator=(const IconUtils &) = delete;

    static QStringList expandPaths(const QString &paths);
    static QString expandPath(const QString &path);

    static IconType iconTypeFromFileName(const QString &fileName, QString *baseName = nullptr);

    using FileIterator = std::function<bool(const QString &fullPath, const QString &subPath,
                                            const QString &fileName)>;
    static void forEachInDirs(const QString &dirs, const FileIterator &iterator);
    static void forEachRecursive(const QString &path, int baseLen, const FileIterator &iterator);

    bool loadIconFile(const QString &fullPath, const QString &subPath, const QString &fileName);
    static void addImageToIcon(IconData &icon, const QString &path, IconType type);

    bool loadDesktopFile(const QString &fullPath, const QString &fileName);

    const IconData *findIcon(const QString &name) const;
    QString findIconNameFromDesktop(const QString &name) const;

    static QString resolveIconPath(const IconData *icon);
    QString resolveIconPathByName(const QString &name) const;

    static QString iconToDataUrl(const QIcon &icon);
    static QString fileToDataUrl(const QString &filePath);
    static QString pixmapToDataUrl(const QPixmap &pixmap);

    // ICON_PATHS and PIXMAP_PATHS
    QHash<QString, IconData> m_icons;
    // APP_PATHS (desktop name / StartupWMClass / appName / execName) → iconName
    QHash<QString, QString> m_desktopIconMap;
    // (name → data URL)
    QHash<QString, QString> m_iconUrlPaire;

    bool m_loaded = false;
};

#endif // ICONUTILS_H
