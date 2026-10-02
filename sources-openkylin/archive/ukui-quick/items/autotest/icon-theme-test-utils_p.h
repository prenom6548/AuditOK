/*
 * Copyright (C) 2026, Kylin Software Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef UKUI_QUICK_ITEMS_AUTOTEST_ICON_THEME_TEST_UTILS_P_H
#define UKUI_QUICK_ITEMS_AUTOTEST_ICON_THEME_TEST_UTILS_P_H

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QImage>
#include <QTemporaryDir>

namespace UkuiQuick {
namespace Test {

class IconThemeScope
{
public:
    IconThemeScope()
        : m_themeName(QIcon::themeName())
        , m_searchPaths(QIcon::themeSearchPaths())
    {
    }

    ~IconThemeScope()
    {
        QIcon::setThemeName(m_themeName);
        QIcon::setThemeSearchPaths(m_searchPaths);
    }

private:
    QString m_themeName;
    QStringList m_searchPaths;
};

inline bool writeThemeIcon(QTemporaryDir *directory,
                           const QString &themeName,
                           const QString &iconName,
                           const QColor &color)
{
    if (!directory || !directory->isValid()) {
        return false;
    }

    const QString themeRoot = directory->filePath(themeName);
    const QString iconDirectory = themeRoot + QStringLiteral("/48x48/apps");
    if (!QDir().mkpath(iconDirectory)) {
        return false;
    }

    QFile index(themeRoot + QStringLiteral("/index.theme"));
    const QByteArray indexContents = QStringLiteral(
        "[Icon Theme]\n"
        "Name=%1\n"
        "Directories=48x48/apps\n"
        "\n"
        "[48x48/apps]\n"
        "Size=48\n"
        "Context=Applications\n"
        "Type=Fixed\n")
        .arg(themeName)
        .toUtf8();
    if (!index.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        index.write(indexContents) != indexContents.size()) {
        return false;
    }

    QImage image(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    return image.save(iconDirectory + QLatin1Char('/') + iconName +
                      QStringLiteral(".png"));
}

} // namespace Test
} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_AUTOTEST_ICON_THEME_TEST_UTILS_P_H
