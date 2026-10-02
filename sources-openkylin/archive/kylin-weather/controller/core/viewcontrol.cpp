/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "viewcontrol.h"
#include "windowmanage.hpp"
#include "ukuiwindowhelper.h"
viewControl *viewControl::m_viewControl = nullptr;
viewControl *viewControl::getInstance()
{
    if (m_viewControl == nullptr) {
        m_viewControl = new viewControl;
    }
    return m_viewControl;
}
viewControl::viewControl()
{
    initConnect();
}
//信号槽
void viewControl::initConnect()
{
    connect(Interaction::getInstance(), &Interaction::toStartView, this, &viewControl::autoStart); //启动前端
    connect(Interaction::getInstance(), &Interaction::showInCenter, this,
            &viewControl::toShowView); //目前没有用到，但之后横竖屏需要此信号
}

//初始化界面
void viewControl::autoStart()
{
    qDebug() << "是否显示前端界面： " << m_isStart;
    showView(m_isStart);
    ViewVar::initGsetting();
    ViewVar::initFontSize();
}
//界面已经初始化之后，显示在屏幕中央
void viewControl::toShowView()
{
    qDebug() << "m_isStart: " << m_isStart;
    if (m_screen == false) {
        if (m_horscreen != nullptr) {
            showInCenter(m_horscreen);
        }
    } else {
        if (m_verscreen != nullptr) {
            showInCenter(m_verscreen);
        }
    }
}

void viewControl::toShowViewAndRefresh()
{
    qDebug() << "desktop activation show and refresh";
    toShowView();
    Interaction::getInstance()->refresh();
}

/**
 * @brief viewControl::updateIsStart 更新启动参数
 * @param isStart
 *      true: 前端界面可以启动
 *      false: 启动的是服务端，此时不带界面，启动的是 kylin-weather noshowmainwindow
 */
void viewControl::updateIsStart(bool isStart)
{
    m_isStart = isStart;
}

/**
 * @brief viewControl::initCore 后端初始化
 * @param list
 * @param isStart
 *      true:  显示主界面，不带 noshowmainwindow 或 不带参
 *      false: 不显示主界面，  kylin-weather noshowmainwindow 启动
 */

//后端初始化
void viewControl::initCore(const QStringList &list, bool isStart)
{
    qDebug() << " ##### Function Positioning ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //带showmainwindow 或 不带参
    m_isStart = isStart;
    //初始化后端
    Interaction::getInstance()->creatCore(list);
}
//返回已经存在的widget，给qsingleapplication激活
QWidget *viewControl::activeWindow()
{
    if (m_screen == false && m_horscreen != nullptr) {
        return m_horscreen;
    } else if (m_screen == true && m_verscreen != nullptr) {
        return m_verscreen;
    }
}
//确定横竖屏
void viewControl::showView(bool isStart)
{
    //获得横竖屏信号  m_screen暂时用来判断
    //确定是什么屏幕
    if (m_screen == false) {
        if (m_horscreen == nullptr) {
            m_horscreen = new horscreen();
            m_horscreen->setAccessibleName("kylin-weather_viewControl_horscreen");
        }
        //如果不是开机自启动，而是双击desktop启动，show主界面
        if (isStart) {
            qDebug() << "横屏显示主界面： " << isStart;
            showInCenter(m_horscreen); //展示在屏幕中央
            emit setActiveWidow(m_horscreen);
        }
        //竖屏存在时，不起竖屏,且deleter
        if (m_verscreen == nullptr) {
            return;
        }
        if (m_verscreen->isHidden()) {
            return;
        }
        m_verscreen->hide();
        return;
    }
    //竖屏模式
    if (m_verscreen == nullptr) {
        m_verscreen = new verscreen();
    }
    if (isStart) {
        qDebug() << "竖屏显示主界面： " << m_isStart;
        showInCenter(m_verscreen);
        emit setActiveWidow(m_verscreen);
    }

    if (m_horscreen == nullptr) {
        return;
    }
    if (m_horscreen->isHidden()) {
        return;
    }
    m_horscreen->hide();
}
//显示在屏幕中央
void viewControl::showInCenter(QWidget *w)
{
    //计算将窗口移动到屏幕中间，并show出来
    if (!w->isVisible()) {
        w->setVisible(true);
        // wayland下居中显示
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            if (ViewVar::getInstance()->m_currentMode == CurrentMode::PadHMode){
                w->showFullScreen();
            }
            else{
                ::kabase::WindowManage::setMiddleOfScreen(w);
            }
        } else {
            QDesktopWidget *m = QApplication::desktop();
            QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
            int desk_x = desk_rect.width();
            int desk_y = desk_rect.height();
            int x = w->width();
            int y = w->height();
            w->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
        }

    }
//    w->hide();
    UkuiWindowHelper *helper = new UkuiWindowHelper(w);
    helper->removeTitleBar();
    w->showNormal();
    w->raise();
    w->activateWindow();
    w->raise();
    ViewVar::widgetIsShow = true;
    //为解决点击托盘，界面show不出来问题，判断是底层将此信号给拦截了
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=] {
        helper->removeTitleBar();
        w->showNormal();
        w->raise();
        w->activateWindow();
    });
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    timer->start(500);
}
