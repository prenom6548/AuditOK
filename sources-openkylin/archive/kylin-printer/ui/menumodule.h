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

#ifndef MENUMODULE_H
#define MENUMODULE_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>

#include "daemonipcdbus.h"
#include "global_variable.h"

class MenuModule : public QMenu
{
    Q_OBJECT
public:
    explicit MenuModule(QWidget *parent= nullptr);
    ~MenuModule();
signals:
    void menuModuleClose();

private:
//    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString m_appName        = "kylin-printer"; //格式kylin-usb-creator
    QString m_appShowingName = tr("Printer"); //格式kylin usb creator ,用来在前端展示
    QString m_appVersion     = qApp->applicationVersion();
    QString m_appDesc        = "Printer";
private:
    QWidget *m_mainWin;

private:
    void init();
    void triggerMenu(QAction* act); //主菜单动作4
    void aboutAction();
    void helpAction();
};

#endif // MENUMODULE_H
