/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include <ukui-log4qt.h>
#include <kysdk/diagnosetest/libkydatacollect.h>

#include <QTranslator>
#include <QDir>
#include <QDebug>
using namespace kdk;

int main(int argc, char *argv[])
{
    kdk_dia_upload_program_startup();

    QApplication a(argc, argv);
    initUkuiLog4qt("kylin-theme-builder-app");
    //防止多开
    QSharedMemory fixsharedmemory("kylin-theme-builder");
    if (fixsharedmemory.attach())
    {
        fixsharedmemory.detach();
    }
    static QSharedMemory *singleApp = new QSharedMemory("kylin-theme-builder");
    if(!singleApp->create(1))//创建失败
    {
        qApp->quit();
        return -1;
    }
    //翻译
    QTranslator app_trans;
    QString locale = QLocale::system().name();
    QString trans_path;
    if (QDir("/usr/share/kylin-theme-builder/translations").exists()) {
        trans_path = "/usr/share/kylin-theme-builder/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
    if (!app_trans.load("themebuilder_" + locale + ".qm", trans_path)) {
        qDebug() << "Load translation file："
                 << "themebuilder_" + locale + ".qm from" << trans_path << "failed!";
    } else {
        a.installTranslator(&app_trans);
    }

    QTranslator sdkapp_trans;
    if(locale == "zh_CN")
    {
        if(sdkapp_trans.load(":/translations/gui_zh_CN.qm"))
        {
            a.installTranslator(&sdkapp_trans);
        }
    }
    if(locale == "bo_CN")
    {
        if(sdkapp_trans.load(":/translations/gui_bo_CN.qm"))
        {
            a.installTranslator(&sdkapp_trans);
        }
    }

    QTranslator *qt_trans = new QTranslator();
    if(qt_trans->load("/usr/share/qt5/translations/qt_"+QLocale::system().name())) {
    a.installTranslator(qt_trans);
    }

//    QTranslator translator;
//    translator.load("translation/themebuilder_zh_CN.qm");
//    qApp->installTranslator(&translator);
    MainWindow w;
    w.show();
    return a.exec();
}
