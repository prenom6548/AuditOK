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
#ifndef CENTERPOPUPBACKGROUNDWIDGET_H
#define CENTERPOPUPBACKGROUNDWIDGET_H
#include <QWidget>
#include <QGSettings>
#include "adaptscreeninfo.h"
#define ORG_UKUI_STYLE  "org.ukui.style"
#define ICON_THEME_NAME "iconThemeName"
#define STYLE_NAME "styleName"
#define STYLE_NAME_KEY_DARK        "ukui-dark"
#define STYLE_NAME_KEY_BLACK       "ukui-black"
#define STYLE_NAME_KEY_LIGHT       "ukui-light"
#define STYLE_NAME_KEY_WHITE       "ukui-white"
#define STYLE_NAME_KEY_DEFAULT     "ukui-default"

class CenterPopupBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    CenterPopupBackgroundWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    void setWidgetAttribute();
    void updataCenterWidgetBackgroundSite();
    void systemThemeChanges();
private:
    adaptScreenInfo *m_sreenInfo;
    int m_screenXGeometry = 0;
    int m_screenYGeometry = 0;
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    QGSettings *m_themeSettings;

};

#endif // CENTERPOPUPBACKGROUNDWIDGET_H
