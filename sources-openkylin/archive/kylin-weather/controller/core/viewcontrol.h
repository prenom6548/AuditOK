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
#ifndef VIEWCONTROL_H
#define VIEWCONTROL_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QFile>
#include <QStandardPaths>
#include "controller/Interaction.h"
#include "view/horscreen/horscreen.h"
#include "view/verscreen/verscreen.h"
#include "global/viewvar.h"

class viewControl : public QObject
{
    Q_OBJECT
    friend class TestViewControl;
public:
    viewControl();
    void initCore(const QStringList &list, bool isStart); //初始化后端
    QWidget *activeWindow();
    static viewControl *getInstance();

private:
    horscreen *m_horscreen = nullptr;
    verscreen *m_verscreen = nullptr;
    bool m_screen = false;         // false-横屏，true-竖屏
    bool m_autoStart = false;      //默认为开机自启，不显示主界面
    bool m_isStart = false;         //默认为带noshowmainwindow启动
    void showView(bool isStart);   //实例化前端
    void showInCenter(QWidget *w); //显示在屏幕中央
    void initConnect();            //信号槽
    static viewControl *m_viewControl;
public slots:
    void autoStart(); //通知实例化界面
    void toShowView(); //在界面打开之后，将界面展示在屏幕中间---暂时没有用到，之后横竖屏会用到
    void toShowViewAndRefresh(); //通过desktop激活已有进程时显示并刷新
    void updateIsStart(bool isStart); // 默认不带showmainwindow启动
signals:
    void setActiveWidow(QWidget *w); //通知激活窗口
};

#endif // VIEWCONTROL_H
