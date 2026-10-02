// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QGSettings>
#include <QVariant>

#include "Gsettings.h"

#define GSETTINGS_THEME_NAME "org.ukui.style"
#define GSETTINGS_THEME_KEY "styleName"
#define GSETTINGS_HIGHLIGHT_KEY "theme-color"

GSettingsManager::GSettingsManager(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled(GSETTINGS_THEME_NAME)) {
        m_settingsTheme = new QGSettings(GSETTINGS_THEME_NAME);
        connect(m_settingsTheme, &QGSettings::changed, this, &GSettingsManager::themeChanged);
    }
    themeChanged();
}

GSettingsManager::~GSettingsManager()
{
    if (m_settingsTheme) {
        delete m_settingsTheme;
        m_settingsTheme = nullptr;
    }
}

QColor GSettingsManager::borderColor() const
{
    return m_borderColor;
}

void GSettingsManager::setBorderColor(const QColor &color)
{
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
    }
}

QColor GSettingsManager::backgroundColor() const
{
    return m_backgroundColor;
}

void GSettingsManager::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
    }
}

QColor GSettingsManager::themeColor() const
{
    return m_themeColor;
}

void GSettingsManager::setThemeColor(const QColor &color)
{
    if (m_themeColor != color) {
        m_themeColor = color;
        emit themeColorChanged();
    }
}

QColor GSettingsManager::highLight() const
{
    return m_highLight;
}

void GSettingsManager::setHighLight(const QColor &color)
{
    if (m_highLight != color) {
        m_highLight = color;
        emit highLightChanged();
    }
}

QColor GSettingsManager::textColor() const
{
    return m_textColor;
}

void GSettingsManager::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        emit textColorChanged();
    }
}

void GSettingsManager::themeChanged()
{
    bool isDark = false;
    QString highLight;
    if (m_settingsTheme) {
        QString themeName = m_settingsTheme->get(GSETTINGS_THEME_KEY).toString();
        isDark = themeName.contains("dark", Qt::CaseInsensitive) ||
                 themeName.contains("black", Qt::CaseInsensitive);

        highLight = m_settingsTheme->get(GSETTINGS_HIGHLIGHT_KEY).toString();
    }

    int highLight_r = 18;
    int highLight_g = 18;
    int highLight_b = 18;
    int highLight_a = 1;
    if (!highLight.isEmpty()) {
        QStringList list = highLight.split(",");
        if (list.size() >= 4) {
            highLight_r = list[0].trimmed().toInt();
            highLight_g = list[1].trimmed().toInt();
            highLight_b = list[2].trimmed().toInt();
            double alphaRatio = list[3].trimmed().toDouble();
            highLight_a = static_cast<int>(alphaRatio * 255);
        }
    }

    QColor themeColor = isDark ? QColor(18, 18, 18) : QColor(255, 255, 255);
    setThemeColor(themeColor);

    QColor backgroundColor = isDark ? QColor(30, 30, 30) : QColor(245, 245, 245);
    setBackgroundColor(backgroundColor);

    QColor borderColor = isDark ? QColor(200, 195, 195) : QColor(131, 129, 129);
    setBorderColor(borderColor);

    QColor textColor = isDark ? QColor(255, 255, 255) : QColor(18, 18, 18);
    setTextColor(textColor);

    QColor highLightColor = QColor(highLight_r, highLight_g, highLight_b, highLight_a);
    setHighLight(highLightColor);
}
