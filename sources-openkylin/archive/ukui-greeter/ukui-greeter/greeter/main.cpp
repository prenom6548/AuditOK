/* main.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "greeterwindow.h"
#include <stdio.h>
#include <string>
#include <QApplication>
#include <QTextCodec>
#include <QResource>
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QPoint>
#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QX11Info>
#include <QProcess>
#include <QScreen>
#include <QDesktopWidget>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include "globalv.h"
#include "mainwindow.h"
#include "display-switch/displayswitch.h"
#include "display-switch/greeterscreen.h"
#include "xeventmonitor.h"
#include "configuration.h"
#include "usersmodel.h"
#include "greeterwrapper.h"
#include "greeterservice.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libintl.h>
#include "pluginsloader.h"
#include "common/autorotatescreen.h"
#include "common/touchscreenset.h"

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QByteArray localMsg = msg.toLocal8Bit();
    switch(type) {
    case QtDebugMsg:
        fprintf(stderr, "%s Debug: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        fprintf(stderr, "%s Info: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "%s Warnning: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s Critical: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s Fatal: %s:%u: %s\n", time.constData(), context.file, context.line, localMsg.constData());
        abort();
    }

}

void waitMonitorsReady()
{
    int n;

    QScreen *screen = QApplication::primaryScreen();
    while(n <= 0 && screen->geometry().x() <= 0 && screen->geometry().y() <= 0
          && screen->geometry().width() <= 0 && screen->geometry().height() <= 0){
        XRRGetMonitors(QX11Info::display(), QX11Info::appRootWindow(), false, &n);
        if(n == -1)
            qWarning() << "get monitors failed";
        else if(n > 0){
            qDebug() << "found " << n << " monitors";
        }
        //启动xrandr，打开视频输出， 自动设置最佳分辨率
        QProcess enableMonitors;
        enableMonitors.start("xrandr --auto");
        enableMonitors.waitForFinished(-1);
    }
}

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

void setXresources(int dpi)
{
    Display    *dpy;
    QString str = QString("Xft.dpi:\t%1\n")
            .arg(dpi);

    dpy = XOpenDisplay (NULL);
    XChangeProperty(dpy, RootWindow (dpy, 0),
                    XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) str.toLatin1().data(), str.length());

    XCloseDisplay (dpy);
}

/* 判断文件是否存在 */
bool isFileExist(QString XresourcesFile)
{
    QFileInfo fileInfo(XresourcesFile);
    if(fileInfo.isFile()){
        qDebug()<<"file is true";
        return true;
    }
    qDebug()<<"file is false";
    return false;
}
/* 写配置文件并设置DPI和鼠标大小*/
void WriteXresourcesFile(QString XresourcesFile,int dpi)
{
    QFile file(XresourcesFile);
    QString content = QString("Xft.dpi:%1\n").arg(dpi);
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QByteArray str = content.toLatin1().data();
    file.write(str);
    file.close();
}

/* 配置新装系统、新建用户第一次登陆时，4K缩放功能*/
void Set4KScreenScale()
{
    Display    *dpy;
    int w,h;
    x11_get_screen_size (&w, &h);

    bool res;
    QString homePath = getenv("HOME");
    QString XresourcesFile = homePath+"/.config/xresources";
    res = isFileExist(XresourcesFile);
    if(!res){
	if(h >= 2000 || (w>2560 && h> 1500)){
            WriteXresourcesFile(XresourcesFile,192);
	}else{
	    WriteXresourcesFile(XresourcesFile,96);
	}
    }
    if(h >= 2000 || (w>2560 && h> 1500)){
        setXresources(192); 
    }else{
        setXresources(96);
    }
}

void XsettingsHidpi()
{
    Display    *dpy;
    int w,h;
    x11_get_screen_size (&w, &h);
    if(h >= 2000 || (w>2560 && h> 1500)){
        dpy = XOpenDisplay (NULL);
        XChangeProperty(dpy, RootWindow (dpy, 0),
        XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) "Xft.dpi:	192\n", 13);
        XCloseDisplay (dpy);
    }
}

static bool XsettingsNormaldpi()
{
    Display    *dpy;
    dpy = XOpenDisplay (NULL);
    if (dpy) {
        QString str = QString("Xft.dpi:\t%1\nXcursor.size:\t%2\n")
                .arg(96)
                .arg(24);
        XChangeProperty(dpy, RootWindow (dpy, 0),
        XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) str.toLatin1().data(), str.length());
        XCloseDisplay(dpy);
        return true;
    }
    return false;
}

static QString getWillLoginUserName()
{
    //显示lightdm传过来的被选中的用户且自动进入认证界面 -- SwitchToUser()
    QString currentUser = "";
    UsersModel *userModel = new UsersModel(GreeterWrapper::instance()->hideUsersHint());
    WillLoginUserInfo willSwitchUser = GreeterService::instance()->getWillSwitchUser();
    QString selectedUser = "";
    if (willSwitchUser.strUserName.isEmpty() || !GreeterService::instance()->isUserNameValid(willSwitchUser.strUserName)) {
        qDebug()<<"WillSwitchUser is NULL or invalid:"<<willSwitchUser.strUserName;
        selectedUser = GreeterWrapper::instance()->selectUserHint();
    }
    bool isSelectUsr = false;
    if (!willSwitchUser.strUserName.isEmpty()) {
        currentUser = willSwitchUser.strUserName;
        isSelectUsr = true;
        qDebug()<<"WillSwitchUser:"<<currentUser;
    }
    else if(!selectedUser.isEmpty())
    {
        qDebug() << "SelectUserHint: " << selectedUser;
        bool isExistUser = false;
        for(int i = 0; i < userModel->rowCount(); i++){
            QString name = userModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
            if(name == selectedUser) {
                isExistUser = true;
                break;
            }
        }
        if (isExistUser) {
            currentUser = selectedUser;
            isSelectUsr = true;
        } else {
            qDebug()<<"SlelectUerHint invalid:"<<selectedUser;
        }
    }
    if (isSelectUsr) {
        // do nothing
    }
    // SwitchToGuest()且自动进入认证界面
    else if(GreeterWrapper::instance()->selectGuestHint())
    {
        qDebug() << "SelectGuest";
    }
    //如果没有可显示出来的用户，则开启手动输入用户名登录
    else if(userModel->rowCount() == 0)
    {
        qDebug()<<"Only manual login!!";
    }
    //如果只有一个用户，直接进入认证界面
    else if(userModel->rowCount() == 1)
    {
        QString userName = userModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
        currentUser = userName;
    }
    else
    {
        QString lastLoginUser = Configuration::instance()->getLastLoginUser();
        bool bFound = false;
        for(int i = 0; i < userModel->rowCount(); i++){
            QString name = userModel->index(i).data(QLightDM::UsersModel::NameRole).toString();
            if(name == lastLoginUser) {
                bFound = true;
                break;
            }
        }
        if (bFound) {
            currentUser = lastLoginUser;
        } else {
            QString userName = userModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
            currentUser = userName;
        }
    }
    if (userModel) {
        userModel->deleteLater();
    }
    qDebug()<<"Will login user:"<<currentUser;
    return currentUser;
}

// 设置一下鼠标的居于主屏中央
void setCursorCenter() {
    auto setCursorPos = [ = ] (QPoint p) {
        /*Display *dpy;
        dpy = XOpenDisplay(0);

        if (!dpy) return;

        Window root_window;
        root_window = XRootWindow(dpy, 0);
        XSelectInput(dpy, root_window, KeyReleaseMask);
        XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, p.x(), p.y());
        XFlush(dpy);*/
        QCursor::setPos(p);
    };

    if (!qApp->primaryScreen()) {
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [ = ] {
            static bool first = true;
            if (first) {
                setCursorPos(qApp->primaryScreen()->geometry().center());
                first = false;
            }
        });
    } else {
        setCursorPos(qApp->primaryScreen()->geometry().center());
    }
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORMTHEME",QByteArray("ukui"));

    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    fprintf(stderr, "%s Debug:  %s\n", time.constData(),"--------------------------------------------------------开始，main函数开始执行");
    
    qInstallMessageHandler(outputMessage); //打印信息处理
    //Set4KScreenScale();
#if !USEKSCREEN
    XsettingsNormaldpi();
#if SURPORT_SAVEPARAM
    DisplayService::instance()->setCurUserName(getWillLoginUserName());
#else
    GreeterScreen* greeterScreen = new GreeterScreen();
    int nResult = 0;
    if (greeterScreen) {
        QString strMsg = greeterScreen->setGreeterConfigParam(&nResult);
        if (!nResult) {
            qDebug()<<"setGreeterConfigParam:"<<strMsg;
        }
        delete greeterScreen;
        greeterScreen = nullptr;
    }
#endif  // SURPORT_SAVEPARAM
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#else
    XsettingsNormaldpi();
#endif // !USEKSCREEN
    QApplication a(argc, argv);

    setCursorCenter();
    // 加载插件实例
    PluginsLoader *pluginsLoader = &PluginsLoader::instance();
    pluginsLoader->start(QThread::HighestPriority);
    pluginsLoader->wait();

    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~qapplication初始化完成";
    QResource::registerResource("image.qrc");

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    Configuration *m_configuration = Configuration::instance();
    //加载翻译文件
    m_configuration->m_trans =  new QTranslator();
    m_configuration->m_trans->load(QM_DIR + QString("%1.qm").arg(QLocale::system().name()));

    qApp->installTranslator( m_configuration->m_trans);
    
    //加载qss文件
//    QFile qss(":/ukui-greeter.qss");
//    qss.open(QFile::ReadOnly);
//    a.setStyleSheet(qss.readAll());

    MainWindow w;

    w.show();
    w.activateWindow();

    TouchScreenSet::instance()->setTouchMapScreen();
    //屏幕自动旋转
    QObject::connect(AutoRotateScreen::instance(), &AutoRotateScreen::screenRotate, [=](AutoRotateScreen::RotateDir dir){
        // 屏幕自动旋转后  对触摸屏进行坐标映射
        TouchScreenSet::instance()->setTouchMapScreen();
    });
    AutoRotateScreen::instance()->start();

//    qss.close();


    return a.exec();
}
