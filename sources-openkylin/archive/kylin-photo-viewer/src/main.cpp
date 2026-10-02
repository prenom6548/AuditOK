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
//*************************************
//代优化列表:
//动态加载；------已完成
//裁剪绘制部分：1）代码拆分，细化功能，目前该函数下代码行数过多；2）有些情况可以合并，需要时间来优化这里；
//优先mimetype读取而非后缀；
//界面qss整理：工具栏按钮的qss设置，主界面部分；
//缩略图读取；
//一些支持查看但不支持操作并保存的图片格式会陆续加进来
//代码文件开头补充开源协议
//看图-ocr，看图识字
// svg-放大缩小单独svg处理
//*************************************

#include "view/kyview.h"
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include "controller/interaction.h"
#include <QLoggingCategory>
#include <ukui-log4qt.h>
#include <ukuiwindowhelper.h>
#include <log.hpp>
#include <QElapsedTimer>
QElapsedTimer startTime;
int main(int argc, char *argv[])
{
    qint64 ts = QDateTime::currentMSecsSinceEpoch();
    qDebug()<<"main - 当前毫秒级时间戳:"<<ts;
    startTime.restart();

    //适配4K屏以及分数缩放
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    //日志输出
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);
    qApp->setWindowIcon(QIcon::fromTheme("kylin-photo-viewer"));

    //翻译
    QTranslator app_trans;
    QTranslator qt_trans;
    QString locale = QLocale::system().name();
    QString trans_path;
    if (QDir("/usr/share/kylin-photo-viewer/translations").exists()) {
        trans_path = "/usr/share/kylin-photo-viewer/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
    QString qt_trans_path;
    qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations
    if (!qt_trans.load("qt_" + locale + ".qm", qt_trans_path)) {
        qDebug() << "Load translation file："
                 << "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
    } else {
        a.installTranslator(&qt_trans);
    }
    QTranslator app_trans_penoy;
    app_trans_penoy.load("/usr/share/libpeony-qt/libpeony-qt_" + QLocale::system().name());
    a.installTranslator(&app_trans_penoy);
    QTranslator sdk_trans;
    if (sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        a.installTranslator(&sdk_trans);
    }
    if (!app_trans.load("kylin-photo-viewer_" + locale + ".qm", trans_path)) {
        qDebug() << "Load translation file："
                 << "kylin-photo-viewer_" + locale + ".qm from" << trans_path << "failed!";
    } else {
        a.installTranslator(&app_trans);
    }

    // 设置线程池最大线程数
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    a.setApplicationName(QApplication::tr("Pictures"));
    //主题框架
    KyView w(a.arguments());
    int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
    int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
    QPoint leftPoint = QPoint((sw - w.width()) / 2, (sh - w.height()) / 2);
    w.setGeometry(QRect(leftPoint, QSize(w.width(), w.height())));

    w.show();
    qWarning()<<"2 main()函数 KyView显示 time = "<<startTime.elapsed()<<" ms";
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(&w);
    windowHelper->removeTitleBar();

    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    Interaction::getInstance()->initUiFinish();

//    QTimer::singleShot(3, [&a, &w] () {
//        Interaction::getInstance()->initUiFinish();
//    });
    return a.exec();
}
