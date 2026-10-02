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
#ifndef MENUMODULE_H
#define MENUMODULE_H

#include <QObject>
#include <QtWidgets>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QUrl>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QApplication>
#include <QFont>
#include "daemondbus.h"
#include "controller/Interaction.h"
#include "global/viewvar.h"
#include <user_manual.hpp>
// 主题sdk关于
#include "kaboutdialog.h"

class menuModule : public QWidget
{
    Q_OBJECT
    friend class TestMenuModule;
public:
    explicit menuModule(QWidget *parent = nullptr);

private:
    QAction *m_locationYes = nullptr;
    QAction *m_locationNo = nullptr;
    int executeCMD(QString, QString&);
    QString getVersion();

public:
    QToolButton *menuButton = nullptr;
    //    QPushButton *menuButton = nullptr;
    QMenu *m_menu = nullptr;
    QDialog *m_aboutWindow = nullptr;

private:
    void init();
                                 ////    void initGsetting();
    void initAction();
    void triggerMenu(QAction *act); //主菜单动作4
                                    ////    void triggerThemeMenu(QAction* act); //主题动作
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    //    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    //    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void initConnect();

public slots:
    void monitorThemeChanged(QString themeChange);
    void setMenuModuleAllowLocationYes();

signals:
    void menuModuleClose(); //从标题栏点击退出的信号
    void allowLocationYes();
    void allowLocationNo();
};

#endif // MENUMODULE_H
