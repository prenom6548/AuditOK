/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#define DEBUG_MENUMODULE 0 // DEBUG模式开关，代码上线之前务必关掉
#include <QObject>
#include <QWidget>
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
#include <QMetaEnum>
#include <QApplication>
#include <QFont>
#include "controller/interaction.h"
#include <kmenubutton.h>
class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *parent = nullptr);
    ~menuModule();

Q_SIGNALS:
    void menuModuleClose();      //从标题栏点击退出的信号
    void openSignal();           //从标题栏进行打开图片的信号
    void aboutShow();            //打开关于，主界面两栏的显示情况
    void helpShow(QString appName);//打开帮助
    void setFontSizeSignal(int); //字体变化
    void exitAppFromMenu();

public:
    QToolButton *menuButton = nullptr;
    QMenu *m_menu = nullptr;

private:
    QMenu *m_themeMenu = nullptr;
    enum typeThemeStatus { themeAuto = 0, themeBlackOnly = 1, themeLightOnly = 2 } themeStatus;
    QFont m_ft; //固定字号
    QGSettings *m_fontSize = nullptr;

    QAction *actionOpen = nullptr;
    QAction *actionHelp = nullptr;
    QAction *actionAbout = nullptr;
    QAction *actionQuit = nullptr;
    QAction *actionFeedback = nullptr;

private:
    void init();
    void initAction();
    void triggerMenu(QAction *act);      //主菜单动作4
    void triggerThemeMenu(QAction *act); //主题动作
    void updateMenuActionState();
};

#endif // MENUMODULE_H
