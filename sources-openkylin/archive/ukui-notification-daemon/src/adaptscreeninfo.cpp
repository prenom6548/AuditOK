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

#include "adaptscreeninfo.h"

adaptScreenInfo::adaptScreenInfo(QObject *parent) : QObject(parent)
{
    m_pDeskWgt = QApplication::desktop();
    InitializeHomeScreenGeometry();
    connect(QApplication::primaryScreen(),&QScreen::geometryChanged,this,&adaptScreenInfo::resolutionOrientationChangedSlot);
    connect(QApplication::primaryScreen(),&QScreen::virtualGeometryChanged,this,&adaptScreenInfo::screenPositionChangedSlot);
    connect(m_pDeskWgt, &QDesktopWidget::primaryScreenChanged, this, &adaptScreenInfo::primaryScreenChangedSlot);
    connect(m_pDeskWgt, &QDesktopWidget::screenCountChanged, this, &adaptScreenInfo::screenCountChangedSlots);
    m_pListScreen = QGuiApplication::screens();
}

/* 初始化屏幕高度， 宽度 */
void adaptScreenInfo::initScreenSize()
{
    m_screenWidth = QGuiApplication::primaryScreen()->geometry().width();   //桌面分辨率的宽
    m_screenHeight = QGuiApplication::primaryScreen()->geometry().height(); //桌面分辨率的高
    qInfo() << "Main screen Width  : " << m_screenWidth;
    qInfo() << "Main screen Height : " << m_screenHeight;
    return;
}

/* 初始化主屏坐标和宽度高度 */
void adaptScreenInfo::InitializeHomeScreenGeometry()
{
    initOsDbusScreen(); // 初始化坐标
    initScreenSize();   // 初始化屏幕宽度
    emit updataTopWidgetSite();
    emit updataCenterWidgetSite();
    return;
}

void adaptScreenInfo::initOsDbusScreen()
{
    if (QGuiApplication::primaryScreen()) {
        m_nScreen_x = QGuiApplication::primaryScreen()->geometry().x();
        m_nScreen_y = QGuiApplication::primaryScreen()->geometry().y();
    } else {
        //如果获取不到主屏，使用0号屏幕作为主屏坐标
        QList<QScreen *> screen = QGuiApplication::screens();
        int count = m_pDeskWgt->screenCount();
        if (count > 1) {
            m_nScreen_x = screen[0]->geometry().x();
            m_nScreen_y = screen[0]->geometry().y();
        } else {
            m_nScreen_x = 0;
            m_nScreen_y = 0;
        }
    }
    qInfo() << "Screen X coordinate:" << m_nScreen_x;
    qInfo() << "Screen Y coordinate:" << m_nScreen_y;
}

/* 分辨率改变、屏幕方向改变 槽函数*/
void adaptScreenInfo::resolutionOrientationChangedSlot(const QRect argc)
{
    Q_UNUSED(argc);
    InitializeHomeScreenGeometry();
    return;
}

/* 双屏相对位置变化 槽函数 */
void adaptScreenInfo::screenPositionChangedSlot(const QRect argc)
{
    Q_UNUSED(argc);
    InitializeHomeScreenGeometry();
}

/* 主屏切换槽函数 */
void adaptScreenInfo::primaryScreenChangedSlot()
{
    InitializeHomeScreenGeometry();
    return;
}

/* 屏幕数量改变时对应槽函数 */
void adaptScreenInfo::screenCountChangedSlots(int count)
{
    m_screenNum = count;
    m_pListScreen = QGuiApplication::screens(); // 更新屏幕信息列表
    InitializeHomeScreenGeometry();
    return;
}
