/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "installermainwidget.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <QDebug>
#include <QLibraryInfo>
#include "klogger.h"
#include "ksetting.h"
#include "ksystemenv.h"
#include "kcommand.h"
#include "kdir.h"
#include "ksystemsetting_unit.h"
#include "../PluginService/base_unit/file_unit.h"
#include "installermainwidget_auto.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
//#include <X11/extensions/Xrandr.h>
using namespace KServer;
using namespace KInstaller;



void x11_get_screen_size(int *width,int *height)
{
    Display* display;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot connect to X server %s/n", "simey:0");
        exit (-1);
    }
    int screen_num;

    screen_num = DefaultScreen(display);

    *width = DisplayWidth(display, screen_num);
    *height = DisplayHeight(display, screen_num);
    XCloseDisplay(display);

}

void XsettingsHidpi()
{
    Display    *dpy;
    int w,h;
    x11_get_screen_size (&w, &h);
    dpy = XOpenDisplay (NULL);
    if(h >= 2000 || (w>2560 && h> 1500)){

        XChangeProperty(dpy, RootWindow (dpy, 0),
            XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) "Xft.dpi:	192\n", 13);
    }else{
        XChangeProperty(dpy, RootWindow (dpy, 0),
            XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) "Xft.dpi:	96\n", 12);
    }
    XCloseDisplay (dpy);
}






void loadSetConfFile(QString flagstr)
{
    if(flagstr == "oem1") {
        KInstaller::Ksetting::init(":/data/file/ksettingsoem1.conf");
        KServer::WriteSettingToIni("config","first_mode","oem1");
    } else if(flagstr == "oem2") {
        KInstaller::Ksetting::init(":/data/file/ksettingsoem2.conf");
         KServer::WriteSettingToIni("config","first_mode","oem2");
    } else {
        KInstaller::Ksetting::init(":/data/file/ksettings.conf");
         KServer::WriteSettingToIni("config","first_mode","normal");
    }
}
int main(int argc, char *argv[])
{
     //添加虚拟键盘
//    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    //如果系统中已有实例程序在运行，则退出
//    QString exePath = "/usr/bin/kylin-os-Installer";
//    int fd = open(exePath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

//    if (fd < 0)
//        exit(1);
//    if (lockf(fd, F_LOCK, 0)) {
//        syslog(LOG_ERR, "Can't lock single file, kylin-os-installer is already running!");
//        qDebug()<<"Can't lock single file, kylin-os-installer is already running!";
//        exit(0);
//    }
    QString strLockFile = QDir(QDir::tempPath()).absolutePath() + QLatin1Char('/') + "kylin_os_installer-lockfile";
    QFile qFileLock(strLockFile);
    qFileLock.open(QIODevice::ReadWrite);
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_WRLCK;
    if (fcntl(qFileLock.handle(), F_SETLK, &fl) == -1) {
        if (errno != EINTR && errno != EAGAIN)//出错了
            qWarning("QtLockedFile::lock(): fcntl: %s", strerror(errno));
        syslog(LOG_ERR, "Can't lock single file, kylin-os-installer is already running!");
        qDebug()<<"Can't lock single file, kylin-os-installer is already running!";
        exit(0);
    }

    XsettingsHidpi();
#if 0
    //自适应高分屏
    Display *display = XOpenDisplay(NULL);
    Screen *scrn = DefaultScreenOfDisplay(display);

    if(scrn == nullptr) {
        return 0;
    }
    int width = scrn->width;
    if(width >= 2560) {
#if(QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

        XCloseDisplay(display);

#endif
    }
#endif
#if(QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    if(argc > 1 && QString(QLatin1String(argv[1])).contains("oem") && !QString(QLatin1String(argv[2])).contains("automatic") ) {//oobe
    	qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    }else{
    	int x=0;
    	int y=0;
    	x11_get_screen_size (&x, &y);
    	if(x>=3000){
        	qputenv("QT_SCREEN_SCALE_FACTORS", "2");
    	}
    	else if(y >= 2000){
        	qputenv("QT_SCREEN_SCALE_FACTORS", "2");
    	}
    	else if(x >= 2000){
            qputenv("QT_SCREEN_SCALE_FACTORS", "1.5");
    	}
    	else if(y >= 1500){
            qputenv("QT_SCREEN_SCALE_FACTORS", "1.5");
    	}
    	else{
        	qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
    	}
    }

    //qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    //自适应高清屏后，设置字体大小的改变
    QFont font = a.font();
    //这个值根据自身情况自行调节，如果字体没变化，把setPointSize()内的值继续增大
    font.setPointSize(font.pointSize());
    a.setFont(font);

    //设置系统主题样式
//  a.setStyle("ukui-default");

    if(argc > 1 && QString(QLatin1String(argv[1])).contains("oem") && !QString(QLatin1String(argv[2])).contains("automatic") ) {//oobe
        KServer::setLogPath(QDir::homePath() + LOG_FILE_NAME);
    }else{
        KServer::setLogPath("/var/log/installer" + LOG_FILE_NAME);
    }
    qInstallMessageHandler(KServer::customLogMessageHandle);
#if 0
    //添加日志,QCommandLineParser解析应用的命令行参数
    QCommandLineParser parser;
    QCommandLineOption debugOpention({"d", "debug"}, QObject::tr("Show debug informations"));

    parser.addOptions({debugOpention});
    parser.process(a);

    if(parser.isSet(debugOpention)) {
        KServer::setLogLevel(QtDebugMsg);
        KServer::setLogPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + LOG_FILE_NAME);
        qInstallMessageHandler(KServer::customLogMessageHandle);
    } else {
        setLogLevel(QtWarningMsg);
    }
#endif

//    KInstaller::Ksetting::init(":/data/file/ksettings.conf");
    KServer::KSystemENV::init();

    //("sudo", {"-u", "kylin", "gsettings", "set" ,"org.ukui.flash-disk.autoload" ,"ifautoload", "false"});
    //root  ->  kylin

    InstallerMainWidget* w = nullptr;
    InstallerMainWidget_auto* w_auto = nullptr;
    //合并配置信息
   // updataIni();

    if(argc > 1) {
        if(QString(QLatin1String(argv[1])).contains("oem")) {
            QString strtype = "oem2";
            if(QString(QLatin1String(argv[2])).contains("automatic")) {
                w_auto = new InstallerMainWidget_auto(true);
                w_auto->show();
            } else {
		setModel(strtype);
                loadSetConfFile(strtype);
                w = new InstallerMainWidget;
                w->setInstallMode(strtype);
                w->show();
            }
        }
    } else {
        //2021-3-29 huawei swap file swapoff
        KServer::KCommand::initCmd()->getInstance()->RunScripCommand("swapoff", {"-a"});
        KServer::KCommand::initCmd()->getInstance()->RunScripCommand("sudo", {"-u", "kylin", "gsettings", "set" ,"org.ukui.flash-disk.autoload" ,"ifautoload", "false"});
        QString readfile = KServer::KReadFile("/proc/cmdline");
        if(readfile.contains("automatic") ||
                readfile.contains("ple-mode") ||
                readfile.contains("test-mode")) {
            loadSetConfFile("");
            w_auto = new InstallerMainWidget_auto(false);
            w_auto->show();

        } else if(KServer::KReadFile("/proc/cmdline").contains("oem-config/enable=true")) {
            QString strtype = "oem1";
            loadSetConfFile(strtype);
            w = new InstallerMainWidget;
            w->setInstallMode(strtype);
            w->show();
        } else {
            loadSetConfFile("");
            w = new InstallerMainWidget;
            w->show();
        }

    }


//    QTranslator* translator = new QTranslator;
//    QString path = KServer::GetKylinInstallPath() + "/language/" + QLocale::system().name() + ".qm";
//    translator->load(path);
//    qApp->installTranslator(translator);

//    QTranslator* translator1 = new QTranslator;
//    translator1->load(KServer::GetKylinInstallPath() + "/language/qt_zh_CN.qm");
//    qApp->installTranslator(translator1);


    int result = a.exec();
    if(w_auto) {
        delete w_auto;
    }
    if(w) {
        delete w;
    }

    return result;
}
