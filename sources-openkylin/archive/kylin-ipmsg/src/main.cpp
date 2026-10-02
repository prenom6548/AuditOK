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

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QDir>
#include <fcntl.h>
#include <syslog.h>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <QScreen>
#include <sys/inotify.h>
#include <signal.h>
#include <QTimer>

#include <singleapplication.h>
#include "log.hpp"
#include "windowmanage.hpp"

#include "controller/control.h"
#include "network/tcp_client.h"
#include "network/udp_socket.h"
#include "global/utils/global_data.h"
#include "global/declare/declare.h"
#include "view/kyview.h"
#include <kysdk/diagnosetest/libkydatacollect.h>
#include "ukuiwindowhelper.h"

void pipeHandler(int)
{
    qDebug() << "Info : catch signal SIGPIPE !!!";
    return;
}

int catchSig()
{
    int ret = -1;

    struct sigaction action;
    memset(&action, 0x00, sizeof(struct sigaction));

    action.sa_handler = pipeHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    ret = sigaction(SIGPIPE, &action, NULL);
    if (ret) {
        qDebug() << "Info : sigaction signal SIGPIPE fail!";
    }

    return 0;
}

void primaryScreen(QApplication *app, KyView *w){
    //初次启动时，获取主屏幕并显示在主屏幕上
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        // 计算窗口居中位置
        int x = primaryScreen->geometry().x() +
                (primaryScreen->geometry().width() - w->width()) / 2;
        int y = primaryScreen->geometry().y() +
                (primaryScreen->geometry().height() - w->height()) / 2;

        w->move(x, y); // 移动到主屏幕中心
    }
    // 监听主屏切换事件,当主/副屏属性发生改变时， 将app显示到主屏幕
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, [w](QScreen *newPrimary) {
        if (newPrimary) {
            qDebug() << "主屏已切换至:" << newPrimary->name();
            w->move(newPrimary->geometry().center() - w->rect().center());
        }
    });
}

int main(int argc, char *argv[])
{
    auto starttimer = QDateTime::currentMSecsSinceEpoch();

    //在程序启动时调用，不需要传入参数
    // kdk_dia_upload_program_startup();
//     qDebug()<< "kdk_dia_upload_program_startup time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

//     qDebug()<< "qInstallMessageHandler time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    /* 适配4K屏以及分数缩放 */
    ::kabase::WindowManage::setScalingProperties();
//     qDebug()<< "setScalingProperties time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    /* 获取命令行参数 */
    if (argc > 1) {
        QString argValueStr = argv[1];
        if (argValueStr == "-d" || argValueStr == "--debug") {
            GlobalUtils::DEBUG_MODE = true;
        }
    }

//     qDebug()<< "argValueStr time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    qRegisterMetaType<ChatMsgInfo>("ChatMsgInfo");
    qRegisterMetaType<ChatMsgInfo>("ChatMsgInfo&");
    qRegisterMetaType<g_tcpItem>("g_tcpItem");

//     qDebug()<< "qRegisterMetaType time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    kdk::QtSingleApplication app(argc, argv); //50ms
//     qDebug()<< "QtSingleApplication time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    app.setApplicationVersion("1.3.1");
    app.setWindowIcon(QIcon::fromTheme("kylin-ipmsg"));
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    /* 加载翻译文件 */
#ifdef NOTKAIMING
    QString kylinIpmsgTransPath = "/usr/share/kylin-ipmsg/data/translations";
    QString peonyTransPath = "/usr/share/libpeony-qt";
#else
    QString kylinIpmsgTransPath = "/opt/apps/top.openkylin.kylin-ipmsg/files/share/kylin-ipmsg/data/translations";
    QString peonyTransPath = "/opt/apps/top.openkylin.kylin-ipmsg/files/share/libpeony-qt";
#endif
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QString locale = QLocale::system().name();

    QTranslator trans_global, trans_peony, trans_qt;
    QTranslator trans_SDK;

    if (!trans_global.load(QLocale(), "kylin-ipmsg", "_", kylinIpmsgTransPath)) {
        qDebug() << "Load global translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }

    if (!trans_peony.load(QLocale(), "libpeony-qt", "_", peonyTransPath)) {
        qDebug() << "Load libpeony translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_peony);
    }

    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qDebug() << "Load qt translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }
    
    if (locale == "zh_CN") {
        if (trans_SDK.load(":/translations/gui_zh_CN.qm")) {
            app.installTranslator(&trans_SDK);
        }
    }
    else if (locale == "bo_CN") {
        if (trans_SDK.load(":/translations/gui_bo_CN.qm")) {
            app.installTranslator(&trans_SDK);
        }
    }
    else {
        QString path = QString(":/translations/gui_%1.qm").arg(locale);
        if (trans_SDK.load(path)) {
            app.installTranslator(&trans_SDK);
        }
    }

//    qDebug()<< "translations time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;


    if (app.isRunning()) {
        qDebug() << "is running";
        app.sendMessage("running , 4000");
        return 0;
    }
//     qDebug()<< "setApp time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    DataBase::getInstance();
    // /* 最小化拉起 */
    app.setActivationWindow(KyView::getInstance());

    /* wayland 下最小化拉起 */
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [=]() {
//            if (!KyView::getInstance()->getWindowId().isEmpty()) {
               kdk::WindowManager::activateWindow(KyView::getInstance()->m_mainwindowId);
//            }
        });
    }
//    qDebug()<< "setActivationWindow time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    /* 居中显示 */
    KyView::getInstance()->show();
//     qDebug()<< "KyView::getInstance()->show() time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    ::kabase::WindowManage::setMiddleOfScreen(KyView::getInstance());
    /* 移除标题栏 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(KyView::getInstance());
    windowhelper->removeTitleBar();

    // 使用singleShot延迟执行翻译加载（延迟0毫秒表示尽快执行但不阻塞当前流程）
    QTimer::singleShot(0, [&app, &starttimer] {
//        qDebug()<< "singleShot start time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
        /* 设置不使用gtk3平台文件对话框 */
        app.setProperty("useFileDialog", false);

        /* 使用sdk管理日志 */
         qInstallMessageHandler(::kabase::Log::logOutput);

        /* 捕捉信号SIGPIPE */
        catchSig();

        /* 实例逻辑中心 */
        Control::getInstance();

        KyView::getInstance()->showList();

        KyView *w = KyView::getInstance();
        primaryScreen(&app, w);

        KyView::getInstance()->checkLocalName();

//        qDebug()<< "singleShot end  time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    });
    qDebug()<< "end  time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    return app.exec();
}
