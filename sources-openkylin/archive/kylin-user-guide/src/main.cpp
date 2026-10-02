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
#include <QTextCodec>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFont>
#include <QString>
#include <QTranslator>
#include <QLibraryInfo>
#include <QProcess>
#include <QFile>
#include <QIcon>
#include <QFileInfo>
#include <QByteArray>

#include <ukui-log4qt.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <execinfo.h>
#include "main_controller.h"
#include "common-tool/comm_func.h"


#define BUFF_SIZE 128
#define CHANGELOG_PATH "/usr/share/doc/kylin-user-guide/changelog.gz"

QString gLang = "zh_CN";
QString gStartShowApp = "";
QStringList pathList;

static bool verify_file(QString realPathStr){
    if(realPathStr.contains("/home")){
        return true;
    }
    return false;
}

static void crashHandler(int sig)
{
    signal(sig, SIG_IGN);
    int size;
    char **strings;
    int i = 0;

    QString path = QString("%1/.config/kylin-user-guide/config-crash.log").arg(getenv("HOME"));
    QString realPath;
    QFile file(path);
    if (file.exists()) {
        realPath = QFileInfo(file).absoluteFilePath();
        QString realPathStr = QString(QLatin1String(realPath.toUtf8()));
        if(verify_file(realPathStr)){
            if (file.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&file);

                void *array[20];
                size = backtrace (array, 20);
                strings = (char **)backtrace_symbols (array, size);

                out << QString("!!!--- received signal: %1=%2! Stack trace\n").arg(sig).arg(strsignal(sig));
                for (i = 0; i < size; i++)
                {
                    out << QString("%1 %2 \n").arg(i).arg(strings[i]);
                }

                file.flush();
                file.close();
                free (strings);

                exit(128 + sig);
            }
        }
    }
}

QString getAppVersion(){
    qDebug()<< Q_FUNC_INFO;
    QProcess process;
    QString cmd = "dpkg-parsechangelog";
    QStringList arg;
    arg.clear();
    arg << "-l" << CHANGELOG_PATH << "--show-field" << "Version";
    process.start(cmd,arg);
    process.waitForFinished();
    QByteArray result = process.readAllStandardOutput();
    result = result.left(result.length()-1);
    /*
        dpkg -l 作为候选方案
    */
    if( "" == result){
        QStringList options;
        options << "-l" << "|" << "grep" << "kylin-user-guide";
        process.start("dpkg", options);
        process.waitForFinished();
        QString retStr = QString::fromLocal8Bit(process.readAllStandardOutput());
        QStringList retList = retStr.trimmed().split("\n");
        QStringList verList = retList[retList.length()-1].split(" ");
        if(verList.length() >= 4 ){
            result = verList[3].trimmed().toUtf8();
        }
    }
    
    return result;
}

int main(int argc, char *argv[])
{
    initUkuiLog4qt("kylin-user-guide");
    qInfo() << " ---------------------------------- main start !!!! --------------------------------------  " ;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    qputenv("QT_QPA_PLATFORMTHEME", "ukui");
    qInfo() << " ---------------------------------- QApplication init start !!!! --------------------------------------  " ;
    QApplication app(argc, argv);
    qInfo() << " ---------------------------------- QApplication init finished !!!! --------------------------------------  " ;
    QStringList args = app.arguments();
    QCommandLineOption jumpAppOption(QStringList()<< "A" << "appName","指定打开应用程序版本手册","");
//    QCommandLineOption asDaemonOption(QStringList()<< "D" << "daemon","作为后台demon，显示图形");
    QCommandLineParser cmdinParser;

    cmdinParser.setApplicationDescription("ubuntukylin user guide");
    cmdinParser.addHelpOption();
    cmdinParser.addVersionOption();
    cmdinParser.addOption(jumpAppOption);
//    cmdinParser.addOption(asDaemonOption);

    cmdinParser.addPositionalArgument("--help","显示全部帮助选项");
    cmdinParser.addPositionalArgument("--version","显示安装的程序的版本并退出");
    cmdinParser.addPositionalArgument("--display","要使用的X显示");

    cmdinParser.process(args);
    bool bJumpApp = cmdinParser.isSet(jumpAppOption);
//    QString jumpApp = cmdinParser.value("-A");//拿不到值
    QString jumpApp = "";
    if(argc>2&&bJumpApp)
    {
        jumpApp = args.at(2);
//        if(1==jumpApp.split(":"))//dbus调用没有带:
        if(2==jumpApp.split(":").length())//标准帮助URL help:xxx,部分开源app调用模式，新app可以统一用help:标准方式
        {
            jumpApp = jumpApp.split(":").at(1);
        }
        
        QStringList gamesList,toolsList,softwareStoreList,ukuiList;
        // //games
        // gamesList << "iagno" << "gnome-mines" << "gnome-tetravex";
        // //tools
        /* toolsList << "onboard" << "atril" << "weather"<< "kylin-recorder" << "kylin-usb-creator" << "kylin-screenshot"\
                   << "kylin-calculator" << "kylin-camera" << "kylin-scanner";*/
        //ukui
        // ukuiList << "ukui_clock" << "ukui_notebook";
        //software-store
        softwareStoreList << "error-code";
        if(softwareStoreList.contains(jumpApp))
            jumpApp = "software-store/"+jumpApp;
        // if(ukuiList.contains(jumpApp))
        //     jumpApp = "ukui/"+jumpApp;
        // if(gamesList.contains(jumpApp))
        //     jumpApp = "game/"+jumpApp;
        // if(toolsList.contains(jumpApp))
        //     jumpApp = "tools/"+jumpApp;
//        QMessageBox::information(NULL, "Title", jumpApp,
//                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }

//    qDebug()<<"args"<<args<< "  jumpApp =" << jumpApp <<"       bJumpApp=" << bJumpApp;

    // if(signal(SIGCHLD,SIG_IGN)==SIG_ERR)//忽略子进程已经停止或退出
    // {
    //     //注册SIGSEGV信号失败
    //     perror("signal error");
    // }
    if(signal(SIGSEGV,crashHandler)==SIG_ERR)//无效内存段访问
    {
        //注册SIGSEGV信号失败
        perror("signal error");
    }
    if(signal(SIGILL,crashHandler)==SIG_ERR)//非法指令
    {
        //注册SIGILL信号失败
        perror("signal error");
    }
    if(signal(SIGHUP,crashHandler)==SIG_ERR)//系统挂断
    {
        //注册SIGHUP信号失败
        perror("signal error");
    }
    if(signal(SIGABRT,crashHandler)==SIG_ERR)//进程停止运行
    {
        //注册SIGABRT信号失败
        perror("signal error");
    }

    // 默认设置中文
    app.setApplicationName(APPLICATION_NAME);
    QLocale locale;
    if(locale.language()==QLocale::Chinese)
    {
        gLang = "zh_CN";
    } else if (locale.language()==QLocale::Tibetan){
        // gLang = "bo_CN";
        // gLang = "en_US";
        gLang = "zh_CN";
        //设置成藏文
        app.setApplicationName(APPLICATION_NAME_BO);
    } else if (locale.language() == QLocale::Mongolian) {
        gLang = "zh_CN";
        app.setApplicationName(APPLICATION_NAME_MN);
    } else {
        gLang = "en_US";
        //设置成英文
        app.setApplicationName(APPLICATION_NAME_EN);
    }

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        gLang = "zh_CN";
    }

    if(systemLang == "ug_CN"){
        app.setApplicationName(APPLICATION_NAME_UG);
    }

    if(systemLang == "ky_KG"){
        app.setApplicationName(APPLICATION_NAME_KG);
    }

    if(systemLang == "kk_KZ"){
        app.setApplicationName(APPLICATION_NAME_KK);
    }
//    app.setQuitOnLastWindowClosed(true);
#ifdef APP_USE_QSS
    //加载qss样式表
    QFile qss(":kylin-user-guide.qss");
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
#endif

    QString qm_name = QLocale::system().name();
//        QString locale = "bo_CN";
    QTranslator translator;
    QTranslator trans;
    if(!translator.load("kylin-user-guide_" + qm_name + ".qm",
                        "/usr/share/kylin-user-guide/translations/"))
        qDebug() << "Load translation file："<< "kylin-user-guide_" + qm_name + ".qm" << " failed!";
    else
        app.installTranslator(&translator);
    // sdk翻译加载
    if(trans.load(":/translations/gui_"+qm_name+".qm"))
    {
        app.installTranslator(&trans);
    }

    //加载Qt对话框默认的国际化
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + qm_name,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    gStartShowApp = jumpApp;
    qInfo() << " ---------------------------------- MainController init start !!!! --------------------------------------  " ;
    MainController *ctrl = MainController::self();
    app.exec();

    delete ctrl;
    return 0;

}
