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

#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QDBusContext>
#include "gsettings_manager.h"

class Theme : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Theme")

    Q_PROPERTY(bool UserCustomized READ IsUserCustomized NOTIFY UserCustomizedChanged)
    Q_PROPERTY(QString GlobalThemeName READ GetGlobalThemeName WRITE SetGlobalThemeName NOTIFY GlobalThemeNameChanged)
    Q_PROPERTY(int ThemeMode READ GetThemeMode NOTIFY ThemeModeChanged)
    Q_PROPERTY(QString QtStyleName READ GetQtStyleName NOTIFY QtStyleNameChanged)
    Q_PROPERTY(QString GtkStyleName READ GetGtkStyleName NOTIFY GtkStyleNameChanged)
    Q_PROPERTY(QString ThemeColor READ GetThemeColor WRITE SetThemeColor NOTIFY ThemeColorChanged)
    Q_PROPERTY(QString IconThemeName READ GetIconThemeName WRITE SetIconThemeName NOTIFY IconThemeNameChanged)
    Q_PROPERTY(QString CursorThemeName READ GetCursorThemeName WRITE SetCursorThemeName NOTIFY CursorThemeNameChanged)
    Q_PROPERTY(QString SoundThemeName READ GetSoundThemeName WRITE SetSoundThemeName NOTIFY SoundThemeNameChanged)
    Q_PROPERTY(QString WindowRadius READ GetWindowRadius WRITE SetWindowRadius NOTIFY WindowRadiusChanged)

public:
    /**
     * @brief The ThemeMode enum
     * 主题模式，分为
     * 深色、浅色、深浅混合、自适应
     * 四种模式
     */
    enum ThemeModeEnum {
        Light,
        Dark,
        SelfAdapted
    };
    Q_ENUM(ThemeModeEnum)

    explicit Theme(QObject *parent = nullptr);
    ~Theme();

public Q_SLOTS:
    /**
     * @brief isUserCustomed
     * 用户是否进行了主题自定义
     * @return
     * 是，用户修改过当前主题，如图标、壁纸等
     */
    Q_INVOKABLE bool IsUserCustomized();

    /**
     * @brief GetStyleThemeList
     * 获取支持的主题列表
     * @return
     * 主题列表，主题名称为非本地化主题名，
     * 例如"Light-Seeking"（寻光），
     * "HeYin"（和印），
     * "Classic"（启典）
     */
    Q_INVOKABLE QStringList GetStyleThemeList();

    /**
     * @brief SetGlobalThemeName
     * 设置主题，可选在GetStyleThemeList()中的主题
     * @param name
     * 主题名称，如"Light-Seeking"
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetGlobalThemeName(const QString &name);

    /**
     * @brief SetThemeMode
     * 设置主题模式
     * @param themeMode
     * @sa ThemeMode
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetThemeMode(int themeMode);

    /**
     * @brief SetThemeColor
     * 设置控件强调色
     * @param hexColor
     * 强调色，rgb hex格式
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetThemeColor(const QString &hexColor);

    /**
     * @brief GetIconThemeList
     * 获取支持的图标主题列表
     * @return
     * 如"ukui-icon-theme-default",
     * "ukui-icon-theme-fashion",
     * "ukui-icon-theme-classical"
     */
    Q_INVOKABLE QStringList GetIconThemeList();

    /**
     * @brief SetIconThemeName
     * 设置主题图标
     * @param iconThemeName
     * 在主题图标列表中的主题图标名
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetIconThemeName(const QString &iconThemeName);

    /**
     * @brief GetCursorThemeList
     * 获取光标主题列表
     * @return
     */
    Q_INVOKABLE QStringList GetCursorThemeList();

    /**
     * @brief SetCursorThemeName
     * 设置声音主题
     * @param cursorThemeName
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetCursorThemeName(const QString &cursorThemeName);

    /**
     * @brief GetSoundThemeList
     * 获取声音主题列表
     * @return
     */
    Q_INVOKABLE QStringList GetSoundThemeList();

    /**
     * @brief SetSoundThemeName
     * 设置声音主题
     * @param soundThemeName
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetSoundThemeName(const QString &soundThemeName);

    /**
     * @brief SetWindowRadius
     * 设置窗口圆角
     * @param radius
     * @return
     * 设置成功/失败
     */
    Q_INVOKABLE bool SetWindowRadius(QString newWindowRadius);

    Q_INVOKABLE bool SetBackgroundPicture(const QString &backgroundPicture);

    const QString GetGlobalThemeName() const;

    const QString GetQtStyleName() const;

    const QString GetGtkStyleName() const;

    const QString GetThemeColor() const;

    const QString GetIconThemeName() const;

    const QString GetCursorThemeName() const;

    const QString GetSoundThemeName() const;

    const QString GetQtThemeColor() const;

    const QString GetWindowRadius() const;

    int GetThemeMode() const;

    const QString GetBackgroundPicture() const;
signals:
    void ThemeModeChanged();
    void QtStyleNameChanged();
    void GtkStyleNameChanged();
    void ThemeColorChanged();
    void IconThemeNameChanged();
    void CursorThemeNameChanged();
    void SoundThemeNameChanged();
    void GlobalThemeNameChanged();
    void WindowRadiusChanged();
    void QtThemeColorChanged();
    void UserCustomizedChanged();
    void BackgroundPictureChanged();

private:

    void unstableSwitchGlobalTheme(int themeIndex);
    void setUserModified(bool modified);
    void initThemeMode();
private:

    QString GlobalThemeName;
    int ThemeMode = 0;
    QString QtStyleName;
    QString GtkStyleName;
    QString ThemeColor;
    QString IconThemeName;
    QString SystemFont;
    QString QtThemeColor;
    QString CursorThemeName;
    QString SoundThemeName;
    QString WindowRadius;
    QString BackgroundPicture;
    
    int SystemFontSize = 10;
    bool UserCustomized = false;

private:

    QHash<QString, OnKeyChanged> gammaManagerChangedHandler();
    QHash<QString, OnKeyChanged> m_gammaManagerChangedHandlers;
    GsettingsManager *m_pgammaManagerGsettingsManager = nullptr;
    const QString GAMMA_MANAGER_SCHEMA           =    "org.ukui.SettingsDaemon.plugins.color";
    const QString GAMMA_MANAGER__KEY_AUTO_THEME  =    "theme-schedule-automatic";

    QHash<QString, OnKeyChanged> interfaceChangedHandler();
    QHash<QString, OnKeyChanged> m_interfaceSettingsChangedHandlers;
    GsettingsManager *m_pInterfaceGsettingsManager = nullptr;
    const QString UKUI_SETTINGS_SCHEMA_ID = "org.ukui.interface";
    const QString UKUI_SETTINGS_PRIMARY_COLOR = "primary-color";


    QHash<QString, OnKeyChanged> globalThemeSetChangedHandler();
    QHash<QString, OnKeyChanged> m_globalThemeSettingsChangedHandlers;
    GsettingsManager *m_pGlobalThemeGsettingsManager = nullptr;

    const QString GLOBAL_SETTINGS_SCHEMA_ID =  "org.ukui.globaltheme.settings";
    const QString GLOBAL_SETTINGS_KEY_THEME_NAME = "global-theme-name";
    const QString GLOBAL_SETTINGS_KEY_MODIFIED = "is-modified";


    QHash<QString, OnKeyChanged> styleChangedHandler();
    QHash<QString, OnKeyChanged> m_styleSettingsChangedHandlers;
    GsettingsManager *m_pstyleGsettingsManager = nullptr;

    const QString QT_SETTINGS_SCHEMA_ID = "org.ukui.style";
    const QString UKUI_STYLE_SETTINGS_KEY_WIDGET_NAME = "widget-theme-name";
    const QString UKUI_STYLE_SETTINGS_KEY_STYLE_NAME = "style-name";
    const QString UKUI_STYLE_SETTINGS_KEY_ICON_NAME = "icon-theme-name";
    const QString UKUI_STYLE_SETTINGS_KEY_THEME_COLOR = "theme-color";
    const QString UKUI_STYLE_SETTINGS_KEY_SYSTEM_FONT = "system-font";
    const QString UKUI_STYLE_SETTINGS_KEY_SYSTEM_FONT_SIZE = "system-font-size";
    const QString UKUI_STYLE_SETTINGS_KEY_WINDOWS_RADIUS = "window-radius";



    QHash<QString, OnKeyChanged> mateSetChangedHandler();
    QHash<QString, OnKeyChanged> m_mateSettingsChangedHandlers;
    GsettingsManager *m_pMateGsettingsManager = nullptr;

    const QString GTK_SETTINGS_SCHEMA_ID = "org.mate.interface";
    const QString GTK_SETTINGS_KEY_GTK_THEME = "gtk-theme";
    const QString GTK_SETTINGS_KEY_GTK_ICON = "icon-theme";

    QHash<QString, OnKeyChanged> mouseChangedHandler();
    QHash<QString, OnKeyChanged> m_mouseSettingsChangedHandlers;
    GsettingsManager *m_pMouseGsettingsManager = nullptr;

    const QString CURSOR_THEME_SCHEMA = "org.ukui.peripherals-mouse";
    const QString UKUI_MOUSE_KEY_CURSOR_THEME = "cursor-theme";

    QHash<QString, OnKeyChanged> soundChangedHandler();
    QHash<QString, OnKeyChanged> m_soundSettingsChangedHandlers;
    GsettingsManager *m_psoundGSettingsManager = nullptr;

    QString KEY_SOUNDS_SCHEMA     =  "org.ukui.sound";
    QString UKUI_SOUND_KEY_THEME_NAME = "theme-name";

    QHash<QString, OnKeyChanged> backgroundChangedHandler();
    QHash<QString, OnKeyChanged> m_backgroundSettingsChangedHandlers;
    GsettingsManager *m_pBackgroundGsettingsManager = nullptr;
    const QString MATE_SETTINGS_SCHEMA_ID = "org.mate.background";
    const QString BACKGROUND_PICTRURE_FILENAME = "picture-filename";
};

#endif // THEME_H
