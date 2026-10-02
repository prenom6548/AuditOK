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

#include "UI/mainwidget.h"
#include "UI/base/xatom-helper.h"
#include "UI/base/log.h"
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QByteArray>
//#include <ukui-log4qt.h>
//#include <kabase/log.hpp>

#include <QScreen>
#include "kwidget.h"
using namespace kdk;

/**
 * @brief matchHighDpiScreen 适配4K屏
 */
void matchHighDpiScreen()
{
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif
}


/**
 * @brief handleCmdList  处理命令行参数
 * @param cmdline 获取的命令行参数
 * @param argv 命令行参数
 */
int handleCmdList(QStringList cmdList, char **argv)
{
    QString str = "";
    QString str2 = "";
    QString str3 = "";

    if(cmdList.size() > 1) {
        str = argv[1];

        if(cmdList.size() == 4) {
            str2 = argv[2];
            str3 = argv[3];
        }
    }

    if (str != "") {
        if(str == "--help"||str == "--h") {
            printf("Usage: kylin-music [options...]\n"
                      " -b, -back  上一首\n"
                      " -n, -next  下一首\n"
                      " -p, -pause  暂停\n"
                      " -s, -start  播放\n"
                      " -i, -increase  调高音量\n"
                      " -r, -reduce    调低音量\n"
                      " -t, -title              当前播放的歌曲名\n"
                      " -S, -state            当前播放状态\n"
                      " -I, -initialization  初始化音乐\n"
                      " -C, -CurrentItemInLoop    单曲循环\n"
                      " -L, -Loop                 列表循环\n"
                      " -R, -Random               随机播放\n"
                      " -m, -move int int 窗口移动\n"
                      " -c, -close     关闭窗口\n"
                   );
            return 2;
        }

        //如果参数不是命令也不是文件路径，则退出
        QString args;
        if (str.startsWith("file://")){
            QUrl url(str);
            args = url.toLocalFile();
        }
        else {
            args = str;
        }

        if(!QFileInfo::exists(args)&&str!="-b"&&str!="-back"&&str!="-n"&&str!="-next"&&str!="-p"&&str!="-pause"&&
                str!="-s"&&str!="-start"&&str!="-i"&&str!="-increase"&&str!="-r"&&str!="-reduce"&&
                str!="-C"&&str!="-CurrentItemInLoop"&&str!="-L"&&str!="-Loop"&&str!="-R"&&str!="-Random"&&
                str!="-m"&&str!="-move"&&str!="-c"&&str!="-close"&&str!="-sta"&&str!="-state"&&
                str!="-t"&&str!="-title"&&str!="-init"&&str!="-initialization") {
            printf("参数不合规，请使用--h或者--help参数获取帮助");
            return -1;
        }

        if (str == "-m"||str == "-move") {
            if (str2!=""&&str3!="") {
                bool ok1;
                bool ok2;
                str2.toInt(&ok1);
                str3.toInt(&ok2);

                if (!ok1 || !ok2) {
                    printf("参数不合规，请使用--h或者--help参数获取帮助");
                    return -1;
                }
            }
        }
    }

    return 0;
}


static QString execCmd(QString cmd)
{
    qDebug() << "cmd = " << cmd;
    QProcess process;
    process.start(QString(cmd));
    process.waitForFinished();
    QByteArray result = process.readAllStandardOutput();
    result = result.left(result.length()-1);
    //KyInfo() << "result = " << result;

    return result;
}

/**
 * @brief getSystemArchitecture 获取当前系统架构
 *
 * @return
 */
static QString getSystemArchitecture()
{
    QString archResult = execCmd("arch");
    qDebug() << "arch: " << archResult;

    return archResult;
}


#define ChangelogFilePath                      "/usr/share/doc/kylin-music/changelog.Debian.gz"
/**
 * @brief getAppVersion 获取当前软件版本
 * @return
 */
static QString getAppVersion()
{
    QString versionResult = execCmd(QString("dpkg-parsechangelog -l %1 --show-field Version").arg(ChangelogFilePath));
    qInfo() << "version: " << versionResult;

    return versionResult;
}


static void primaryInit(QApplication *app, Widget *widget){
    // 使用singleShot延迟执行翻译加载（延迟0毫秒表示尽快执行但不阻塞当前流程）
    QTimer::singleShot(50, [app, widget]() {

//        initUkuiLog4qt("kylin-music");
        qInstallMessageHandler(kdk::kabase::Log::logOutput);
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
            qputenv("QT_QPA_PLATFORM", "wayland");
        }
        getSystemArchitecture();
        matchHighDpiScreen();

        widget->showList();
    });

}

static void primaryScreen(QApplication *app, Widget *widget){
    //初次启动时，获取主屏幕并显示在主屏幕上
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        // 计算窗口居中位置
        int x = primaryScreen->geometry().x() +
                (primaryScreen->geometry().width() - widget->width()) / 2;
        int y = primaryScreen->geometry().y() +
                (primaryScreen->geometry().height() - widget->height()) / 2;

        widget->move(x, y); // 移动到主屏幕中心
    }
    // 监听主屏切换事件,当主/副屏属性发生改变时， 将app显示到主屏幕
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, [widget](QScreen *newPrimary) {
        if (newPrimary) {
            qDebug() << "主屏已切换至:" << newPrimary->name();
            widget->move(newPrimary->geometry().center() - widget->rect().center());
        }
    });
}

int main(int argc, char *argv[])
{
    QStringList strList;
    QString str1;

    for(int i = 0; i < argc;i++) {
        str1 = argv[i];
        QString args;
        if (str1.startsWith("file://")){
            QUrl url(str1);
            args = url.toLocalFile();
        }
        else {
            args = str1;
        }
        strList << args;
    }
    QApplication a(argc, argv);
    a.setApplicationVersion(getAppVersion());
    a.setWindowIcon(QIcon::fromTheme("kylin-music"));
    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QString locale = QLocale::system().name();
    g_user_signal->isRTLLang =  (locale == "ug_CN" || locale == "ky_KG" || locale == "kk_KZ");
    QString qtTranslationDirectory;
    QString appTranslationDirectory;
    QTranslator apptranslator; // 应用内语言翻译
    QTranslator qtTranslator; // qt 控件语言翻译
    QTranslator peonyTranslator; // peony 控件语言翻译
    QTranslator kdkTranslator; // kdk 控件语言翻译
    if(kdkTranslator.load(":/translations/gui_" + locale  + ".qm")){
         a.installTranslator(&kdkTranslator);
    }

#ifdef NOTKAIMING
    if (QDir("/usr/share/kylin-music/translations").exists()) {
        appTranslationDirectory = "/usr/share/kylin-music/translations";
    } else {
        appTranslationDirectory = qApp->applicationDirPath() + "/translations";
    }
#else
    appTranslationDirectory = "/opt/apps/top.openkylin.kylin-music/files/share/translations";
#endif

    if(! apptranslator.load("kylin-music_" + locale + ".qm",  appTranslationDirectory)) {
        qDebug() << "Load translation file："<< "kylin-music_" + locale + ".qm from" <<  appTranslationDirectory << "failed!";
    } else {
        a.installTranslator(& apptranslator);
    }


    qtTranslationDirectory = QLibraryInfo::location(QLibraryInfo::TranslationsPath);  // /usr/share/qt5/translations
    if(!qtTranslator.load("qt_" + locale + ".qm", qtTranslationDirectory)) {
        qDebug() << "Load translation file："<< "qt_" + locale + ".qm from" << qtTranslationDirectory << "failed!";
    } else {
        a.installTranslator(&qtTranslator);
    }

    QString peonyTranslatorFilePath = "/usr/share/libpeony-qt/libpeony-qt_" + locale+".qm";
    QFile peonyTranslatorFile(peonyTranslatorFilePath);
    if (peonyTranslatorFile.exists()) {
        if (!peonyTranslator.load(peonyTranslatorFilePath)){
            qDebug() << "Load translation file："<<  peonyTranslatorFilePath;
        } else {
            a.installTranslator(&peonyTranslator);
        }
    }
    int retCmd = handleCmdList(strList, argv);
    if (retCmd == -1) {
        // 参数不符合，也应该可以打开应用
        Widget w(QStringList() << "kylin-music  ");
        w.show();
        primaryScreen(&a, &w);
        primaryInit(&a, &w);
        return a.exec();
    } else if (retCmd == 2) {
        return 0;
    }

    Widget w(strList);
    w.show();
    primaryScreen(&a, &w);
    primaryInit(&a, &w);
    return a.exec();
}
