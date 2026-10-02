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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "frmlunarcalendarwidget.h"
#include "lunarcalendarwidget.h"
#include <QApplication>
#include <QTextCodec>
#include <QCommandLineParser>
#include <single_application/single_application.hpp>
#include <QFileInfo>
#include <QDBusInterface>
#include <ukui-log4qt.h>
#include "unistd.h"
#include "fcntl.h"
#include <libkydatacollect.h>
static const QString NotificationsDBusService = "org.ukui.calendar.Notifications";
static const QString NotificationsDBusPath = "/org/ukui/calendar/Notifications";
#include "notificationsadaptor.h"
int main(int argc, char *argv[])
{
#ifndef QT_DEBUG
    initUkuiLog4qt("kylin-calendar");
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    Q_CLASSINFO("D-Bus Interface", "org.ukui.kylin_calendar") //调用DBus一定要加这一行
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    //    a.setFont(QFont("Microsoft Yahei", 9));
    QTranslator *translator = new QTranslator();
    if(translator->load("/usr/share/qt5/translations/qt_" + QLocale::system().name()))
    {
        qApp->installTranslator(translator);
    }
    frmLunarCalendarWidget w;
    w.setWindowFlags(Qt::FramelessWindowHint);
    w.setWindowTitle("日历");

    // w.onActivateRequest();
    // return a.exec();

    KWindowSystem::setState(w.winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    //命令参数设置
    QStringList strList;
    QString str1;
    for (int i = 0; i < argc; i++)
    {
        str1 = argv[i];
        strList.push_back(str1);
    }
    QString str = "";
    QString str2 = "";
    QString str3 = "";
    if (strList.size() > 1)
    {
        str = argv[1];
        if (strList.size() == 4)
        {
            str2 = argv[2];
            str3 = argv[3];
        }
    }
    if (str != "")
    {
        if (str == "--help" || str == "-h") //帮助
        {
            qDebug() << "\nkylin-calendar [cmd]\n"
                     "-s -show  打开日历\n"
                     "-h -hide  隐藏日历\n";
            w.m_widget->hide();
            //            return 0;
        }
        //如果参数不是命令也不是文件路径，则退出
        if (!QFileInfo::exists(str) && str != "-s"
            && str != "-show" && str != "-h" && str != "-hide")
        {
            qDebug() << "参数不合规，请使用--h或者--help参数获取帮助";
            return 0;
        }
    }
    //单例检测
    w.checkSingle(strList);
    w.changeWidowposFromClickButton(true);
    if (str != "")
    {
        if (str == "--help" || str == "-h") //帮助
        {
            qDebug() << "\nkylin-calendar [cmd]\n"
                     "-s -show  打开日历\n"
                     "-h -hide  隐藏日历\n";
            w.m_widget->hide();
            w.hide();
            //            return 0;
        }
        //如果参数不是命令也不是文件路径，则退出
        if (!QFileInfo::exists(str) && str != "-s"
            && str != "-show" && str != "-h" && str != "-hide")
        {
            qDebug() << "参数不合规，请使用--h或者--help参数获取帮助";
            return 0;
        }
    }
    //    w.hide();
    //    w.show();
    return a.exec();
}
