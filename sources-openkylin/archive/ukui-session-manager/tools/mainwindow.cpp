/*Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "mainwindow.h"
#include "commonpushbutton.h"
#include "powerprovider.h"

#include "mylistview.h"
//#include <QListView> //XTest.h/Xlib.h/XInput.h/X.h中定义了一个None,QStyleOption中也定义了None,会造成冲突，把QListView的头文件放到前面
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QException>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QScreen>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QX11Info>
#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include "grab-x11.h"
#include "xeventmonitor.h"
#include <QFileInfo>
#include <QDBusInterface>
#include <QTextStream>
#include <QTextBrowser>
#include <QStringListModel>
#include <QMessageBox>
#include <QPushButton>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QPainterPath>
#include <pwd.h>
#include <libkydate.h>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include "plasma-shell-manager.h"

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly,
                         int transposed = 0);
QT_END_NAMESPACE
#define BLUR_RADIUS         300
#define BACKGROUND_SETTINGS "org.mate.background"

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage   srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);

    //在设置Qt::WA_TranslucentBackground属性后，模糊图片会导致锁屏界面透明
    //不设置上面这个属性，在wayland模式下也出现了透明
    //因此这里修改image图形的alpha值为255.
    for (int y = 0;y < srcImg.height();++y) {
      QRgb *row = (QRgb*)srcImg.scanLine(y);
      for (int x = 0; x < srcImg.width(); ++x) {
        ((unsigned char*)&row[x])[3] = 255;
      }
    }

    painter.end();
    return QPixmap::fromImage(srcImg);
}

QString getUserName(QFile *a)
{
    QString user = getenv("USER");
    if (a->exists()) {
        a->open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream fileStream(a);
        int         k = 0;
        while (!fileStream.atEnd()) {
            QString line = fileStream.readLine();
            if (k == 0) {
                QString a = line;
                qDebug() << "uid=" << a;
                struct passwd *user1;
                user1 = getpwuid(a.toInt());
                qDebug() << "name=" << user1->pw_name << ",uid=" << user1->pw_uid;
                if (user1->pw_name == NULL) {
                    return user;
                }
                user = user1->pw_name;
            }
            k++;
        }
    }
    return user;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , m_power(new UkuiPower(this))
                                        , xEventMonitor(new XEventMonitor(this))
{
    const QByteArray bid(BACKGROUND_SETTINGS);
    if (QGSettings::isSchemaInstalled(bid)) {
        QGSettings    *gset = new QGSettings(BACKGROUND_SETTINGS, "", this);
        QString     fullstr = gset->get("picture-filename").toString();
        qDebug() << "picture path = " << fullstr;
        QFileInfo fileInfo(fullstr);
        if (fileInfo.isFile() && pix.load(fullstr)) {
            //增加对pix的判断，有些图片格式qt不支持，无法读取，导致pix为null,引起程序崩溃 #bug75856
            if (pix.isNull()) {
                pix.load(":/images/background-ukui.png");
                pix = blurPixmap(pix);
            } else {
                pix = blurPixmap(pix);
            }

        } else {
            QString   imagefile = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
            QFileInfo fileimage(imagefile);
            if (fileimage.isFile() && fileimage.exists() && pix.load(imagefile)) {
                pix = blurPixmap(pix);
            }
            else{
                pix.load("/usr/share/ukui/ukui-session-manager/images/background-ukui.png");
                pix = blurPixmap(pix);
            }
        }
        gset->deleteLater();
    } else {
        pix.load("/usr/share/ukui/ukui-session-manager/images/background-ukui.png");
        pix = blurPixmap(pix);
    }

    this->setObjectName("widget");
    m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());

    m_toolWidget = new QWidget(this);
    m_toolWidget->setGeometry(m_screen);

    qDebug() << "m_toolWidget width:" << m_toolWidget->width()<<m_toolWidget->height();
    m_vBoxLayout = new QVBoxLayout();
    m_buttonHLayout = new QGridLayout();
    m_dateTimeLayout = new QVBoxLayout();

    m_timeLabel = new QLabel();
    m_dateLabel = new QLabel();
    m_judgeWidgetVLayout = new QVBoxLayout();
    m_judgeBtnHLayout = new QHBoxLayout();
    m_judgeLabel = new QLabel();
    m_messageLabel1 = new QLabel();
    m_messageLabel2 = new QLabel();

    m_scrollArea = new QScrollArea;
    m_scrollArea->setObjectName("scrollArea");
    m_scrollArea->removeEventFilter(this);

    m_btnWidget = new QWidget();
    m_btnWidget->setObjectName("btnWidget");

    m_judgeWidget = new QWidget(this);
    m_judgeWidget->setObjectName("judgeWidget");

    //m_showWarningArea 作为该界面所有组件的父指针，方便排版
    m_showWarningArea = new QWidget(this);
    m_showWarningArea->setObjectName(QString::fromUtf8("area"));
    initialSystemMonitor();

    initialBtn();

    initialJudgeWidget();

    upgradeCheck = new systemUpgradeCheck();

    //Make a hash-map to store tableNum-to-lastWidget
    if (m_power->canAction(UkuiPower::PowerHibernate)) {
        isHibernateHide = false;
    }

    if (m_power->canAction(UkuiPower::PowerSuspend)) {
        isSuspendHide = false;
    }

    if (m_power->canAction(UkuiPower::PowerLogout)) {
        isLogoutHide = false;
    }

    if (m_power->canAction(UkuiPower::PowerMonitorOff)) {
        isLockscreenHide = false;
    }
    if (m_power->canAction(UkuiPower::PowerReboot)) {
        isRebootHide = false;
    }

    if (m_power->canAction(UkuiPower::PowerShutdown)) {
        isPowerOffHide = false;
    }

    if (LockChecker::getCachedUsers() > 1 && m_power->canAction(UkuiPower::PowerSwitchUser)) {
        isSwitchuserHide = false;
    }

    if (upgradeCheck->checkSystemUpgrade()) {
        isUpgradeThenPowerOffHide = false;
        isUpgradeThenRebootHide = false;
    }

    //天翼云屏蔽以下按钮
    QString platform = kdk_system_get_hostCloudPlatform();
    if ("ctyun" == platform) {
        isHibernateHide=true;
        isSuspendHide=true;
        isLockscreenHide=true;
        isRebootHide=true;
        isPowerOffHide=true;
        isSwitchuserHide=true;
    }

    m_btnHideMap.insert(m_switchUserBtn, isSwitchuserHide);
    m_btnHideMap.insert(m_hibernateBtn, isHibernateHide);
    m_btnHideMap.insert(m_suspendBtn, isSuspendHide);
    m_btnHideMap.insert(m_lockScreenBtn, isLockscreenHide);
    m_btnHideMap.insert(m_logoutBtn, isLogoutHide);
    m_btnHideMap.insert(m_upgradeThenRebootBtn, isUpgradeThenRebootHide);
    m_btnHideMap.insert(m_rebootBtn, isRebootHide);
    m_btnHideMap.insert(m_upgradeThenshutDownBtn, isUpgradeThenPowerOffHide);
    m_btnHideMap.insert(m_shutDownBtn, isPowerOffHide);

//    initialBtnCfg();
    map.insert(0, m_switchUserBtn);
    map.insert(1, m_hibernateBtn);
    map.insert(2, m_suspendBtn);
    map.insert(3, m_lockScreenBtn);
    map.insert(4, m_logoutBtn);
    map.insert(5, m_upgradeThenRebootBtn);
    map.insert(6, m_rebootBtn);
    map.insert(7, m_upgradeThenshutDownBtn);
    map.insert(8, m_shutDownBtn);

    gs = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");

    tableNum = -1;
    changeBtnState("empty");

    //不知为何，sdk接口要第二次创建才能获取正确的值
    //规避方案，这里先创建一次
    char *user = qgetenv("USER").data();
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(user);
    //QString  curTime     = QString(dateInfo->time);
    delete dateInfo;

    //更新显示时间的控件
    initialDateTimeWidget();
    QTimer *timeWidgetUpdateTimer = new QTimer(this);
    connect(timeWidgetUpdateTimer, &QTimer::timeout, this, &MainWindow::updateDateTimerWidget);
    timeWidgetUpdateTimer->start(1000);


    ResizeEvent();

    m_vBoxLayout->addStretch(20);
    m_vBoxLayout->addLayout(m_dateTimeLayout, 80);
    m_vBoxLayout->addStretch(120);
    m_vBoxLayout->addWidget(m_scrollArea, 640);
    m_vBoxLayout->addStretch(120);
    m_vBoxLayout->addWidget(m_systemMonitorBtn, 48, Qt::AlignHCenter);
    m_vBoxLayout->addStretch(62);
    m_vBoxLayout->setSpacing(0);

    m_vBoxLayout->setContentsMargins((m_screen.width() - m_scrollArea->width() - 20)/2,0,(m_screen.width() - m_scrollArea->width() - 20)/2,0);

    qDebug() << "width..........." << m_judgeLabel->width() << m_scrollArea->width() << m_messageLabel1->width() << m_messageLabel2->width();
    //根据屏幕分辨率与鼠标位置重设界面
    //m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    //setFixedSize(QApplication::primaryScreen()->virtualSize());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());
    move(0, 0);//设置初始位置的值

    //设置窗体无边框，不可拖动拖拽拉伸;为顶层窗口，无法被切屏;不使用窗口管理器
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    } else if (QString(qgetenv("XDG_SESSION_TYPE")) == "x11") {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
        setAttribute(Qt::WA_AlwaysShowToolTips);    
    }
    setFocusPolicy(Qt::NoFocus);

    // setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    m_toolWidget->setLayout(m_vBoxLayout);
    //setCentralWidget(m_toolWidget);

    qDebug() << "m_toolWidget...." << m_toolWidget->geometry();
    qDebug() << "pos..." << QCursor::pos() << this->geometry();
    qDebug() << "m_screen..." << m_screen;

    QString platformName = QGuiApplication::platformName();
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        PlasmaShellManager::getInstance()->setAppWindowKeepAbove(true);
    } else {
        /*捕获键盘，如果捕获失败，那么模拟一次esc按键来退出菜单，如果仍捕获失败，则放弃捕获*/
        if (establishGrab()) {
            qDebug() << "establishGrab : true";
        } else {
            qDebug() << "establishGrab : false";
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape),
                              True, 1);
            XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape),
                              False, 1);
            XFlush(QX11Info::display());
            sleep(1);
            if (!establishGrab()) {
                qDebug() << "establishGrab : false again!";
                // exit(1);
            }
        }
    }
    // KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events has been redirected
    connect(xEventMonitor, SIGNAL(keyPress(const QString &)), this,
            SLOT(onGlobalKeyPress(const QString &)));
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)), this,
            SLOT(onGlobalkeyRelease(const QString &)));

    xEventMonitor->start();

    //screencount changed
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::screenCountChanged, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::resized, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::workAreaResized, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::primaryScreenChanged, this, &MainWindow::screenCountChanged);
    qDebug() << "m_btnWidget FixedHeight000:" << m_btnWidget->width() << m_scrollArea->width();

    if (platformName.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        //wayland interface
    } else {
        XWindowAttributes rootAttr;
        XGetWindowAttributes(QX11Info::display(), QX11Info::appRootWindow(), &rootAttr);
        XSelectInput(QX11Info::display(), QX11Info::appRootWindow(),
                     SubstructureNotifyMask|rootAttr.your_event_mask);

        qApp->installNativeEventFilter(this);
    }
}

MainWindow::~MainWindow()
{
    delete m_power;
    delete xEventMonitor;
}


void MainWindow::initialSystemMonitor()
{
    m_systemMonitorHLayout = new QHBoxLayout();
    m_systemMonitorBtn = new QWidget(this);
    m_systemMonitorIconLabel = new QLabel();
    m_systemMonitorLabel = new QLabel(m_systemMonitorBtn);
    m_systemMonitorBtn->setObjectName("systemMonitor");
    QFont font = QApplication::font();
    QFontMetrics fm(font);

    m_systemMonitorLabel->setText(QApplication::tr("system-monitor"));
    double fsize = 10;
    if (QGSettings::isSchemaInstalled("org.ukui.style")){
        QGSettings *gset = new QGSettings("org.ukui.style");
        fsize = gset->get("system-font-size").toDouble();
    }
    fsize = fsize + 8;
    const QString font_size= "QLabel{font-size: " + QString::number(fsize) + "px}";
    m_systemMonitorLabel->setStyleSheet(font_size +
                                        "QLabel{color: white}");

    int btnWidth = fm.boundingRect(m_systemMonitorLabel->text()).width() + 60;

    qDebug() << "btnWidth:" << btnWidth << fontMetrics().width(m_systemMonitorLabel->width()) ;
    m_systemMonitorBtn->setFixedSize(btnWidth, 48);
    QString str = "QWidget#systemMonitor{background-color: transparent;border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";

    //QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,40);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
    m_systemMonitorBtn->setStyleSheet(str);
    m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);
    QRegion region0(m_systemMonitorBtn->x() + m_systemMonitorBtn->height()/2 + 1, m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->width()-m_systemMonitorBtn->height(), m_systemMonitorBtn->height() + 2);
    QRegion region1(m_systemMonitorBtn->x() - 1, m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->height() + 2, m_systemMonitorBtn->height() + 2, QRegion::Ellipse);
    QRegion region2(m_systemMonitorBtn->x() + m_systemMonitorBtn->width() - m_systemMonitorBtn->height(), m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->height() + 2, m_systemMonitorBtn->height() + 2, QRegion::Ellipse);
    QRegion region = region0 + region1 + region2;
    m_systemMonitorBtn->setMask(region);
    m_systemMonitorLabel->setAlignment(Qt::AlignCenter);
    m_systemMonitorHLayout->setAlignment(Qt::AlignCenter);

    m_systemMonitorHLayout->addWidget(m_systemMonitorLabel,fm.boundingRect(m_systemMonitorLabel->text()).width());

    m_systemMonitorBtn->setLayout(m_systemMonitorHLayout);
    m_systemMonitorBtn->installEventFilter(this);
    qDebug() << "m_systemMonitorIcon:" << m_systemMonitorIcon.width() << m_systemMonitorIcon.height() << m_systemMonitorIconLabel->width() << m_systemMonitorIconLabel->height();
    qDebug() << "m_systemMonitorBtn:" << m_systemMonitorBtn->height();
}

void MainWindow::initialBtn()
{
    m_switchUserBtn = new MyPushButton(m_btnImagesPath+"/switchuser.svg", QApplication::tr("Switch User"), "switchuser", m_scrollArea);
    m_hibernateBtn = new MyPushButton(m_btnImagesPath+"/hibernate.svg", QApplication::tr("Hibernate"), "hibernate", m_scrollArea);
    m_suspendBtn = new MyPushButton(m_btnImagesPath+"/suspend.svg", QApplication::tr("Suspend"), "suspend", m_scrollArea);
    m_logoutBtn = new MyPushButton(m_btnImagesPath+"/logout.svg", QApplication::tr("Log Out"), "logout", m_scrollArea);
    m_rebootBtn = new MyPushButton(m_btnImagesPath+"/reboot.svg", QApplication::tr("Restart"), "reboot", m_scrollArea);
    m_shutDownBtn = new MyPushButton(m_btnImagesPath+"/shutdown.svg", QApplication::tr("Shut Down"), "shutdown", m_scrollArea);
    m_lockScreenBtn = new MyPushButton(m_btnImagesPath+"/lockscreen.svg", QApplication::tr("Lock Screen"), "lockscreen", m_scrollArea);
    m_upgradeThenRebootBtn = new MyPushButton(m_btnImagesPath+"/reboot.svg", QApplication::tr("UpgradeThenRboot"), "upgradethenreboot", m_scrollArea);
    m_upgradeThenshutDownBtn = new MyPushButton(m_btnImagesPath+"/shutdown.svg", QApplication::tr("UpgradeThenShutdown"), "upgradeThenShutdown", m_scrollArea);

    //ui->setupUi(this);
    m_switchUserBtn->installEventFilter(this);
    m_hibernateBtn->installEventFilter(this);
    m_suspendBtn->installEventFilter(this);
    m_lockScreenBtn->installEventFilter(this);
    m_logoutBtn->installEventFilter(this);
    m_rebootBtn->installEventFilter(this);
    m_shutDownBtn->installEventFilter(this);
    m_upgradeThenRebootBtn->installEventFilter(this);
    m_upgradeThenshutDownBtn->installEventFilter(this);

    m_hibernateBtn->setToolTip(QApplication::tr("Turn off your computer, but the app stays open. When the computer is turned on, it can be restored to the state you left."));
    m_suspendBtn->setToolTip(QApplication::tr("The computer stays on, but consumes less power. The app stays open and can quickly wake up and revert to where you left off."));
    m_logoutBtn->setToolTip(QApplication::tr("The current user logs out of the system, terminates the session, and returns to the login page."));
    m_rebootBtn->setToolTip(QApplication::tr("Close all apps, turn off your computer, and then turn your computer back on."));
    m_shutDownBtn->setToolTip(QApplication::tr("Close all apps, and then shut down your computer."));

    connect(m_switchUserBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_hibernateBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_suspendBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_lockScreenBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_logoutBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_rebootBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_shutDownBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_upgradeThenRebootBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_upgradeThenshutDownBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
}

void MainWindow::initialJudgeWidget()
{
    QStringList userlist = LockChecker::getLoginedUsers();
    if (userlist.count() > 1) {
        close_system_needed_to_confirm = true;
    }
    QString tips = QApplication::tr("Multiple users are logged in at the same time.Are you sure "
                                    "you want to close this system?");
    m_judgeLabel->setText(tips);
    m_judgeLabel->setStyleSheet("color:white");//;font:12pt;
    m_judgeLabel->setObjectName("label");
    //m_judgeLabel->setGeometry(0,0,m_screen.width(),50);
    //m_judgeLabel->setFixedHeight(60);

    qDebug() << "m_judgeLabel width:" << m_judgeLabel->width() << m_judgeLabel->height();
    m_judgeLabel->setAlignment(Qt::AlignHCenter);
    m_judgeLabel->setWordWrap(true);

    m_cancelBtn = new CommonPushButton(QApplication::tr("cancel"), QString::fromUtf8("cancelButton"), 120, 48, 8);
    m_confirmBtn = new CommonPushButton(QApplication::tr("confirm"), QString::fromUtf8("confirmButton"), 120, 48, 8);

    m_judgeBtnHLayout->setAlignment(Qt::AlignHCenter);
    m_judgeBtnHLayout->setSpacing(0);
    m_judgeBtnHLayout->addWidget(m_cancelBtn);
    m_judgeBtnHLayout->addSpacing(24);
    m_judgeBtnHLayout->addWidget(m_confirmBtn);

    m_judgeWidgetVLayout->addSpacing(10);
    m_judgeWidgetVLayout->addWidget(m_judgeLabel);
    m_judgeWidgetVLayout->addSpacing(10);
    m_judgeWidgetVLayout->addLayout(m_judgeBtnHLayout);
    m_judgeWidgetVLayout->setAlignment(Qt::AlignCenter);

    m_judgeWidgetVLayout->setContentsMargins(0,0,0,0);

    connect(m_cancelBtn, &CommonPushButton::clicked, this, &MainWindow::exitt);
    connect(m_confirmBtn, &CommonPushButton::clicked, [&]() { emit confirmButtonclicked(); });
    m_judgeWidget->setLayout(m_judgeWidgetVLayout);
    m_judgeWidget->setStyleSheet("QWidget#judgeWidget{background-color: transparent;}");
    m_judgeWidget->setVisible(false);
    m_judgeWidget->setGeometry(m_screen.x() + (m_screen.width() - 800 * m_screen.width()/1920)/2,
                                     m_screen.y() + (m_screen.height() - 200)/2 - 10,
                               800 * m_screen.width()/1920, 200);
}

void MainWindow::initialDateTimeWidget()
{
    char *user = qgetenv("USER").data();
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(user);
    QString  curTime     = QString(dateInfo->time);
    char   *curDateLong = kdk_system_get_longformat_date();
    char   *curWeekLong = kdk_system_longweek();
    QString currentDate = QString(curDateLong) + " " + QString(curWeekLong);

    qDebug() << "curDateLong is " << curDateLong << " and curWeekLong is " << curWeekLong << ",curTime=" << curTime;
    delete curDateLong;
    delete curWeekLong;
    delete dateInfo;

//    QDateTime        current_date_time = QDateTime::currentDateTime();
//    const QByteArray id_control("org.ukui.control-center.panel.plugins");

//    QString          current_time;
//    if (QGSettings::isSchemaInstalled(id_control)) {
//        QGSettings *controlSetting = new QGSettings(id_control, QByteArray(), this);
//        QString     formate_b      = controlSetting->get("hoursystem").toString();

//        if (formate_b == "12") {
//            current_time = current_date_time.toString("A hh:mm");
//        } else if (formate_b == "24") {
//            current_time = current_date_time.toString("hh:mm");
//        } else {
//            current_time = current_date_time.toString("hh:mm");
//        }
//    } else {
//        current_time = current_date_time.toString("hh:mm");
//    }

    m_timeLabel->setText(curTime);
    m_dateLabel->setText(currentDate);

//    m_timeLabel->setFont(QFont("Noto Sans CJK SC", 28, 50));
//    m_dateLabel->setFont(QFont("Noto Sans CJK SC", 12, 50));
    m_timeLabel->setStyleSheet("color: white; font: 28pt");
    m_dateLabel->setStyleSheet("color: white; font: 12pt");
    m_dateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_dateLabel->setObjectName("date_label");
    m_timeLabel->setObjectName("time_lable");

    m_dateTimeLayout->addStretch();
    m_dateTimeLayout->addWidget(m_timeLabel);
//    m_messageVLayout->addSpacing(10);
    //m_dateTimeLayout->setStretch(0,1);
    m_dateTimeLayout->addWidget(m_dateLabel);
    //m_dateTimeLayout->setStretch(1,2);
}

void MainWindow::updateDateTimerWidget()
{
    char *user = qgetenv("USER").data();
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(user);
    QString  curTime     = QString(dateInfo->time);
    char   *curDateLong = kdk_system_get_longformat_date();
    char   *curWeekLong = kdk_system_longweek();
    QString currentDate = QString(curDateLong) + " " + QString(curWeekLong);

    qDebug() << "curDateLong is " << curDateLong << " and curWeekLong is " << curWeekLong<< ",curTime=" << curTime;
    delete curDateLong;
    delete curWeekLong;
    delete dateInfo;

//    QDateTime        current_date_time = QDateTime::currentDateTime();
//    const QByteArray id_control("org.ukui.control-center.panel.plugins");

//    QString          current_time;
//    if (QGSettings::isSchemaInstalled(id_control)) {
//        QGSettings *controlSetting = new QGSettings(id_control, QByteArray(), this);
//        QString     formate_b      = controlSetting->get("hoursystem").toString();

//        if (formate_b == "12") {
//            current_time = current_date_time.toString("A hh:mm");
//        } else if (formate_b == "24") {
//            current_time = current_date_time.toString("hh:mm");
//        } else {
//            current_time = current_date_time.toString("hh:mm");
//        }
//    } else {
//        current_time = current_date_time.toString("hh:mm");
//    }

    m_timeLabel->setText(curTime);
    m_dateLabel->setText(currentDate);
}

void MainWindow::initialBtnCfg()
{
    bool newIniFile = false;//ini文件是否为新建文件
    QString iniDir = "/usr/share/ukui/ukui-session-manager/config";
    if(!QFile::exists(iniDir + "/btnconfig.ini")){
        qDebug() << "btnconfig.ini file is not exists!!!";

        QDir dir(iniDir);
        if(!dir.exists(iniDir)){
            if(dir.mkdir(iniDir)){//目前创建不成功  没有权限
                QFile iniFile(iniDir + "/btnconfig.ini");
                if(iniFile.open(QIODevice::WriteOnly)){
                    newIniFile = true;
                    iniFile.close();
                }
                qDebug() << "inifile open faile!";
            }
            else
                qDebug() << "create inidir faile!";
        }
    }
    m_btnCfgSetting = new QSettings("/usr/share/ukui/ukui-session-manager/config/btnconfig.ini", QSettings::IniFormat);

    if (newIniFile) {//貌似路径下文件只可读不可写
        m_btnCfgSetting->setValue("btn/SwitchUserBtnHide", false);
        m_btnCfgSetting->setValue("btn/HibernateBtnHide", false);
        m_btnCfgSetting->setValue("btn/LockScreenBtnHide", false);
        m_btnCfgSetting->setValue("btn/LogoutBtnHide", false);
        m_btnCfgSetting->setValue("btn/RebootBtnHide", false);
        m_btnCfgSetting->setValue("btn/ShutDownBtnHide", false);
        m_btnCfgSetting->setValue("btn/SuspendBtnHide", false);
    }

    m_btnCfgSetting->setValue("btn/SwitchUserBtnHide", isSwitchuserHide);
    m_btnCfgSetting->setValue("btn/HibernateBtnHide", isHibernateHide);
    m_btnCfgSetting->setValue("btn/SuspendBtnHide", isSuspendHide);
    m_btnCfgSetting->setValue("btn/LogoutBtnHide", isLogoutHide);
    m_btnCfgSetting->setValue("btn/RebootBtnHide", isRebootHide);
    m_btnCfgSetting->setValue("btn/ShutDownBtnHide", isPowerOffHide);

    qDebug() << "isHibernateHide..." << isHibernateHide;
    m_btnHideMap.insert(m_switchUserBtn, isSwitchuserHide);
    m_btnHideMap.insert(m_hibernateBtn, isHibernateHide);
    m_btnHideMap.insert(m_suspendBtn, isSuspendHide);
    m_btnHideMap.insert(m_lockScreenBtn, m_btnCfgSetting->value("btn/LockScreenBtnHide").toBool());
    m_btnHideMap.insert(m_logoutBtn, isLogoutHide);
    m_btnHideMap.insert(m_rebootBtn, isRebootHide);
    m_btnHideMap.insert(m_shutDownBtn, isPowerOffHide);
}

void MainWindow::setLayoutWidgetVisible(QLayout* layout, bool show)
{
    for (int i = 0;i < layout->count(); i++) {
        QLayoutItem*item = layout->layout()->itemAt(i);
        if (item->widget() != nullptr) {
            item->widget()->setVisible(show);
        }
    }
}

void MainWindow::changeBtnState(QString btnName, bool isKeySelect)
{
    for(auto item = map.begin(); item != map.end(); item++)
    {
        if (btnName == QString("empty")) {
            item.value()->changeIconBackColor((item.value()->objectName() == btnName), isKeySelect);
        }

        if (isKeySelect) {
            item.value()->changeIconBackColor((item.value()->objectName() == btnName), isKeySelect);
        } else {
            if (item.value()->objectName() == btnName) {
                item.value()->changeIconBackColor((item.value()->objectName() == btnName), isKeySelect);
            }

            if (lastWidget && lastWidget->objectName() == item.value()->objectName() && lastWidget->objectName() != btnName) {
                item.value()->changeIconBackColor(false, isKeySelect);
            }
        }
    }
}

void MainWindow::mouseReleaseSlots(QEvent *event, QString objName)
{
    for (auto iter = map.begin(); iter != map.end(); iter++) {
        if (iter.value()->getIconLabel()->objectName() == objName) {
            changePoint(iter.value(), event);
            if (event->type() == QEvent::MouseButtonRelease) {
                qDebug() << "mouseReleaseSlots..." << objName;
                doEvent(objName, iter.key());
                return;
            }
        }
    }
}

void MainWindow::screenCountChanged()
{
    QDesktopWidget *desktop = QApplication::desktop();
    qDebug() << "inside screenCountChanged,screenCount = " << desktop->screenCount() << QApplication::desktop()->screenGeometry(QCursor::pos()).width() << QApplication::desktop()->screenGeometry(QCursor::pos()).height();
    //setGeometry(desktop->geometry());
    //updateGeometry();
    //move(0,0);
    //setFixedSize(QApplication::primaryScreen()->virtualSize());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());
    ResizeEvent();
    update();
}

void MainWindow::calculateBtnSpan(int allNum, int lineMaxNum, MyPushButton *btn, int &row, int &colum)
{
    int afterHideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (map.key(btn) > i) {
            if (m_btnHideMap.value(map.value(i))) {
                afterHideNum++;
            }
        } else if(map.key(btn) == i) {
            colum = (i - afterHideNum) % lineMaxNum;
            row = (i - afterHideNum) / lineMaxNum;
            return;
        }
    }
}

void MainWindow::showNormalBtnWidget(int hideNum)
{
    int margins = 0;

    if (hideNum >= 8) {
        margins = (m_screen.width() -160 - 128 * (9 - hideNum));
    } else {
        margins = (m_screen.width() -160 - 128 * (9 - hideNum)) / (8-hideNum);
    }

    qDebug() << "showNormalBtnWidget hideNum:" << hideNum << "margins:" << margins;
    m_lineNum = 1;
    int btnWidgetWidth = 0;
    if (margins > 60) {
        //m_buttonHLayout->addWidget(m_listView);
        if (hideNum == 9) {
            btnWidgetWidth = m_screen.width();
        } else {
            btnWidgetWidth = (128 * (9 - hideNum) + 60 * (8 - hideNum));
        }

        m_buttonHLayout->setHorizontalSpacing(60);
    } else {
        btnWidgetWidth = 128 * (9 - hideNum) + margins * (8 - hideNum);
        m_buttonHLayout->setHorizontalSpacing(margins);
    }
    m_btnWidget->setGeometry(QRect(m_btnWidget->x(),m_btnWidget->y(),btnWidgetWidth+ 24, 632 * m_screen.height()/1080));
    m_btnWidget->setContentsMargins(0,0,0,100 * m_screen.height()/1080);

    m_scrollArea->setGeometry(QRect(m_scrollArea->x(),m_scrollArea->y(),btnWidgetWidth + 24, 632 * m_screen.height()/1080));

    m_scrollArea->setContentsMargins(0,0,0,0);
    m_scrollArea->verticalScrollBar()->setVisible(false);
    m_scrollArea->verticalScrollBar()->setDisabled(true);
    m_scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar{ background: transparent; margin-top:0px;margin-bottom:0px ; }"\
                                                     "QScrollBar:vertical{width: 0px;background: transparent;border-radius:3px;}"\
                                                     "QScrollBar::handle:vertical{width: 0px; background: rgba(255,255,255, 40); border-radius:3px;}"\
                                                     "QScrollBar::handle:vertical:hover{width: 0px; background: rgba(255,255,255, 60); border-radius:3px;}"\
                                                     "QScrollBar::add-line:vertical{width:0px;height:0px}"\
                                                     "QScrollBar::sub-line:vertical{width:0px;height:0px}");

    //m_buttonHLayout->setContentsMargins(0,0,0,(m_scrollArea->height() - m_switchUserBtn->height()) * 3/5);

    for (int i = 0;i < m_buttonHLayout->count(); i++) {
        QLayoutItem*item = m_buttonHLayout->layout()->itemAt(i);
        if (item->widget() != nullptr) {
            m_buttonHLayout->removeWidget(item->widget());
        }
    }

    m_buttonHLayout->addWidget(m_switchUserBtn,0,0);
    m_buttonHLayout->addWidget(m_hibernateBtn,0,1);
    m_buttonHLayout->addWidget(m_suspendBtn,0,2);
    m_buttonHLayout->addWidget(m_lockScreenBtn,0,3);
    m_buttonHLayout->addWidget(m_logoutBtn,0,4);
    m_buttonHLayout->addWidget(m_upgradeThenRebootBtn,0,5);
    m_buttonHLayout->addWidget(m_rebootBtn,0,6);
    m_buttonHLayout->addWidget(m_upgradeThenshutDownBtn,0,7);
    m_buttonHLayout->addWidget(m_shutDownBtn,0,8);
    m_btnWidgetNeedScrollbar = false;
}

void MainWindow::showHasScrollBarBtnWidget(int hideNum)
{
    int allBtnNum = 9 - hideNum;
    int lineWidth = m_screen.width() - 160 - 6;
    int lineMaxBtnNum = (lineWidth - 128)/188 + 1;
    m_lineNum = allBtnNum/lineMaxBtnNum + ((allBtnNum%lineMaxBtnNum > 0) ? 1 : 0);
    int needHeight = (m_lineNum * 171 + (m_lineNum - 1) * 32);
    int btnWidgetHeight = 632;

    qDebug() << "showHasScrollBarBtnWidget lineWidth:" << lineWidth << "lineMaxBtnNum:" << lineMaxBtnNum << "needHeight:" << needHeight << "lineNum:" << m_lineNum<< allBtnNum/lineMaxBtnNum << allBtnNum%lineMaxBtnNum;

    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_switchUserBtn, m_switchRow, m_switchColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_hibernateBtn, m_hibernateRow, m_hibernateColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_suspendBtn, m_suspendRow, m_suspendColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_lockScreenBtn, m_lockScreenRow, m_lockScreenColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_logoutBtn, m_logoutRow, m_logoutColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_upgradeThenRebootBtn, m_upgradeThenRebootRow, m_upgradeThenRebootColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_rebootBtn, m_rebootRow, m_rebootColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_upgradeThenshutDownBtn, m_upgradeThenShutdownRow, m_upgradeThenShutdownColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_shutDownBtn, m_shutDownRow, m_shutDownColumn);

//        qDebug() << "switchRow:" << m_switchRow << m_switchColumn;
//        qDebug() << "hibernateRow:" << m_hibernateRow << m_hibernateColumn;
//        qDebug() << "suspendRow:" << m_suspendRow << m_suspendColumn;
//        qDebug() << "lockScreenRow:" << m_lockScreenRow << m_lockScreenColumn;
//        qDebug() << "logoutRow:" << m_logoutRow << m_logoutColumn;
//        qDebug() << "rebootRow:" << m_rebootRow << m_rebootColumn;
//        qDebug() << "shutDownRow:" << m_shutDownRow << m_shutDownColumn;

    {
        m_scrollArea->verticalScrollBar()->setVisible(!m_judgeLabel->isVisible());
        m_scrollArea->verticalScrollBar()->setDisabled(false);
        m_scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar{ background: transparent; margin-top:0px;margin-bottom:0px ; }"\
                                                         "QScrollBar:vertical{width: 6px;background: transparent;border-radius:3px;}"\
                                                         "QScrollBar::handle:vertical{width: 6px; background: rgba(255,255,255, 40); border-radius:3px;}"\
                                                         "QScrollBar::handle:vertical:hover{width: 6px; background: rgba(255,255,255, 60); border-radius:3px;}"\
                                                         "QScrollBar::add-line:vertical{width:0px;height:0px}"\
                                                         "QScrollBar::sub-line:vertical{width:0px;height:0px}");

        qDebug() << "set bar pos...";
        //m_scrollArea->verticalScrollBar()->setGeometry(QRect(lineWidth + 20, 0, 6, 100));
        m_scrollArea->setContentsMargins(0,0,0,0);
        m_scrollArea->setGeometry(QRect(0,0,128 * lineMaxBtnNum + 60 * (lineMaxBtnNum-1) + 6, btnWidgetHeight * m_screen.height()/1080));

        m_buttonHLayout->setContentsMargins(0,0,0,0);// (needHeight > 632 ? 0 : (632 - needHeight)) * m_screen.height()/1080);

        m_btnWidget->setGeometry(QRect(0,0,128 * lineMaxBtnNum + 60 * (lineMaxBtnNum-1), needHeight));
        m_btnWidget->setContentsMargins(6,0,12,0);
        qDebug() << "m_btnWidget FixedHeight:" << needHeight << btnWidgetHeight << m_btnWidget->width() << m_scrollArea->width();
        qDebug() << "isSwitchuserHide:" << isSwitchuserHide << "isHibernateHide:" << isHibernateHide << "isSuspendHide:" << isSuspendHide;

        for (int i = 0;i < m_buttonHLayout->count(); i++) {
            QLayoutItem*item = m_buttonHLayout->layout()->itemAt(i);
            if (item->widget() != nullptr) {
                m_buttonHLayout->removeWidget(item->widget());
            }
        }
        m_buttonHLayout->addWidget(m_switchUserBtn, m_switchRow, m_switchColumn);
        m_buttonHLayout->addWidget(m_hibernateBtn, m_hibernateRow, m_hibernateColumn);
        m_buttonHLayout->addWidget(m_suspendBtn, m_suspendRow, m_suspendColumn);
        m_buttonHLayout->addWidget(m_lockScreenBtn, m_lockScreenRow, m_lockScreenColumn);
        m_buttonHLayout->addWidget(m_logoutBtn, m_logoutRow, m_logoutColumn);
        m_buttonHLayout->addWidget(m_upgradeThenRebootBtn, m_upgradeThenRebootRow, m_upgradeThenRebootColumn);
        m_buttonHLayout->addWidget(m_rebootBtn, m_rebootRow, m_rebootColumn);
        m_buttonHLayout->addWidget(m_upgradeThenshutDownBtn, m_upgradeThenShutdownRow, m_upgradeThenShutdownColumn);
        m_buttonHLayout->addWidget(m_shutDownBtn, m_shutDownRow, m_shutDownColumn);
        m_buttonHLayout->setHorizontalSpacing(60);
        m_buttonHLayout->setAlignment(Qt::AlignHCenter);
        m_btnWidgetNeedScrollbar = true;
    }
}

void MainWindow::ResizeEvent()
{
    m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());

    //处理界面停留在阻止提醒界面时拔插屏幕出现的问题
    if (m_showWarningMesg) {
        showInhibitWarning(m_inhibitList, defaultnum);
        m_toolWidget->setGeometry(m_screen);
        return;
    }

    m_judgeWidget->setGeometry(m_screen.x() + (m_screen.width() - 800 * m_screen.width()/1920)/2,
                               m_screen.y() + (m_screen.height() - 200)/2 - 10,800 * m_screen.width()/1920, 200);


    if(m_judgeWidget->isVisible())
        return;
    qDebug() << "ResizeEvent moveWidget  m_screen:" << m_screen.width() << m_screen.height();
    int hideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (m_btnHideMap.value(map.value(i))) {
            hideNum++;
        }
    }
    qDebug()<<"m_btnHideMap.count()=" << m_btnHideMap.count() <<",hideNum=" << hideNum;

    // Move the widget to the direction where they should be
    for (int i = 0; i <= 8; i++) {
        if (m_btnHideMap.value(map.value(i)))
        {
            map[i]->hide();
        }
        else
        {
            map[i]->show();
        }
    }
    if (hideNum >= 8 || (m_screen.width() -160 - 128 * (9 - hideNum)) / (8-hideNum) >= 16) {
        showNormalBtnWidget(hideNum);
    } else {
        showHasScrollBarBtnWidget(hideNum);
    }
    m_btnWidget->setStyleSheet("QWidget#btnWidget{background-color: transparent;}");
    m_btnWidget->setLayout(m_buttonHLayout);

    m_scrollArea->horizontalScrollBar()->setVisible(false);
    m_scrollArea->horizontalScrollBar()->setDisabled(true);
    m_scrollArea->setWidget(m_btnWidget);
    m_scrollArea->setStyleSheet("QScrollArea#scrollArea{background-color: transparent;}");
    m_scrollArea->setAlignment(Qt::AlignCenter);
    qDebug() << "m_scrollArea geometry:" << m_scrollArea->geometry();
    qDebug() << "m_btnWidget geometry:" << m_btnWidget->geometry();
    qDebug() << "m_buttonHLayout geometry:" << m_buttonHLayout->geometry();

    m_vBoxLayout->setContentsMargins((m_screen.width() - m_scrollArea->width() - 20)/2,0,(m_screen.width() - m_scrollArea->width() - 20)/2,0);

    //m_scrollArea->adjustSize();
    //m_scrollArea->setWidgetResizable(true);
    //行
    rowMap.clear();
    rowMap.insert(0, m_switchRow);
    rowMap.insert(1, m_hibernateRow);
    rowMap.insert(2, m_suspendRow);
    rowMap.insert(3, m_lockScreenRow);
    rowMap.insert(4, m_logoutRow);
    rowMap.insert(5,m_upgradeThenRebootRow);
    rowMap.insert(6, m_rebootRow);
    rowMap.insert(7, m_upgradeThenShutdownRow);
    rowMap.insert(8, m_shutDownRow);

    //列
    columMap.clear();
    columMap.insert(0, m_switchColumn);
    columMap.insert(1, m_hibernateColumn);
    columMap.insert(2, m_suspendColumn);
    columMap.insert(3, m_lockScreenColumn);
    columMap.insert(4, m_logoutColumn);
    columMap.insert(5, m_upgradeThenRebootColumn);
    columMap.insert(6, m_rebootColumn);
    columMap.insert(7, m_upgradeThenShutdownColumn);
    columMap.insert(8, m_shutDownColumn);

    m_toolWidget->setGeometry(m_screen);
}

// Paint the background picture
void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0, 0, 0, 78));   // 178
    for (QScreen *screen: QApplication::screens()) {
        // draw picture to every screen
        QRect rect = screen->geometry();
        painter.drawPixmap(rect, pix.scaled(screen->size()));
        //drawRect可以避免在白色壁纸的情况下模糊背景造成的问题
        painter.drawRect(rect);
    }
    QWidget::paintEvent(e);
}

//lock screen
void MainWindow::doLockscreen()
{
    QString SS_DBUS_SERVICE = "org.ukui.ScreenSaver";
    QString SS_DBUS_PATH = "/";
    QString SS_DBUS_INTERFACE = "org.ukui.ScreenSaver";
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    qDebug() << "dispaly = " << displayNum ;
    QString sessionDbus = QString("%1%2").arg(SS_DBUS_SERVICE).arg(displayNum);
    QDBusInterface *interface = new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE);

    if (!interface->isValid()) {
        qDebug() << "interface not valid";
        delete interface;
        interface = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    }

    /*监听锁屏起来的信号，再执行界面退出操作，规避点击锁屏后先显示桌面再打开锁屏
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("lock"), this, SLOT(exitt()));*/
    QDBusMessage msg = interface->call("Lock");
    //延迟界面退出操作，规避点击锁屏后先显示桌面再打开锁屏
//    QTimer::singleShot(500, this, SLOT(exitt()));
    exit(0);
}

void MainWindow::startUkuiSplash()
{
    QProcess *splash = new QProcess(this);
    qDebug() << "start ukui-ksplash";
    splash->startDetached("ukui-ksplash");
}

void MainWindow::doSystemMonitor()
{
    qDebug() << "doSystemMonitor....";
    QProcess::startDetached("ukui-system-monitor", QStringList());
    exitt();
}

// handle mouse-clicked event
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "systemMonitor") {
           if (event->type() == QEvent::MouseButtonRelease) {
               doSystemMonitor();
           } else {
               if (event->type() == QEvent::Leave) {
                   QString str = "QWidget#systemMonitor{background-color: transparent;border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

                   //tableNum   = -1;
                   flag = false;
                   changeBtnState("empty");
               } else if (event->type() == QEvent::Enter) {
                   QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,40);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

                   //tableNum   = -1;
                   flag = true;
                   changeBtnState("empty");
               }  else if (event->type() == QEvent::MouseButtonPress) {
                   QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,80);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);
               }
           }
    }


    if(event->type() == QEvent::WindowActivate){
        if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
            PlasmaShellManager::getInstance()->setAppWindowKeepAbove(true);
        }
        //QTimer::singleShot(20,this,&MainWindow::laterActivate);
    }

    return QWidget::eventFilter(obj, event);
}

void MainWindow::changePoint(QWidget *widget, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        changeBtnState(widget->objectName());
    }
    if (event->type() == QEvent::Leave) {
        flag = false;
        lastWidget = widget;
    }
}

void MainWindow::doEvent(QString test, int i)
{
    qDebug() << "doevent... i:" << i << test << inhibitShutdown << inhibitSleep << close_system_needed_to_confirm;
    defaultnum = i;

    gs->set("win-key-release", false);
    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        //wayland interface
    } else {
        if (click_blank_space_need_to_exit) {
            if (closeGrab()) {
                qDebug() << "success to close Grab";
            } else {
                qDebug() << "failure to close Grab";
            }
        }
    }


    switch (i) {
    case (1) :
    case (2) :
    {
        //获取inhibitor，不为空展示界面，为空直接执行动作
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("sleep");
        if (res.isEmpty()) {
            qDebug() << "Start do action" << test << defaultnum;

            this->hide();
            startUkuiSplash();
            emit signalTostart();
        } else {
            showInhibitWarning(res, i);
        }
        break;
    }
    case (3) : {
        doLockscreen();
        break;
    }
    case (4) : {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            qDebug() << "Start do action" << test << defaultnum;

            this->hide();
            emit signalTostart();
        } else {
            showInhibitWarning(res, i);
        }
        break;
    }
    case (5) : {
        qDebug() << "Start do action " << test << defaultnum;

        this->hide();
        upgradeCheck->doUpgradeThenRboot();
        exit(0);
        break;
    }
    case (6) : {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            if (close_system_needed_to_confirm) {
                connect(this, &MainWindow::confirmButtonclicked, [&](){
                    qDebug() << "Start do action" << defaultnum;
                    this->hide();
                    emit signalTostart();
                });
                this->judgeboxShow();
            } else {
                qDebug() << "Start do action" << defaultnum;
                this->hide();
                emit signalTostart();
            }
        } else {
            showInhibitWarning(res, i);
        }
        break;
    }
    case (7) : {
        qDebug() << "Start do action" << test << defaultnum;

        this->hide();
        upgradeCheck->doUpgradeThenShutdown();
        exit(0);
        break;
    }
    case (8) : {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            if (close_system_needed_to_confirm) {
                connect(this, &MainWindow::confirmButtonclicked, [&](){
                    qDebug() << "Start do action" << defaultnum;
                    this->hide();
                    emit signalTostart();
                });
                this->judgeboxShow();
            } else {
                qDebug() << "Start do action" << defaultnum;
                this->hide();
                emit signalTostart();
            }
        } else {
            showInhibitWarning(res, i);
        }
        break;
    }
    default : {

        this->hide();
        emit signalTostart();
        break;
    }
    }
}

// handle the blank-area mousePressEvent
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (click_blank_space_need_to_exit) {
        QPainterPath path;
        QRect rect(m_systemMonitorBtn->geometry());
        const qreal radius = m_systemMonitorBtn->height()/2;
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

        qDebug() << "mousePressEvent" << m_switchUserBtn->geometry() << m_scrollArea->mapFromGlobal(event->pos());

        if (!m_suspendBtn->getIconLabel()->containsPoint(m_suspendBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_hibernateBtn->getIconLabel()->containsPoint(m_hibernateBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_lockScreenBtn->getIconLabel()->containsPoint(m_lockScreenBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_switchUserBtn->getIconLabel()->containsPoint(m_switchUserBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_logoutBtn->getIconLabel()->containsPoint(m_logoutBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_upgradeThenRebootBtn->getIconLabel()->containsPoint(m_upgradeThenRebootBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_rebootBtn->getIconLabel()->containsPoint(m_rebootBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_upgradeThenshutDownBtn->getIconLabel()->containsPoint(m_upgradeThenshutDownBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_shutDownBtn->getIconLabel()->containsPoint(m_shutDownBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !path.contains(m_toolWidget->mapFromGlobal(event->pos()))) {
            exitt();
        }

    }
}

bool MainWindow::exitt()
{
    gs->set("win-key-release", false);
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "xcb") {
        if (closeGrab()) {
            qDebug() << "success to close Grab";
        } else {
            qDebug() << "failure to close Grab";
        }
    }
    close();
    exit(0);
}

bool MainWindow::judgeBtnIsEnable(int index)
{
    if (!m_btnHideMap.value(map.value(index))) {
        return true;
    }
    return false;
}

bool MainWindow::matchKeyBtn(int row, int colum)
{
    for (int j = 8; j >= 0; j--) {
        if (rowMap.value(j) == row && columMap.value(j) == colum && judgeBtnIsEnable(j)) {
            tableNum = j;
            return true;
        }
    }
    return false;
}

void MainWindow::calculateKeyBtn(const QString &key)
{
    if (key == "Left") {
        if (tableNum == 0 || tableNum == -1) {
            for (int i = 8; i >= 0; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        } else {
            for (int i = tableNum - 1; i >= 0; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
            for (int i = 8; i >= tableNum + 1; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        }
    } else if (key == "Right") {
        if (tableNum == 8 || tableNum == -1) {
            for (int i = 0; i <= 8; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        } else {
            for (int i = tableNum + 1; i <= 8; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
            for (int i = 0; i <= tableNum - 1; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        }
    } else if (m_lineNum > 1 && key == "Up") {
        if (tableNum == -1) {
            for (int i = m_lineNum - 1; i >= 0; i--) {
                if(matchKeyBtn(i, 0))
                    return;
            }
        }

        int tableRow = rowMap.value(tableNum);
        int tableColum = columMap.value(tableNum);
        for (int i = 1; i < tableRow + 1; i++) {
            if (matchKeyBtn(tableRow - i, tableColum))
                return;
        }
        for (int i = m_lineNum - 1; i > tableRow; i--) {
            if(matchKeyBtn(i, tableColum))
                return;
        }
    } else if (m_lineNum > 1 && key == "Down") {
        if (tableNum == -1) {
            for (int i = 0; i < m_lineNum; i++) {
                if (matchKeyBtn(i, 0))
                    return;
            }
        }

        int tableRow = rowMap.value(tableNum);
        int tableColum = columMap.value(tableNum);
        for (int i = tableRow + 1; i < m_lineNum; i++) {
            if (matchKeyBtn(i, tableColum))
                return;
        }
        for (int i = 0; i < tableRow + 1; i++) {
            if (matchKeyBtn(i, tableColum))
                return;
        }
    }
}

void MainWindow::onGlobalKeyPress(const QString &key)
{
}

// handle "Esc","Left","Right","Enter" keyPress event
void MainWindow::onGlobalkeyRelease(const QString &key)
{
    if(QString(qgetenv("XDG_SESSION_TYPE")) != "wayland") {
        return;
    }
    qDebug() << "key: " << key << "flag:" << flag << "click_blank_space_need_to_exit:" << click_blank_space_need_to_exit;

    if (!click_blank_space_need_to_exit && m_enterJudgeBox) {
        return;
    } /*else if (!click_blank_space_need_to_exit && m_enterInhibitList) {
        //处理阻止列表界面的键盘逻辑
        int totalRow = m_inhibitDataModel->rowCount();
        if (key == "Escape") {
            exitt();
        } else if (key == "Tab") {
            if (m_isFirstEnterInhibitList) {
                m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(0, 0));
                m_currentListSelect = 0;
                m_isFirstEnterInhibitList = false;
            } else {
                if (!m_isCancelBtnSelect) {
                    QString str = "QWidget#cancelBtn{background-color: rgb(255,255,255,80);border: 1px solid #296CD9; border-radius: " + QString::number(m_inhibitCancelBtn->height()/2) + "px;}";
                    m_inhibitCancelBtn->setStyleSheet(str);
                    m_inhibitCancelBtn->setAttribute(Qt::WA_StyledBackground);
                    m_inhibitAppList->clearSelection();
                    m_isCancelBtnSelect = true;
                } else {
                    QString str = "QWidget#cancelBtn{background-color: rgb(255,255,255,40);border-radius: " + QString::number(m_inhibitCancelBtn->height()/2) + "px;}";
                    m_inhibitCancelBtn->setStyleSheet(str);
                    m_inhibitCancelBtn->setAttribute(Qt::WA_StyledBackground);

                    m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(0, 0));
                    m_currentListSelect = 0;
                    m_isCancelBtnSelect = false;
                }
            }
        } else if (key == "Down") {
            if (!m_isCancelBtnSelect) {
                m_isFirstEnterInhibitList = false;
                if (m_currentListSelect < totalRow - 1) {
                    m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(++m_currentListSelect, 0));
                } else if (m_currentListSelect == totalRow - 1) {
                    m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(0, 0));
                    m_currentListSelect = 0;
                }
            }
        } else if (key == "Up") {
            if (!m_isCancelBtnSelect) {
                m_isFirstEnterInhibitList = false;
                if (m_currentListSelect > 0) {
                    m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(--m_currentListSelect, 0));
                } else if (m_currentListSelect <= 0) {
                    m_inhibitAppList->setCurrentIndex(m_inhibitDataModel->index(totalRow - 1, 0));
                    m_currentListSelect = totalRow - 1;
                }
            }
        } else if (key == "Return" || key == "KP_Enter") {
            if (m_isCancelBtnSelect) {
                exitt();
            }
        }
    }*/ else {
        if (key == "Escape") {
            exitt();
        } else if (key == "Left" || key == "Right" || key == "Up" || key == "Down") {
            if (!m_isSystemMonitorSelect) {
                m_isFirstEnter = false;
                int oldNum = tableNum;
                calculateKeyBtn(key);
                qDebug() << "key...." << oldNum << tableNum;
                if (oldNum == tableNum)
                    return;
                QString button = map[tableNum]->objectName();

                if (m_btnWidgetNeedScrollbar) {
                    int currentLine = rowMap[tableNum];
                    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
                    if (currentLine == 0) {
                        scrollBar->setValue(0);
                    } else {
                        int sValue = scrollBar->maximum() - scrollBar->minimum();
                        float scale = (currentLine * 180 + currentLine * m_buttonHLayout->verticalSpacing() + 128) * 1.0 / (m_btnWidget->height() * 1.0);
                        scrollBar->setValue(sValue * scale);
                    }
                }
                changeBtnState(button, true);
            }
        } else if (key == "Return" || key == "KP_Enter") {   // space,KP_Enter
            if (m_isSystemMonitorSelect) {
                doSystemMonitor();
            } else {
                for (auto item = map.begin(); item != map.end(); item++) {
                    if (item.value()->getIsKeySelect())
                        return doEvent(item.value()->objectName(), item.key());
                }

                for (auto item = map.begin(); item != map.end(); item++) {
                    if (item.value()->getIsMouseSelect()) {
                        return doEvent(item.value()->objectName(), item.key());
                    }
                }
            }
        } /*else if (key == "Tab") {
            if (m_isFirstEnter) {
                tableNum = -1;
                calculateKeyBtn("Right");
                QString button = map[tableNum]->objectName();
                changeBtnState(button, true);
                m_isFirstEnter = false;
            } else {
                if (!m_isSystemMonitorSelect) {
                    QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,80);border: 1px solid #296CD9; border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                    m_systemMonitorBtn->setStyleSheet(str);
                    m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

                    flag = true;

                    changeBtnState("empty", true);
                    m_isSystemMonitorSelect = true;
                } else {
                    tableNum = -1;
                    calculateKeyBtn("Right");
                    QString button = map[tableNum]->objectName();
                    changeBtnState(button, true);

                    QString str = "QWidget#systemMonitor{background-color: transparent;border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                    m_systemMonitorBtn->setStyleSheet(str);
                    m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

                    flag = false;
                    m_isSystemMonitorSelect = false;
                }
            }
        }*/
    }
}

void MainWindow::showInhibitWarning(QVector<InhibitInfo::InhibitorInfo> &list, int action)
{
    QRect mainScreen;
    QList<QScreen*> screens = QApplication::screens();
    QPoint ptf(QCursor::pos());
    for (QScreen *screen : screens) {
        QRect rec = screen->geometry();
        if (rec.contains(ptf)) {
            mainScreen = rec;//获取鼠标所在屏幕
        }
    }

    //进入应用阻止列表后点击一些事件不会再被响应
    click_blank_space_need_to_exit = false;
    m_enterInhibitList = true;
    for (int j = 0; j < 9; j++) {
        map[j]->hide();//隐藏界面上原有的部件
    }
    m_systemMonitorBtn->hide();
    m_scrollArea->verticalScrollBar()->setVisible(false);
    drawWarningWindow(mainScreen, list, action);
}

void MainWindow::drawWarningWindow(QRect &rect, QVector<InhibitInfo::InhibitorInfo> &list, int action)
{
    int xx = rect.x();
    int yy = rect.y();//用于设置相对位置

    bool isEnoughBig = m_screen.height() - 266 - 467 > 0 ? true : false;

    m_showWarningArea->setGeometry(0, 0, isEnoughBig ? 740 : 1200 * m_screen.width()/1920, isEnoughBig ? 467 : (500 * m_screen.height()/1080));
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    //顶部提醒信息
    QLabel *tips = new QLabel(m_showWarningArea);
    tips->setObjectName(QString::fromUtf8("tips"));
    tips->setGeometry(0, 0, isEnoughBig ? 740 : m_showWarningArea->width(), 27);
    tips->setWordWrap(true);

    QString str;
    //defaultnum会在doevent中初始化为按钮的编号，结合defaultnum判断可以保证sleep和shutdown都被阻止时能够正确显示信息
    switch (action) {
    case 1 :
        str = QObject::tr("The following program is running to prevent the system from hibernate!");
        break;
    case 2 :
        str = QObject::tr("The following program is running to prevent the system from suspend!");
        break;
    case 4:
        str = QObject::tr("The following program is running to prevent the system from logout!");
        break;
    case 6 :
        str = QObject::tr("The following program is running to prevent the system from reboot!");
        break;
    case 8 :
        str = QObject::tr("The following program is running to prevent the system from shutting down!");
        break;
    }

    tips->setText(str);
    tips->setAlignment(Qt::AlignCenter);
    tips->setContentsMargins(0,0,0,0);
    tips->setStyleSheet(QString::fromUtf8("color:white"));//;font:14pt;

    //数据模型
    m_inhibitDataModel = new QStandardItemModel(this);
    for (auto iter = list.begin(); iter != list.end(); ++iter) {
        QIcon icon;
        QString appName = iter->name;
        QString iconName = iter->icon;

        if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
            icon = QIcon::fromTheme(iconName);
        } else if (QIcon::hasThemeIcon("application-x-desktop")) {
            icon = QIcon::fromTheme("application-x-desktop");
        }

        m_inhibitDataModel->appendRow(new QStandardItem(icon, appName));
    }

    //列表视图
    m_inhibitAppList = new MyListView(m_showWarningArea);
    m_inhibitAppList->setObjectName(QString::fromUtf8("applist"));

    if (isEnoughBig) {
        m_inhibitAppList->setFixedSize(520 * (isEnoughBig ? 1: m_screen.width()/1920), 320 * (isEnoughBig ? 1 : m_screen.height()/1080));
    } else {
        m_inhibitAppList->setGeometry(0,0,520 * (isEnoughBig ? 1: m_screen.width()/1920), 320 * (isEnoughBig ? 1 : m_screen.height()/1080));
    }

    m_inhibitAppList->verticalScrollMode();
    m_inhibitAppList->setStyleSheet("QListView#applist{font:10pt;color:white;background-color: rgb(255,255,255,80);border-style: outset;border-width: 0px;border-radius: 6px;}\
                                    QListView#applist::item{height:48px;margin-top:2px;border-radius: 6px;}\
                                    QListView#applist::item::selected {background-color: rgb(255,255,255,80);border: 1px solid #296CD9;\
                                    height:48px;margin-top:2px;border-radius: 6px;}\
                                    QListView#applist::item::hover {background-color: rgb(255,255,255,80);height:48px;margin-top:2px;border-radius: 6px;}");

    m_inhibitAppList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_inhibitAppList->setIconSize(QSize(32,32));
    m_inhibitAppList->setModel(m_inhibitDataModel);
    m_inhibitAppList->setMinimumHeight(40);
    m_inhibitAppList->verticalScrollBar()->setStyleSheet("QScrollBar{ background: transparent; margin-top:3px;margin-bottom:3px ; }"\
                                                "QScrollBar:vertical{width: 6px;background: transparent;border-radius:3px;}"\
                                                "QScrollBar::handle:vertical{width: 6px; background: rgba(255,255,255, 40); border-radius:3px;}"\
                                                "QScrollBar::handle:vertical:hover{width: 6px; background: rgba(255,255,255, 60); border-radius:3px;}"\
                                                "QScrollBar::add-line:vertical{width:0px;height:0px}"\
                                                "QScrollBar::sub-line:vertical{width:0px;height:0px}");

    //继续操作按钮
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    QString confirBTnText;

    if (inhibitSleep) {
        if (defaultnum == 1) {
            confirBTnText = (QObject::tr("Still Hibernate"));
        } else if (defaultnum == 2) {
            confirBTnText = (QObject::tr("Still Suspend"));
        }
    }

    if (inhibitShutdown) {
        if (defaultnum == 6) {
            confirBTnText = (QObject::tr("Still Reboot"));
        } else if (defaultnum == 8) {
            confirBTnText = (QObject::tr("Still Shutdown"));
        }
    }

    CommonPushButton *confirmBtn = new CommonPushButton(confirBTnText, QString::fromUtf8("confirmBtn"), 120, 48, 24, m_showWarningArea);

    connect(confirmBtn, &CommonPushButton::clicked, [this]() {
        gs->set("win-key-release", false);
        qDebug() << "Start do action" << defaultnum;
        this->signalTostart();
    });

    //取消按钮
    m_inhibitCancelBtn = new CommonPushButton(QObject::tr("Cancel"), QString::fromUtf8("cancelBtn"), 120, 48, 24, m_showWarningArea);
//    cancelBtn->setStyleSheet("QPushButton#cancelBtn {\
//                              background-color: rgb(255,255,255,80);\
//                              border-style: outset;\
//                              border-width: 0px;\
//                              border-radius: 24px;}");
    connect(m_inhibitCancelBtn, &CommonPushButton::clicked, this, &MainWindow::exitt);

    qDebug() << "applist->width():" << m_inhibitAppList->width() << m_inhibitCancelBtn->width();
    //hBoxLayout->setContentsMargins(0,0,0,0);
    //hBoxLayout->setSpacing(isEnoughBig ? 24 : 12);
    hBoxLayout->addStretch();
    //hBoxLayout->addWidget(confirmBtn);
    hBoxLayout->addWidget(m_inhibitCancelBtn);
    hBoxLayout->addStretch();

    vBoxLayout->addWidget(tips);
    vBoxLayout->addWidget(m_inhibitAppList, 0, Qt::AlignHCenter);
    vBoxLayout->addSpacing(isEnoughBig ? 32 : 0);
    vBoxLayout->addLayout(hBoxLayout, Qt::AlignHCenter);

    //移动整个区域到指定的相对位置
    m_showWarningArea->move(xx + (m_screen.width() - m_showWarningArea->width()) / 2, (yy + 266 * m_screen.height()/1080) + (isEnoughBig ? 0 : 10));
//    applist->move((area->width() - applist->width()) / 2, isEnoughBig ? 51 : 32);
    m_showWarningArea->setContentsMargins(0,0,0,0);
    m_showWarningArea->setLayout(vBoxLayout);
    m_showWarningArea->show();

    m_showWarningMesg = true;
    m_inhibitList = list;
}

void MainWindow::judgeboxShow()
{
    click_blank_space_need_to_exit = false;
    m_enterJudgeBox = true;
    for (int j = 0; j < 9; j++) {
        map[j]->hide();
    }
    m_scrollArea->verticalScrollBar()->setVisible(false);

    setLayoutWidgetVisible(m_dateTimeLayout, false);
    setLayoutWidgetVisible(m_judgeWidgetVLayout, true);
    m_judgeWidget->setVisible(true);
}

bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    const uint8_t responseType = event->response_type & ~0x80;
    if (responseType == XCB_CONFIGURE_NOTIFY) {
        xcb_configure_notify_event_t *xc = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        XClassHint ch;
        ch.res_name = NULL;
        ch.res_class = NULL;
        XGetClassHint(QX11Info::display(), xc->window, &ch);
        qDebug() << "ch name is " << ch.res_name;
        if (QString(ch.res_name) == "ukui-session-tools") {
            if (ch.res_name) {
                XFree(ch.res_name);
            }

            if (ch.res_class) {
                XFree(ch.res_class);
            }

            return false;
        }

        if (xc->event == QX11Info::appRootWindow()) {
            XRaiseWindow(QX11Info::display(), this->winId());
            XFlush(QX11Info::display());
        }

        return false;
    } else if (responseType == XCB_MAP_NOTIFY) {
        xcb_map_notify_event_t *xc = reinterpret_cast<xcb_map_notify_event_t*>(event);
        XClassHint ch;
        ch.res_name = NULL;
        ch.res_class = NULL;
        XGetClassHint(QX11Info::display(), xc->window, &ch);
        qDebug() << "ch name is " << ch.res_name;
        if (QString(ch.res_name) == "ukui-session-tools") {
            if (ch.res_name) {
                XFree(ch.res_name);
            }

            if (ch.res_class) {
                XFree(ch.res_class);
            }

            return false;
        }

        XRaiseWindow(QX11Info::display(), this->winId());
        XFlush(QX11Info::display());
    }
    return false;
}

