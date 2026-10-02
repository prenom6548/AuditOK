/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef UKMEDIAOSDDISPLAYWIDGET_H
#define UKMEDIAOSDDISPLAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDBusReply>
#include <QGSettings>
#include <QApplication>
#include <QBoxLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QX11Info>
#include <QPainter>
#include <QTimer>
#include <QIcon>
#include <QScreen>
#include <ukcc/interface/ukcccommon.h>
#include "windowmanager/windowmanager.h"
#include "ukuistylehelper/ukuistylehelper.h"

#define OSDWIDGET_SIZE 92,92
#define ICON_SIZE 48,48
#define TRANSPARENCY "org.ukui.control-center.personalise"
#define THEME "org.ukui.style"

//任务栏多屏显示声音应用获取屏幕可用区域接口
#define PANEL_SETTINGS      "org.ukui.panel.settings"
#define PANEL_SIZE_KEY      "panelsize"
#define PANEL_POSITION_KEY  "panelposition"

class UkmediaOsdDisplayWidget : public QWidget
{
    Q_OBJECT

    enum PanelPosition{
        Bottom = 0, //!< The bottom side of the screen.
        Top,    //!< The top side of the screen.
        Left,   //!< The left side of the screen.
        Right   //!< The right side of the screen.
    };

public:
    UkmediaOsdDisplayWidget(QWidget *parent = nullptr);
    ~UkmediaOsdDisplayWidget();
    friend class DeviceSwitchWidget;

    void initAttribute();
    void initGsettings();
    void initPanelGSettings();
    QRect caculatePosition(); //任务栏多屏显示声音应用获取屏幕可用区域接口
    void geometryChangedHandle();
    void setIcon(QString iconStr);
    void dialogShow();

    QTimer  *mTimer;
    QLabel  *iconLabel;
    QString mIconName;

    QGSettings *m_pThemeSetting;
    QPixmap drawLightColoredPixmap(const QPixmap &source, const QString &style);

public Q_SLOTS:
    void    repaintWidget();
    void    timeoutHandle();
    void    ukuiThemeChangedSlot(const QString &themeStr);
    double  getGlobalOpacity();

private:
    QFrame *m_frame;
    QGSettings *m_panelGSettings = nullptr;
    int m_panelPosition;
    int m_panelSize;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
};

#endif // UKMEDIAOSDDISPLAYWIDGET_H
