/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GSETTINGS_MONITOR_H
#define GSETTINGS_MONITOR_H

#include <QObject>
#include <QGSettings>


class GsettingsMonitor : public QObject
{
    Q_OBJECT

public:
    enum StyleStatus{
        NONE = 0,
        LIGHT,
        DARK
    };

    enum WidgetThemeStatus{
        UNCLASSICAL = 0,
        CLASSICAL
    };

    GsettingsMonitor(/* args */);
    ~GsettingsMonitor() override;
    int transparency() const;
    StyleStatus styleStatus() const;
    WidgetThemeStatus widgetThemeName() const;

signals:
    void transparencyUpdate(int transparency);
    void styleStatusUpdate(StyleStatus styleStatus);
    void widgetThemeUpdate(WidgetThemeStatus widgetThemeStatus);

private:
    bool gsettingsCheck();
    void initGsettings();
    void getTransparency();
    void getStyleStatus();
    void getWidgetThemeStatus();

    StyleStatus styleNameConvert(const QString &styleName);
private:
    const char *UKUI_STYLE = "org.ukui.style";
    const char *UKUI_FONT_SIZE = "systemFontSize";
    const char *UKUI_FONT = "systemFont";
    const char *UKUI_MENU_TRANSPARENCY = "menuTransparency";
    const char *UKUI_STYLE_NAME = "styleName";
    const char *UKUI_WIDGET_THEME_NAME = "widgetThemeName";
private:
    QGSettings *m_ukuiStyle;

    int m_menuTrans = -1;
    StyleStatus m_styleStatus = NONE;
    WidgetThemeStatus m_isClassical = UNCLASSICAL;
};

#endif // GSETTINGS_MONITOR_H
