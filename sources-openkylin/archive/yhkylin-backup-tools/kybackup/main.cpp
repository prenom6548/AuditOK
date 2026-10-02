/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "maindialog.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include <QCommandLineParser>
#include <QtDBus>
#include <QMessageBox>
#include <QDesktopWidget>
#include <signal.h>
#include <unistd.h>
#include "qtsingleapplication/qtsingleapplication.h"
#include "../common/utils.h"
#include "xatom-helper.h"
#include "globalbackupinfo.h"
#include "messageboxutils.h"
#include "backup_manager_interface.h"
#include <libkydatacollect.h>
#include "qtsingleapplication/qtlocalpeer.h"
#include <QScreen>
// 声明
void initApp(QApplication& a);
bool isManager();
void remountPartitionsForCasperMode();
void centerToScreen(QWidget* widget);
void showMessageBox();

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QtSingleApplication a ("kybackup", argc, argv);


    if (a.isRunning()) {
        qDebug() << "isRunning!" ;
        QString strUid = QString::number(getuid());
        QString ack = a.sendMessage(strUid, 3000);
        qDebug() << "strUid:" << strUid ;
        qDebug() << "ack:" << ack ;

        if (strUid != ack) {
            showMessageBox();
        }

        //此时只是退出，不对锁文件等做析构操作
        QtLocalPeer::isRealClose = false;
        return EXIT_SUCCESS;
    }



    MainDialog w;
    // 居中窗口
    centerToScreen(&w);
    a.setWindowIcon(QIcon::fromTheme(THEME_YHKYLIN_BACKUP_TOOLS, QIcon(":/images/yhkylin-backup-tools.png")));
    a.setActivationWindow(&w, true);
    a.setApplicationName(QObject::tr("Backup & Restore"));
    a.setApplicationVersion("4.1.0.0-0k21.3");

    //如果是第一个实例,则绑定,方便下次调用
    QObject::connect(&a,SIGNAL(messageReceived(const QString&)),&w,SLOT(sltMessageReceived(const QString&)));

    w.show();

    QTimer::singleShot(50,[&a,&w](){
        //加速主界面启动时间，低至300ms
        initApp(a);
        w.initUI();
        w.initConnect();

        //数据埋点：开始
        kdk_dia_upload_program_startup();
    });

    //Utils::inhibit();

    return a.exec();
}

/**
 * @brief 应用初始化
 * @param a
 * @note  new出来的两个QTranslator不用删除，需要整个生命周期内有效，应用结束由操作系统收回即可
 */
void initApp(QApplication& a)
{
    // 前端向后端传递QString参数，若参数中含有中文则保证不会乱码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // 区分中英文
    QString locale = QLocale::system().name();

    // QT自身标准的翻译
#ifndef QT_NO_TRANSLATION
    QTranslator *selfTransOfQt = new QTranslator();
    if (selfTransOfQt->load(QLocale(), QString("qt"), QString("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        a.installTranslator(selfTransOfQt);
    else
        qDebug() << "load qt translator file failed!";
#endif

    // 部分sdk控件用到翻译文件
    QTranslator *transGui = new QTranslator();
    if (locale == "zh_CN") {
        if (transGui->load(":/translations/gui_zh_CN.qm"))
        {
            a.installTranslator(transGui);
        }
    } else if (locale == "bo_CN") {
        if (transGui->load(":/translations/gui_bo_CN.qm"))
        {
            a.installTranslator(transGui);
        }
    } else if (locale == "mn_MN" || locale == "mn") {
        if (transGui->load(":/translations/gui_mn_MN.qm")) {
            a.installTranslator(transGui);
        }
    } else if (locale == "zh_HK") {
        if(transGui->load(":/translations/gui_zh_HK.qm")) {
            a.installTranslator(transGui);
        }
    }else if (locale == "kk_KZ") {
        if(transGui->load(":/translations/gui_kk.qm")) {
            a.installTranslator(transGui);
        }
    }else if (locale == "ug_CN") {
        if(transGui->load(":/translations/gui_ug.qm")) {
            a.installTranslator(transGui);
        }
    }else if (locale == "ky_KG") {
        if(transGui->load(":/translations/gui_ky.qm")) {
            a.installTranslator(transGui);
        }
    }

    // 应用内的翻译
    QTranslator *translator = new QTranslator();
    QString translatorResourceName(":/language/qt_");
    translatorResourceName += locale;
    translatorResourceName += ".qm";
    if (!translator->load(translatorResourceName)) //qtcreator启动后看到在资源目录下
        qDebug() << translatorResourceName << "load translator file failed!";
    else {
        a.installTranslator(translator);
        qDebug() << translatorResourceName << "load translator file success!";
    }
//    if (locale == "zh_CN") {
//        //中文需要翻译
//        if (!translator->load(":/language/qt_zh_CN.qm")) //qtcreator启动后看到在资源目录下
//            qDebug() << "load translator file failed!";
//        else
//            a.installTranslator(translator);
//    } else if (locale == "bo_CN") {
//        //藏文需要翻译
//        if (!translator->load(":/language/qt_bo_CN.qm")) //qtcreator启动后看到在资源目录下
//            qDebug() << "load translator file failed!";
//        else
//            a.installTranslator(translator);
//    } else if (locale == "mn_MN") {
//        //蒙古文需要翻译
//        if (!translator->load(":/language/qt_mn_MN.qm")) //qtcreator启动后看到在资源目录下
//            qDebug() << "load translator file failed!";
//        else
//            a.installTranslator(translator);
//    }

    // 命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("kybackup helper");
    parser.addHelpOption();
    parser.addVersionOption();
    // 启动后进入的页面
    QCommandLineOption functionOption(QStringList() << "r" << "restore", QCoreApplication::translate("restore", "system restore"));
    parser.addOption(functionOption);
    parser.process(a);
    GlobelBackupInfo::instance().setHasArgRestore(parser.isSet(functionOption));

    Utils::initSysRootPath();
    Utils::initSystemInfo();

    // 初始化日志
    QString logFile = QDir::homePath() + "/.log/kybackup.log";
    logFile.replace("//", "/");
    Utils::setLogFile(logFile);
    qInstallMessageHandler(Utils::customMessageHandler);

    // 当前操作员是否管理员用户
    GlobelBackupInfo::instance().setIsManager(isManager());

    // 试用模式运行时需要将原系统的根分区挂载到/target目录，并将其它分区分别挂载，以备后面系统还原使用
    if (GlobelBackupInfo::instance().isCasper())
    {
        remountPartitionsForCasperMode();
    }
}

/**
 * @brief 判断启动账号是否管理员账号
 * @return true-相当于是；false-不是
 */
bool isManager()
{
    uid_t uid = getuid();
    QString sid = QString::number(uid);
    GlobelBackupInfo::instance().setCurUserId(sid);

    QString rootPath = Utils::getSysRootPath();
    // 只正常启动程序时需校验是否管理员账号启动
    if ("/" != rootPath) {
        // 试用模式运行
        GlobelBackupInfo::instance().setIsCasper(true);
        return true;
    }

    // root用户
    if (0 == uid)
        return true;

    QString userObject = "/org/freedesktop/Accounts/User" + sid;

    // 创建QDBusInterface
    QDBusInterface iface("org.freedesktop.Accounts", userObject, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    if (!iface.isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        exit(1);
    }

    int result = 0;
    QDBusReply<QDBusVariant> reply = iface.call("Get", QString("org.freedesktop.Accounts.User"), QString("AccountType"));
    if (reply.isValid()) {
        QVariant val = reply.value().variant();
        result = val.toInt();
    }

    return 0 == result ? false : true;
}

/**
 * @brief CasperMode重新挂载分区
 * @note 试用模式使用备份还原时需将各个分区挂载到/target目录，当前进入试用模式后，各个分区是挂载到/media/xxx/目录下的，为了方便还原重新挂载到/target目录
 */
void remountPartitionsForCasperMode()
{
    // 卸载/backup分区
    ComKylinBackupManagerInterface intf("com.kylin.backup", "/", QDBusConnection::systemBus());
    intf.remountPartitionsForCasperMode();
}

/**
 * @brief 居中显示窗口
 * @param widget
 */
void centerToScreen(QWidget* widget)
{
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    // QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    QRect desk_rect = m->screenGeometry(widget);
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());

    // 获取主屏幕
    QScreen *primaryScreen = QApplication::primaryScreen();
    QRect primaryRect = primaryScreen->availableGeometry();  // 主屏幕的矩形
    // 获取当前窗口位置
    QRect windowRect = widget->geometry();
    // 检查窗口中心是否在主屏幕上
    if (!primaryRect.contains(windowRect.center())) {
        // 如果不在主屏幕上，移动到主屏幕
        int x = primaryRect.x() + (primaryRect.width() - widget->width()) / 2;
        int y = primaryRect.y() + (primaryRect.height() - widget->height()) / 2;
        widget->move(x, y);
    }
}

/**
 * @brief 同一时间如果有其他用户尝试启动第二个实例，本地化弹窗提示（性能优化将加载翻译过程滞后，因此这里单独本地化）
 */
void showMessageBox()
{
    QString locale = QLocale::system().name();
    if (locale == "zh_CN") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "其他用户已开启备份还原工具，不能再开启",
                                              "确定");
    } else if (locale == "bo_CN") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "སྤྱོད་མཁན་གཞན་དག་གིས་ད་དུང་ཐོག་མའི་ཡོ་བྱད་སྤྲད་ཟིན་པས་སླར་ཡང་བློ་སྒོ་འབྱེད་མི་རུང་།",
                                              "ངེས་གཏན།");
    } else if (locale == "mn_MN" || locale == "mn") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "ᠪᠤᠰᠤᠳ ᠬᠡᠷᠡᠭᠯᠡᠭᠴᠢ ᠨᠦᠭᠡᠴᠡ ᠵᠢ ᠡᠬᠡᠬᠦᠯᠬᠦ ᠪᠠᠭᠠᠵᠢ ᠵᠢ ᠨᠡᠬᠡᠬᠡᠭᠰᠡᠨ᠂ ᠳᠠᠬᠢᠵᠤ ᠨᠡᠬᠡᠬᠡᠵᠤ ᠪᠤᠯᠬᠤ ᠦᠬᠡᠢ᠃",
                                              "ᠳᠤᠰᠢᠶᠠᠬᠤ");
    } else if (locale == "zh_HK") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "其他使用者已開啟備份還原工具，不能再開啟",
                                              "確定");
    }else if (locale == "kk_KZ") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "جانە بٸر الارمان kybackup نى اشتٸ، ٴسىز ونى قاتە قوزغىتالمايٴسىز.",
                                              "بەكىتۉۉ");
    }else if (locale == "ug_CN") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              " kybackup نى ئاچتى، سىز ئۇنى قايتا قوزغىتالمايسىز.",
                                              "جەزملەشتۈرۈش");
    }else if (locale == "ky_KG") {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "داعى بىر  كەرەكتۅۅچۉ kybackup نى اچتى، سىز انى  قايرا قوزعوتوو البايسىز.",
                                              "ماقۇل");
    } else {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(nullptr, QObject::tr("Information"),
                                              "Another user had opened kybackup, you can not start it again.",
                                              "OK");
    }
}

