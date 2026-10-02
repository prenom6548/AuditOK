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

#ifndef ADAPTSCREENINFO_H
#define ADAPTSCREENINFO_H

#include <QObject>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QDBusInterface>

class adaptScreenInfo : public QObject
{
    Q_OBJECT
public:
    explicit adaptScreenInfo(QObject *parent = nullptr);
    void InitializeHomeScreenGeometry();

    QDesktopWidget *m_pDeskWgt; // 桌面信息类
    int m_screenWidth;          // 桌面宽度
    int m_screenHeight;         // 桌面高度
    int m_screenNum;            // 屏幕数量
    int m_nScreen_x;            // 主屏起始坐标X
    int m_nScreen_y;            // 主屏起始坐标Y

Q_SIGNALS:
    void updataTopWidgetSite(); //更新通知窗口显示位置信号
    void updataCenterWidgetSite(); //更新霸屏窗口显示位置信号

private:
    void initScreenSize();   //初始化屏幕高度、宽度
    bool initHuaWeiDbus();   //初始化华为990dbus接口
    void initOsDbusScreen(); //初始化主屏起始坐标

private slots:
    void primaryScreenChangedSlot();                         //主屏切换槽函数
    void resolutionOrientationChangedSlot(const QRect argc); //分辨率改变、屏幕方向改变 槽函数
    void screenPositionChangedSlot(const QRect argc);        //双屏相对位置变化 槽函数
    void screenCountChangedSlots(int count);                 //屏幕数量改变 槽函数

private:
    QList<QScreen *> m_pListScreen; //所有屏幕信息列表
};

#endif // ADAPTSCREENINFO_H
