/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "clock.h"
#include "singleApplication.h"
#include <QApplication>
#include <QDesktopWidget>
#include "gsettingsubject.h"
#include "ClockInterface.h"
#include <QDBusMetaType>
#include "kabase/log.h"
#include "clockdbusadaptor.h"
#include "dbusupdateclockadaptor.h"
#include "dbusselectclockbyidadaptor.h"
#include "dbusdeleteclockbyidadaptor.h"
int main(int argc, char *argv[])
{
        //适配分数缩放
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    //构造单例 利用本地连接qlocalsocket作为判断的公共访问资源
    SingleApplication a(argc, argv);
    //支持主题框架图标
     a.setWindowIcon(QIcon::fromTheme("kylin-alarm-clock"));
    //此属性保存关闭最后一个窗口时应用程序是否隐式退出。默认值为true。
    a.setQuitOnLastWindowClosed(false);

    //如果没在运行
    if (!a.isRunning()) {
        Clock w;
        a.w = &w;
        w.setAttribute(Qt::WA_AlwaysShowToolTips);
        QObject::connect(&a, SIGNAL(messageReceived(/*const QString&*/)), &w, SLOT(sltMessageReceived(/*const QString&*/)));
        w.showThisWindow();
        QTimer::singleShot(50,[&w](){
            qInstallMessageHandler(kdk::kabase::Log::logOutput);
            w.showList();
        });
        return a.exec();
    }
    return 0;
}
