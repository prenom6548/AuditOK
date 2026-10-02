/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QWidget>
#include <QApplication>

#include "global/global.h"

using namespace Global;

class ThemeWidget : public QWidget
{
    friend class ContralBar;
    friend class SetupShortcut;
    friend class PlayListItem;
    friend class MarkListItem;
    friend class PlayListWidget;

    Q_OBJECT
public:
    explicit ThemeWidget(QWidget *parent = nullptr):
        QWidget(parent)
    {
        m_highlightColor = QApplication::palette(this).highlight().color();
    }
    ~ThemeWidget(){}

    void connectThemeSetting(){
        if (FOLLOW_SYS_THEME) {
            if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                setDefaultTheme();
            else
                setBlackTheme();

            connect(g_gsettings, &QGSettings::changed, this, [&](QString key){
                // 如果不是跟随主题的话直接返回
                if (key == "styleName") {
                    if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                        setDefaultTheme();
                    else
                        setBlackTheme();
                }
            });
        }
        else {
            setBlackTheme();
        }
    }

    virtual void setBlackTheme(){}
    virtual void setDefaultTheme(){}
    virtual void updateHightLightColor(){}

private:
    bool m_isBlackTheme;
    QColor m_highlightColor;
    bool event(QEvent *event) override{
        if (event->type() == QEvent::PaletteChange) {
            m_highlightColor = QApplication::palette(this).highlight().color();
            updateHightLightColor();
        }
        return QWidget::event(event);
    }
};

#endif // THEMEWIDGET_H
