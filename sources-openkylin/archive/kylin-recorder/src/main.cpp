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
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "mainwindow.h"
#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <fcntl.h>
#include <libkydatacollect.h>
// #include "xatom-helper.h"
#include <ukuistylehelper/ukuistylehelper.h>
#include "log.h"

#define DOUBLE 2
#define MAX_FILE_SIZE 1024
#define LOG_FILE0 "demo_0.log"
#define LOG_FILE1 "demo_1.log"
#define LOG_FILE_PATH "/.cache/kylin-recorder/log"
/************************************************
 * 函数名称：qtMessagePutIntoLog
 * 功能描述：日志记录函数
 * 输入参数：qDebug标准输出
 * 输出参数：kylinRecorderLog.txt
 * 修改日期：2021.03.22
 * 修改内容：
 *   创建  bjc
 *
 *************************************************/

static QString execCmd(QString cmd)
{
    qInfo() << "cmd = " << cmd;
    QProcess process;
    process.start(QString(cmd));
    process.waitForFinished();
    QByteArray result = process.readAllStandardOutput();
    result = result.left(result.length()-1);
    qInfo() << "arch = " << result;
    return result;
}

static QString getAppVersion()
{
    QString versionResult = execCmd(QString("dpkg-parsechangelog -l %1 --show-field Version").arg("/usr/share/doc/kylin-recorder/changelog.Debian.gz"));
    return versionResult;
}

int main(int argc, char *argv[])
{
/*
 * 添加4K 屏幕支持。
 */
// #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
//     QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//     QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
// #endif

// #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
//     QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
// #endif
    QApplication a(argc, argv);

    QTranslator app_trans;
    QTranslator qt_trans;
    QTranslator sdk_trans;
    QString locale = QLocale::system().name();

    QString trans_path;
#ifdef NOTKAIMING
    if (QDir("/usr/share/kylin-recorder/translations").exists()) {
        trans_path = "/usr/share/kylin-recorder/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
#else
    if (QDir("/opt/apps/top.openkylin.kylin-recorder/files").exists()) {
        trans_path = "/opt/apps/top.openkylin.kylin-recorder/files";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
#endif
    QString qt_trans_path;
    qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations

    if (locale == "en_US") {
            KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
            kdk_dia_upload_default(node, (char *)"English Language", (char *)"main");
            kdk_dia_data_free(node);
        }
    if (!app_trans.load("kylin-recorder_" + locale + ".qm", trans_path))
        qDebug() << "Load translation file："
                 << "kylin-recorder_" + locale + ".qm from" << trans_path << "failed!";
    else
        a.installTranslator(&app_trans);

    if (!qt_trans.load("qt_" + locale + ".qm", qt_trans_path))
        qDebug() << "Load translation file："
                 << "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
    else
        a.installTranslator(&qt_trans);
    //    }

    QString path = QString(":/translations/gui_%1.qm").arg(locale);
    if(sdk_trans.load(path)){
        a.installTranslator(&sdk_trans);
    }

    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator();
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(translator);
    } else {
        qDebug() << "加载中文失败";
    }
    QStringList strList;
    QString str1;
    for (int i = 0; i < argc; i++) {
        str1 = argv[i];
        strList.push_back(str1);
    }
    QString str = "";
    QString str2 = "";
    QString str3 = "";
    if (strList.size() > 1) {
        str = argv[1];
        if (strList.size() == 4) {
            str2 = argv[2];
            str3 = argv[3];
        }
    }
    qDebug() << "strList = " << strList;
    qDebug() << "str = " << str;
    if (str != "") {
        if (str == "--help" || str == "--h") //帮助
        {
            qDebug() << "\nkylin-recorder [cmd]\n"
                        "-s -start  开始录音\n"
                        "-p -pause   暂停/继续\n"
                        "-f -finish 停止录音\n"
                        "-c -close   关闭窗口";
            return 0;
        }
        //如果参数不是命令也不是文件路径，则退出
        if (!QFileInfo::exists(str) && str != "-b" && str != "-back" && str != "-n" && str != "-next" && str != "-p"
            && str != "-pause" && str != "-s" && str != "-start" && str != "-f" && str != "-finish" && str != "-c"
            && str != "-close") {
            qDebug() << "1参数不合规，请使用--h或者--help参数获取帮助";
            return 0;
        }
        if (str == "-m" || str == "-move") {
            if (str2 != "" && str3 != "") {
                bool ok1;
                bool ok2;
                str2.toInt(&ok1);
                str3.toInt(&ok2);
                if (!ok1 || !ok2) {
                    qDebug() << "2参数不合规，请使用--h或者--help参数获取帮助";
                    return -1;
                }
            }
        }
    }
    MainWindow w(strList);
    w.currentGSettingChangeUI();
    kdk::UkuiStyleHelper::self()->removeHeader(&w);

    // 添加窗管协议
    a.setApplicationName(QApplication::tr("Recorder"));
    w.show();
    QTimer::singleShot(50, [&a, &w, locale]() {
      qInstallMessageHandler(kdk::kabase::Log::logOutput);
      printf("Program start ...\n");
      qApp->setProperty("notChangeSystemFontSize", true); //设置字体不跟随主题变化
      a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
      a.setOrganizationName("kylin");
      QLocale loc = QLocale::system().name();

      a.setApplicationVersion(getAppVersion());
      //设置主题图标跟随变换
      a.setWindowIcon(QIcon::fromTheme("kylin-recorder", QIcon(":/svg/svg/recording_128.svg")));
    });
    return a.exec();
}
