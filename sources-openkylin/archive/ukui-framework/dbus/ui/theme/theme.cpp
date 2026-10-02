/*
 * UKUI Framework
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "theme.h"

#include <QProcess>
#include <QStringList>

#include "qgsettings.h"

#undef signals
#include <gio/gio.h>


const QStringList g_themes = {"Light-Seeking", "HeYin", "Classic"};

const QStringList g_widget_theme_names = {"default", "fashion"};

const QStringList g_iconThemes = {"ukui-icon-theme-default", "ukui-icon-theme-fashion", "ukui-icon-theme-classical"};

const QStringList g_cursorThemes = {"dark-sense", "DMZ-White", "blue-crystal"};

const QStringList g_soundThemes = {"Light-Seeking", "HeYin", "Classic"};

static bool g_selfAdaptedMode = false;

bool g_themeServiceAvailable = true;
using namespace  UKUI;
// 临时切换实现，需要将GlobalTheme的机制从控制面板中迁移到后端服务，通过后端服务进行切换
void Theme::unstableSwitchGlobalTheme(int themeIndex) {
    QString wallpaperPath;
    QString iconThemeName;
    QString qtColor;
    QString gtkColor;
    QString qtWidgetTheme;
    QString qtStyleName;
    QString gtkStyleName;
    QString cursorTheme;
    QString windowRadius;

    switch (themeIndex) {
    case 0: {
        wallpaperPath = "/usr/share/backgrounds/1-openkylin.jpg";
        iconThemeName = "ukui-icon-theme-default";
        qtColor = "55,144,250,1";
        gtkColor = "#3790FA";
        qtStyleName = "ukui-light";
        qtWidgetTheme = "default";
        gtkStyleName = "ukui-light";
        cursorTheme = "dark-sense";
        windowRadius = "12,8";
        break;
    }
    case 1: {
        wallpaperPath = "/usr/share/backgrounds/heyin.png";
        iconThemeName = "ukui-icon-theme-fashion";
        qtColor = "120,115,245,1";
        gtkColor = "#7873F5";
        qtStyleName = "ukui-dark";
        qtWidgetTheme = "fashion";
        gtkStyleName = "ukui-black";
        cursorTheme = "DMZ-White";
        windowRadius = "12,8";
        break;
    }
    case 2: {
        wallpaperPath = "/usr/share/backgrounds/water.jpg";
        iconThemeName = "ukui-icon-theme-classical";
        qtColor = "daybreakBlue";
        gtkColor = "#3790FA";
        qtStyleName = "ukui-light";
        qtWidgetTheme = "classical";
        gtkStyleName = "ukui-white";
        cursorTheme = "dark-sense";
        windowRadius = "0,0";
        break;
    }
    default:
        return;
    }
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME, qtWidgetTheme);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_STYLE_NAME, qtStyleName);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_ICON_NAME, iconThemeName);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_THEME_COLOR, qtColor);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS, windowRadius);

    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME, qtWidgetTheme);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_STYLE_NAME, qtStyleName);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_ICON_NAME, iconThemeName);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_THEME_COLOR, qtColor);
    m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS, windowRadius);

    m_pMateGsettingsManager->set(GTK_SETTINGS_KEY_GTK_THEME, gtkStyleName);
    m_pMateGsettingsManager->set(GTK_SETTINGS_KEY_GTK_ICON, iconThemeName);

//    m_pInterfaceGsettingsManager->set(UKUI_SETTINGS_PRIMARY_COLOR, gtkColor);

    m_pBackgroundGsettingsManager->set(BACKGROUND_PICTRURE_FILENAME, wallpaperPath);

    m_pMouseGsettingsManager->set(UKUI_MOUSE_KEY_CURSOR_THEME, cursorTheme);
}

Theme::Theme(QObject *parent):
    QObject(parent)
{
    if (!QGSettings::isSchemaInstalled(qPrintable(GLOBAL_SETTINGS_SCHEMA_ID)) || !QGSettings::isSchemaInstalled(qPrintable(QT_SETTINGS_SCHEMA_ID))
            || !QGSettings::isSchemaInstalled(qPrintable(GAMMA_MANAGER_SCHEMA)) || !QGSettings::isSchemaInstalled(qPrintable(GTK_SETTINGS_SCHEMA_ID)) ||
            !QGSettings::isSchemaInstalled(qPrintable(CURSOR_THEME_SCHEMA)) || !QGSettings::isSchemaInstalled(qPrintable(KEY_SOUNDS_SCHEMA))) {
        g_themeServiceAvailable = false;
        return;
    }
    //org.ukui.interface no install.
//    m_pInterfaceGsettingsManager = new AbstractGsettingsManager(this, UKUI_SETTINGS_SCHEMA_ID, m_interfaceSettingsChangedHandlers);

    m_pGlobalThemeGsettingsManager = new GsettingsManager(this, GLOBAL_SETTINGS_SCHEMA_ID, globalThemeSetChangedHandler());//globalThemeSettings
    GlobalThemeName = m_pGlobalThemeGsettingsManager->get(GLOBAL_SETTINGS_KEY_THEME_NAME).toString();
    UserCustomized = m_pGlobalThemeGsettingsManager->get(GLOBAL_SETTINGS_KEY_MODIFIED).toBool();


    m_pstyleGsettingsManager = new GsettingsManager(this, QT_SETTINGS_SCHEMA_ID, styleChangedHandler());//selfAdaptedSettings
    QtStyleName = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME).toString();
    IconThemeName = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_ICON_NAME).toString();
    WindowRadius = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS).toString();
    QtThemeColor = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_THEME_COLOR).toString();

    m_pgammaManagerGsettingsManager = new GsettingsManager(this, GAMMA_MANAGER_SCHEMA, gammaManagerChangedHandler());//
    g_selfAdaptedMode = m_pgammaManagerGsettingsManager->get(GAMMA_MANAGER__KEY_AUTO_THEME).toBool();


    m_pMateGsettingsManager = new GsettingsManager(this, GTK_SETTINGS_SCHEMA_ID, mateSetChangedHandler());//gtkSettings
    GtkStyleName = m_pMateGsettingsManager->get(GTK_SETTINGS_KEY_GTK_THEME).toString();


    m_pMouseGsettingsManager = new GsettingsManager(this, CURSOR_THEME_SCHEMA, mouseChangedHandler());//cursorSettings
    CursorThemeName = m_pMouseGsettingsManager->get(UKUI_MOUSE_KEY_CURSOR_THEME).toString();


    m_psoundGSettingsManager = new GsettingsManager(this, KEY_SOUNDS_SCHEMA, soundChangedHandler()); //soundSettings
    SoundThemeName = m_psoundGSettingsManager->get(UKUI_SOUND_KEY_THEME_NAME).toString();

    m_pBackgroundGsettingsManager = new GsettingsManager(this, MATE_SETTINGS_SCHEMA_ID, backgroundChangedHandler());
    BackgroundPicture = m_pBackgroundGsettingsManager->get(BACKGROUND_PICTRURE_FILENAME).toString();
}

Theme::~Theme()
{

}

bool Theme::IsUserCustomized()
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
    }
    return UserCustomized;
}

QStringList Theme::GetStyleThemeList()
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
    }
    // fixme: 从后台获取全局主题
    return g_themes;
}

bool Theme::SetGlobalThemeName(const QString &name)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    if (g_themes.contains(name)) {
        // fixme: 后台响应全局主题切换
        unstableSwitchGlobalTheme(g_themes.indexOf(name));
        m_pGlobalThemeGsettingsManager->set(GLOBAL_SETTINGS_KEY_THEME_NAME, name);//g_settings_set_string(settings, "global-theme-name", name.toLocal8Bit().constData());
        setUserModified(false);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid theme name: %1, avaliable names are %2").arg(name).arg(g_themes.join(" ")));
        }
        return false;
    }
    return true;
}

bool Theme::SetThemeMode(int themeMode)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    // 如果不是自适应模式，则需要重新设置当前主题自动切换功能
    if (themeMode != 2) {
        m_pgammaManagerGsettingsManager->set(GAMMA_MANAGER__KEY_AUTO_THEME, false);
        g_selfAdaptedMode = false;
    } else {
        g_selfAdaptedMode = true;
    }

    switch (ThemeModeEnum(themeMode)) {
    case Light: {
        m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_STYLE_NAME, "ukui-light");
        m_pstyleGsettingsManager->set(GTK_SETTINGS_KEY_GTK_THEME, "ukui-white");
        break;
    }
    case Dark: {
        m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_STYLE_NAME, "ukui-dark");
        m_pstyleGsettingsManager->set(GTK_SETTINGS_KEY_GTK_THEME, "ukui-black");
        break;
    }
    case SelfAdapted: {
        m_pgammaManagerGsettingsManager->set(GAMMA_MANAGER__KEY_AUTO_THEME, true);
        break;
    }
    default:
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid Mode: %1").arg(themeMode));
        }
        return false;
    }


    setUserModified(true);

    return true;
}

bool Theme::SetThemeColor(const QString &hexColor)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    if (!hexColor.isEmpty()) {
        //Fix Me: need add check for hexColor
        m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_THEME_COLOR, hexColor);
        setUserModified(true);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::NotSupported, tr("Not supported yet."));
        }
        return false;
    }

    return true;
}

QStringList Theme::GetIconThemeList()
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
    }
    // fixme: 从后台获取图标主题
    return g_iconThemes;
}

bool Theme::SetIconThemeName(const QString &iconThemeName)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    if (g_iconThemes.contains(iconThemeName)) {
        m_pstyleGsettingsManager->set(UKUI_STYLE_SETTINGS_KEY_ICON_NAME, iconThemeName);
        m_pMateGsettingsManager->set(GTK_SETTINGS_KEY_GTK_ICON, iconThemeName);
        //成功再设置
        setUserModified(true);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid theme name: %1, avaliable names are %2").arg(iconThemeName).arg(g_iconThemes.join(" ")));
        }
        return false;
    }
    return true;
}

QStringList Theme::GetCursorThemeList()
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
    }
    // fixme: 从后台获取支持的光标主题
    return g_cursorThemes;
}

bool Theme::SetCursorThemeName(const QString &cursorThemeName)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    if (g_cursorThemes.contains(cursorThemeName)) {
        m_pMouseGsettingsManager->set(UKUI_MOUSE_KEY_CURSOR_THEME, cursorThemeName);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid theme name: %1, avaliable names are %2").arg(cursorThemeName).arg(g_cursorThemes.join(" ")));
        }
        return false;
    }
    return true;
}

QStringList Theme::GetSoundThemeList()
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
    }

    // fixme: 从后台获取支持的声音主题
    return g_soundThemes;
}

bool Theme::SetSoundThemeName(const QString &soundThemeName)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    if (g_soundThemes.contains(soundThemeName)) {
        m_psoundGSettingsManager->set(UKUI_SOUND_KEY_THEME_NAME, soundThemeName);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid theme name: %1, avaliable names are %2").arg(soundThemeName).arg(g_soundThemes.join(" ")));
        }
        return false;
    }
    return true;
}

bool Theme::SetBackgroundPicture(const QString &backgroundPicture)
{
    if (!g_themeServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Theme service is not available"));
        }
        return false;
    }

    QMimeDatabase mimeDb;
    QMimeType mime = mimeDb.mimeTypeForFile(backgroundPicture);
    if (mime.name().startsWith("image/")) {
         m_pBackgroundGsettingsManager->set(BACKGROUND_PICTRURE_FILENAME, backgroundPicture);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid theme name: %1, is't picture").arg(backgroundPicture));
        }
        return false;
    }
 
    return true;
}

/**
 * @brief setUserModified
 * 更新主题自定义状态，在用户设置全局主题后设置为false，
 * 设置其它主题配置是设置为true
 * @param modified
 */
void Theme::setUserModified(bool modified)
{
    m_pGlobalThemeGsettingsManager->set(GLOBAL_SETTINGS_KEY_MODIFIED, modified);
}

void Theme::initThemeMode()
{
    // 判断自适应模式
    if (g_selfAdaptedMode) {
        if (ThemeMode != SelfAdapted) {
            ThemeMode = SelfAdapted;
            emit ThemeModeChanged();
        }
        return;
    }
    QString qtColorSchemeName = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_STYLE_NAME).toString();
    if (qtColorSchemeName == "ukui-dark") {
        ThemeMode = Dark;
    } else {
        ThemeMode = Light;
    }
}

QHash<QString, OnKeyChanged> Theme::gammaManagerChangedHandler()
{
     initThemeMode();

    return m_gammaManagerChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::globalThemeSetChangedHandler()
{
    m_globalThemeSettingsChangedHandlers[GLOBAL_SETTINGS_KEY_THEME_NAME] = [this]{
        GlobalThemeName = m_pGlobalThemeGsettingsManager->get(GLOBAL_SETTINGS_KEY_THEME_NAME).toString();
        emit GlobalThemeNameChanged();
    };

    m_globalThemeSettingsChangedHandlers[GLOBAL_SETTINGS_KEY_MODIFIED] = [this]{
        UserCustomized = m_pGlobalThemeGsettingsManager->get(GLOBAL_SETTINGS_KEY_MODIFIED).toBool();
        emit UserCustomizedChanged();
    };

    return m_globalThemeSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::styleChangedHandler()
{
    m_styleSettingsChangedHandlers[UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME] = [this]{
        QtStyleName = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME).toString();
        emit QtStyleNameChanged();
    };

    m_styleSettingsChangedHandlers[UKUI_STYLE_SETTINGS_KEY_STYLE_NAME] = [this]{
        initThemeMode();
        emit ThemeModeChanged();
    };

    m_styleSettingsChangedHandlers[UKUI_STYLE_SETTINGS_KEY_ICON_NAME] = [this]{
        IconThemeName = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_ICON_NAME).toString();
        emit IconThemeNameChanged();
    };

    m_styleSettingsChangedHandlers[UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS] = [this]{
        WindowRadius = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS).toInt();
        emit WindowRadiusChanged();
    };

    m_styleSettingsChangedHandlers[UKUI_STYLE_SETTINGS_KEY_THEME_COLOR] = [this]{
        QtThemeColor = m_pstyleGsettingsManager->get(UKUI_STYLE_SETTINGS_KEY_THEME_COLOR).toInt();
        emit QtThemeColorChanged();
    };
    return m_styleSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::mateSetChangedHandler()
{
     m_mateSettingsChangedHandlers[GTK_SETTINGS_KEY_GTK_THEME] = [this] {
        GtkStyleName = m_pMateGsettingsManager->get(GTK_SETTINGS_KEY_GTK_THEME).toString();
        emit GtkStyleNameChanged();
    };
    return m_mateSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::mouseChangedHandler()
{
    m_mouseSettingsChangedHandlers[UKUI_MOUSE_KEY_CURSOR_THEME] = [this] {
        CursorThemeName = m_pMouseGsettingsManager->get(UKUI_MOUSE_KEY_CURSOR_THEME).toString();
        emit CursorThemeNameChanged();
    };
    return m_mouseSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::soundChangedHandler()
{
    m_soundSettingsChangedHandlers[UKUI_SOUND_KEY_THEME_NAME] = [this] {
        SoundThemeName = m_psoundGSettingsManager->get(UKUI_SOUND_KEY_THEME_NAME).toString();
        emit SoundThemeNameChanged();
    };
    return m_soundSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Theme::backgroundChangedHandler()
{
    m_backgroundSettingsChangedHandlers[MATE_SETTINGS_SCHEMA_ID] = [this] {
        BackgroundPicture = m_pBackgroundGsettingsManager->get(BACKGROUND_PICTRURE_FILENAME).toString();
        emit BackgroundPictureChanged();
    };
    return m_backgroundSettingsChangedHandlers;
}

int Theme::GetThemeMode() const
{
    return ThemeMode;
}

bool Theme::SetWindowRadius(QString newWindowRadius)
{
    if (WindowRadius == newWindowRadius)
        return false;
    WindowRadius = newWindowRadius;
    emit WindowRadiusChanged();

    return true;
}

const QString Theme::GetWindowRadius() const
{
    return WindowRadius;
}

const QString Theme::GetGlobalThemeName() const
{
    return GlobalThemeName;
}

const QString Theme::GetSoundThemeName() const
{
//    SoundThemeName = m_psoundGSettingsManager->get(UKUI_SOUND_KEY_THEME_NAME).toString();
    return SoundThemeName;
}

const QString Theme::GetCursorThemeName() const
{
    return CursorThemeName;
}

const QString Theme::GetIconThemeName() const
{
    return IconThemeName;
}

const QString Theme::GetGtkStyleName() const
{
    return GtkStyleName;
}

const QString Theme::GetThemeColor() const
{
    return ThemeColor;
}

const QString Theme::GetQtStyleName() const
{
    return QtStyleName;
}

const QString Theme::GetQtThemeColor() const
{
    return QtThemeColor;
}

const QString Theme::GetBackgroundPicture() const
{
    return BackgroundPicture;
}
