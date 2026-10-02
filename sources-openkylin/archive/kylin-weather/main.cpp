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
#include "controller/core/viewcontrol.h"
#include <QApplication>
#include <QObject>
#include <signal.h>
#include <QDBusInterface>
#include <single_application/single_application.hpp>
#include <window_management.hpp>
#include "kabaselog.hpp"
#include "windowmanage.hpp"
#include <kysdk/diagnosetest/libkydatacollect.h>
#include "ukuiwindowhelper.h"

//初始化核心，初始化前端，启动时是否展示界面
void controlView(kdk::kabase::QtSingleApplication &a, const QStringList &args, bool isStart)
{
    //实例化viewcontrol

    viewControl::getInstance()->initCore(args, isStart);
    QObject::connect(&a, &kdk::kabase::QtSingleApplication::messageReceived, viewControl::getInstance(),
                     &viewControl::toShowViewAndRefresh);
    //激活窗口

    // ::kabase::WindowManage::removeHeader(viewControl::getInstance()->activeWindow());
    UkuiWindowHelper *helper = new UkuiWindowHelper(viewControl::getInstance()->activeWindow());
    helper->removeTitleBar();
    a.setActivationWindow(viewControl::getInstance()->activeWindow());
}
int main(int argc, char *argv[])
{
    //在程序启动时调用，不需要传入参数
    kdk_dia_upload_program_startup();
    //适配4K屏以及分数缩放
    kdk::kabase::WindowManagement::setScalingProperties();
    //日志输出
    qInstallMessageHandler(::kabase::KabaseLog::logOutput);
    kdk::kabase::QtSingleApplication a(argc, argv);

    a.setWindowIcon(QIcon::fromTheme("kylin-weather"));
    //单例
    if (a.isRunning()) {
        qDebug() << "is running";
        QStringList args = a.arguments();
        bool isShowMainwindow = !(args.contains("noshowmainwindow") || args.contains("--dbus-service"));
        qDebug() << "isShowMainwindow = " << isShowMainwindow;
//        viewControl::getInstance()->initCore(a.arguments(), isShowMainwindow);
        viewControl::getInstance()->updateIsStart(isShowMainwindow);

        if (!isShowMainwindow) {
            // 此时可以是通过 `xrdp` 启动的天气影音，带有 noshowmainwindow 参数。因此，不需要再次显示应用界面。
            return 0;
        }
        a.sendMessage("running , 4000");
        return 0;
    }
    //翻译
    QTranslator qt_trans;
    QString locale = QLocale::system().name();
    QString trans_path;
#ifdef NOTKAIMING
    if (QDir("/usr/share/kylin-weather/translations").exists()) {
        trans_path = "/usr/share/kylin-weather/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
#else
    if (QDir("/opt/apps/top.openkylin.kylin-weather/files/share/kylin-weather/translations").exists()) {
        trans_path = "/opt/apps/top.openkylin.kylin-weather/files/share/kylin-weather/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
#endif
    QString qt_trans_path;
    qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations
    if (!qt_trans.load("qt_" + locale + ".qm", qt_trans_path)) {
        qDebug() << "Load translation file："
                 << "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
    } else {
        a.installTranslator(&qt_trans);
    }
    QTranslator sdk_trans;
    if (sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        a.installTranslator(&sdk_trans);
    }
    QTranslator app_trans;
    if (!app_trans.load("kylin-weather_" + locale + ".qm", trans_path)) {
        qDebug() << "Load translation file："
                 << "kylin-weather_" + locale + ".qm from" << trans_path << "failed!";
    } else {
        a.installTranslator(&app_trans);
    }
    a.setApplicationName(QApplication::tr("Weather"));
    //初始化后端前端，判断启动方式，是否带参
    QStringList args = a.arguments();
    bool isShowMainwindow = !(args.contains("noshowmainwindow") || args.contains("--dbus-service"));
    controlView(a, args, isShowMainwindow);

    return a.exec();
}
