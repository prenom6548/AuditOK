#include "mainwindow.h"
#include "rebootinstallnotify.h"
#include "desktopnotify.h"
#include "checkpower.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QTranslator>
#include <qlogging.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#ifdef KYDEBUG
#ifndef __sw_64__
#include <ukui-log4qt.h>
#endif
#endif
int main(int argc, char *argv[])
{
    setenv("QT_DEBUG_PLUGINS", "1", 0);
    QApplication a(argc, argv);
    /*mainwindows为service弹窗
      desktopnotify为desktop弹窗
      其余类别为执行命令的弹窗（各种功能）*/
#ifdef KYDEBUG
#ifndef __sw_64__
    initUkuiLog4qt("kylin-update-frontend-notifysend");
#endif
#endif


    /*以后需要写配置文件，通过标志位判断某个弹窗是否弹窗*/

/*此部分为demo
    QCommandLineOption op1("a");// 短名称，无参数
    QCommandLineOption op2("BBB");// 长名称，无参数
    QCommandLineOption op3(QStringList() << "c" << "CCC");// 多个名称，无参数
    QCommandLineOption op4("D", "", "Name");// 短名称，有参数，无默认值
    QCommandLineOption op5("Age", "", "", "18");// 长名称，有参数，有默认值
    QCommandLineOption op6("m", "", "."); // 短名称，有参数，无默认值
    QCommandLineParser parser;
    parser.addOption(op1);
    parser.addOption(op2);
    parser.addOption(op3);
    parser.addOption(op4);
    parser.addOption(op5);
    parser.addOption(op6);
    parser.process(a);
    qInfo() << parser.isSet(op1); // true
    qInfo() << parser.isSet("BBB"); // true
    qInfo() << parser.isSet("CCC"); // true
    qInfo() << parser.value(op4); // "Jack"
    qInfo() << parser.value(op5); // "18"
    qInfo() << parser.values(op6);// ("Hello", "World")
    qInfo() << parser.helpText();
*/
    QTranslator translator ;
    qInfo()<<"the system.name is "<<QLocale::system().name();
    translator.load("/usr/share/upgradenotifysend/translations/" + QLocale::system().name() +".qm");
    a.installTranslator(&translator);

    QCommandLineParser parser;
    QCommandLineOption waitsignalnotify("WaitSignalNotify");
    QCommandLineOption rebootinstallnotify("RebootInstallNotify");
    QCommandLineOption desktopnotify("DesktopNotify");
    QCommandLineOption powerchecknotify("PowercheckNotify");
    parser.addOption(waitsignalnotify);
    parser.addOption(rebootinstallnotify);
    parser.addOption(desktopnotify);
    parser.addOption(powerchecknotify);
    parser.process(a);
    qInfo() << parser.isSet(waitsignalnotify);
    qInfo() << parser.isSet(rebootinstallnotify); // true
    qInfo() << parser.isSet(desktopnotify);
    qInfo() << parser.isSet(powerchecknotify);
    qInfo() << parser.helpText();
    /*分步更新的稍后提醒功能*/
    if(parser.isSet(rebootinstallnotify))
    {/*24小时等待*/
        qInfo()<<"[INFO]get in rebootinstallnotify";
        RebootInstallNotify *Rebootinstallnotify = new RebootInstallNotify();
    }
    if(parser.isSet(desktopnotify))
    {/*开机自启检测是否需要重启*/
        qInfo()<<"[INFO]get in desktopnotify";
        DesktopNotify *Desktopnotify = new DesktopNotify();
    }
    if(parser.isSet(powerchecknotify))
    {/*电量检测*/
        qInfo()<<"[INFO]get in powerchecknotify";
        CheckPower *Powerchecknotify = new CheckPower();
    }
    if(parser.isSet(waitsignalnotify))
    {/*接受信号弹窗*/
        qInfo()<<"[INFO] get in MainWindow";
        MainWindow *w = new MainWindow();
    }


//    w.show();
    return a.exec();
}
