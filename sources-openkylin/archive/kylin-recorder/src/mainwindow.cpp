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
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "QPixmap"
#include <QLabel>
#include <QSlider>
#include <qtextcodec.h> //保存中文路径时用上此头文件
#include <QMessageBox>
#include <QFileDialog>
#include <kabase/currency.hpp>
#include <time.h>
#include "tools.h"
#include "clipwidget.h"
#include "fileitem.h"
// #include "xatom-helper.h"
#include "libkydate.h"
#include <libkydatacollect.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>

#define BufferSize 35280
// const qint64 TIME_TRANSFORM = 1000 * 1000;              // 微秒转秒;
#define rectangleCount 48 //矩形条个数

MainWindow *MainWindow::mutual = nullptr; //初始化！！！

MainWindow::MainWindow(QStringList str, QWidget *parent) : QWidget(parent)
{
    checkSingle(str);   //检查单例模式
    m_ukuiWindowHelper = new UkuiWindowHelper(this);
    setMinimumSize(810, 460);
    this->resize(810, 460);
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2);
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    setMouseTracking(true);
    m_widget = new QWidget(this);
    // 关键：让widget自动填充父容器,
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QLabel *imageLabel = new QLabel(m_widget);
    imageLabel->setAlignment(Qt::AlignCenter); // 标签内容居中
    imageLabel->setFixedSize(100, 100);
    imageLabel->setPixmap(QIcon::fromTheme("kylin-recorder").pixmap(imageLabel->size()));
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(imageLabel);
    m_widget->setLayout(layout);
    m_mainLayout = new QVBoxLayout();        //主窗体的布局
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_widget);
    this->setLayout(m_mainLayout);
    blurEffect();
    QTimer::singleShot(100, [this, str]() {
    m_widget->hide();
    m_mainLayout->removeWidget(m_widget);
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
        if (getpid() == kdk::WindowManager::getPid(window_id) && m_window_id == 0)
        {
            m_window_id = window_id.toULongLong();
        }
   });

    mutual = this;
    //线程的使用
    m_mainThread = new QThread(this); //主线程
    m_subThread = new MyThread;       //子线程不要传参数this
    qDebug() << "主线程:" << QThread::currentThread();
    m_subThread->moveToThread(m_mainThread);

    int res; //返回结果
    res = DATABASE_INSTANCE.initDataBase();
    if (res != DB_OP_SUCC) {
        qDebug() << "数据库加载失败" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    }
    initGsetting();     //初始化GSetting配置文件
    initDbus();         //初始化插拔信号和s3/s4的dbus
    initMainWindow();   //初始化主界面
    setTwoPageWindow(); //设置第二个页面
    m_mainThread->start();
    this->setAttribute(Qt::WA_AlwaysShowToolTips);//设置始终显示tooltip
    MainWindowLayout();        //主窗体布局方法
    bindGSetting();            //绑定当前GSetting
    //currentGSettingChangeUI(); // GSetting未变化时用当前的值
    mainWindow_page2(); //必须加上初始化第二个主页面(此函数有两处需要被调用:构造函数+切换页面时)
    updateGsetting_ListWidget();
    initShortcutStyle();

    //这个分支要放在下面,对于执行命令时有些还没来得及实例化所以要放在后面
    if (!m_argName.isEmpty()) {
        qDebug() << "单例参数argName = " << m_argName;
        int num = m_argName.size();
        switch (num) {
        case 1:
            if(QFileInfo::exists(m_argName[0])){   // 不支持用录音打开第三方文件
                QMessageBox::warning(this, tr("Alert"), tr("Recorder does not support selecting the opening method to open files!"));
                break;
            }else{
                command_Control(m_argName[0]);
                break;
            }
        default:
            break;
        }
    }
    m_isFirstObject = false; //可以接收外部命令
        kdk_dia_upload_program_startup();
    connect(m_subThread, &MyThread::audioError, this, &MainWindow::onAudioInputError);
    QTimer::singleShot(0, this, [this]{
                onVolumeDeviceChanged(0, QString(), QString());
            });

      });
}

//初步配置文件
void MainWindow::initGsetting()
{
    if (QGSettings::isSchemaInstalled(KYLINRECORDER)) {
        m_defaultPathData = new QGSettings(KYLINRECORDER);
        m_source = m_defaultPathData->get("source").toInt();
    }
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }
    if (QGSettings::isSchemaInstalled(FITCONTROLTRANS)) {
        m_pGsettingControlTrans = new QGSettings(FITCONTROLTRANS);
    }
}

void MainWindow::bindGSetting()
{
    if (m_defaultPathData != nullptr) {
        connect(m_defaultPathData, &QGSettings::changed, this, &MainWindow::changePathDataSlot);
    }
    if (m_themeData != nullptr) {
        connect(m_themeData, &QGSettings::changed, this, &MainWindow::changeThemeColorSlot);
    }
    if (m_pGsettingControlTrans != nullptr) {
        connect(m_pGsettingControlTrans, &QGSettings::changed, this, &MainWindow::changeControlTransSlot);
    }
}

void MainWindow::currentGSettingChangeUI()
{
    if (m_defaultPathData != nullptr) {
        sourceInput(m_defaultPathData->get("source").toInt());
    }
    if (m_themeData != nullptr) {
        themeStyle(m_themeData->get("styleName").toString());
        themeFontStyle(m_themeData->get("systemFont").toString(), m_themeData->get("systemFontSize").toInt());
    }
    if (m_pGsettingControlTrans != nullptr) {
        transChange();
    }
}

void MainWindow::changePathDataSlot(const QString &key)
{
    if (key == "source") {

        m_source = m_defaultPathData->get("source").toInt();
        sourceInput(m_source);
    }
}

void MainWindow::changeThemeColorSlot(const QString &key)
{
#ifndef WHITE_TEST
    if (key == "styleName") {

        m_theme = m_themeData->get("styleName").toString();
        themeStyle(m_theme);
        qDebug() << "主题:" << m_theme;
        m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal");
        adjustBackground();

    } else if (key == "systemFont") {

        m_fontType = m_themeData->get("systemFont").toString();
        m_fontSize = m_themeData->get("systemFontSize").toInt();
        themeFontStyle(m_fontType, m_fontSize);

    } else if (key == "systemFontSize") {

        m_fontType = m_themeData->get("systemFont").toString();
        m_fontSize = m_themeData->get("systemFontSize").toInt();
        themeFontStyle(m_fontType, m_fontSize);

    } else if (key == "themeColor"){
        m_themeColor = m_themeData->get("themeColor").toString();
        emit themeColorchange();
        qDebug() << "主题颜色切换为:" << m_themeColor;
    }
#endif
}

void MainWindow::changeControlTransSlot(const QString &key)
{
    if (key == "transparency" || key == "effect") {
        transChange();
    }
}

void MainWindow::initDbus()
{
    // 用户手册功能
    m_DaemonIpcDbus = new DaemonDbus();
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("org.ukui.kylin_recorder")) {
        sessionBus.registerObject("/org/ukui/kylin_recorder", this, QDBusConnection::ExportAllContents);
        qDebug() << "初始化DBUS成功";
    }else
        qDebug() << "初始化DBUS失败";
    // S3 S4策略
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"), QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"), QString("PrepareForShutdown"), this,
                                         SLOT(onPrepareForShutdown(bool)));
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"), QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"), QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));
    QDBusConnection::sessionBus().connect(
                    "org.ukui.volume.control",
                    "/org/ukui/volume/control",
                    "org.ukui.volume.control",
                    "deviceChanged",this,
                    SLOT(onVolumeDeviceChanged(int,QString,QString))
                    );
    QDBusConnection::sessionBus().connect(
                            "org.ukui.volume.control",
                            "/org/ukui/volume/control",
                            "org.ukui.volume.control",
                            "deviceAdjust",this,
                            SLOT(onVolumeDeviceAdjust(int))
                            );

//    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
//                                          "/com/kylin/kysdk/Date",
//                                          "com.kylin.kysdk.DateInterface",
//                                          "LongDateSignal",
//                                          this,
//                                          SLOT(updateDateFormat(QString)));
    DbusAdapter *dbs_adapter = new DbusAdapter;
    connect(dbs_adapter, &DbusAdapter::sigShowPause, this, &MainWindow::receivepause);

}

void MainWindow::receivepause()
{
    // 如果是播放列表页面，则停止播放音频
    if(m_pStackedWidget->currentIndex() == 2){
        // 通过FileItem静态接口获取“正在播放”那一项
        FileItem* playing = FileItem::currentPlayingItem();
        if(playing){
            // 调用暂停方法，将播放状态切换为暂停
            playing->handlePause();
        }
    }
}

//初始化主界面
void MainWindow::initMainWindow()
{
    int WIDTH = 810;
    int HEIGHT = 460;
    this->installEventFilter(this);
    this->setFixedSize(WIDTH, HEIGHT);
    this->setWindowTitle(tr("Recorder"));
    //this->setProperty("useSystemStyleBlur", true);          //毛玻璃
    //this->setAttribute(Qt::WA_TranslucentBackground, true); //毛玻璃
//    setAttribute(Qt::WA_TranslucentBackground);
//    KWindowEffects::enableBlurBehind(winId(), true); //需要窗口先设置为透明背景
    //显示在活动屏幕中间新方法
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    m_tipWindow = new TipWindow();

    m_titleLeftWidget = new QWidget(); 
    m_titleLeftWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_titleLeftWidget");
    m_titleRightWidget = new QWidget();
    m_titleRightWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_titleRightWidget");

    m_titleWidget = new QWidget();           //标题栏Wid
    m_titleWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_titleWidget");
    m_pStackedWidget = new QStackedWidget(); //录音图标和波形图的堆栈Wid
    m_pStackedWidget->setAccessibleName("kylin-recorder_mainwindow_QStackedWidget_m_pStackedWidget");
    m_fileListWidget = new QWidget();        //列表Wid
    m_fileListWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_fileListWidget");
    m_leftMainWidget = new QWidget();        //主左Wid
    m_leftMainWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_leftMainWidget");
    m_rightMainWidget = new QWidget();       //主右Wid
    m_rightMainWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_rightMainWidget");

    m_firstPageWidget = new QWidget(); //初始页面只有一个大按钮
    m_firstPageWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_firstPageWidget");
    m_listPageWidget = new QWidget();  //含有列表和按钮的页面
    m_listPageWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_listPageWidget");
    m_centerWidget = new QWidget();    //中间Wid
    m_centerWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_centerWidget");
    m_fileListWidgetLayout = new QVBoxLayout();
    m_recordButtonWidget = new QWidget();
    m_recordButtonWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_recordButtonWidget");
    m_titleLeftLayout = new QHBoxLayout();  //左标题布局
    m_titleRightLayout = new QHBoxLayout(); //右标题布局
    m_titleLayout = new QHBoxLayout();      //标题栏布局

    m_leftMainWidLayout = new QHBoxLayout();  //主左布局
    m_rightMainWidLayout = new QVBoxLayout(); //主右布局

    m_firstPageLayout = new QHBoxLayout(); //第一个页面布局
    m_listPageLayout = new QHBoxLayout();  //第三个页面布局
    m_centerLayout = new QHBoxLayout();    //中间布局

    m_appPicture = new QPushButton(this); //窗体左上角标题图片
    m_appPicture->setAccessibleName("kylin-recorder_QPushButton_m_appPicture");
    m_appPicture->setIcon(QIcon::fromTheme("kylin-recorder", QIcon(":/png/png/recording_32.png")));
    m_appPicture->setFixedSize(25, 25);
    m_appPicture->setIconSize(QSize(25, 25)); //重置图标大小
    m_appPicture->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}");

    m_appLabel = new QLabel(this);
    m_appPicture->setAccessibleName("kylin-recorder_QLabel_m_appLabel");
    m_appLabel->setText(tr("Recorder")); //？字体待修改
    m_appLabel->setFixedHeight(24);
    m_appLabel->setProperty("isWindowLabel" , true);

    //(三)按钮及下拉菜单
    m_menuModule = new menuModule(this);
    m_menuModule->setAccessibleName("kylin-recorder_mainwindow_menuModule_m_menuModule");
    m_menuModule->m_menuButton->setToolTip(tr("Options"));
    m_menuModule->m_menuButton->setFixedSize(30, 30);
    m_menuModule->m_menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuModule->m_menuButton->setAutoRaise(true);
    m_menuModule->m_menuButton->setProperty("isWindowButton", 0x1);
    m_menuModule->m_menuButton->setProperty("useIconHighlightEffect", 0x2);
    m_menuModule->m_menuButton->setPopupMode(QToolButton::InstantPopup); // QToolButton必须加上的

    // mini按钮
    m_miniButton = new QToolButton(this);
    m_miniButton->setAccessibleName("kylin-recorder_mainwindow_QToolButton_m_miniButton");
    m_miniButton->setFixedSize(30, 30);
    m_miniButton->setToolTip(tr("Mini"));
    m_miniButton->setProperty("isWindowButton", 0x1);
    m_miniButton->setProperty("useIconHighlightEffect", 0x2);
    m_miniButton->setAutoRaise(true);
    m_miniButton->setIcon(QIcon::fromTheme("ukui-mini-symbolic")); //主题库的mini图标

    //最小化按钮
    m_minButton = new QToolButton(this);
    m_minButton->setAccessibleName("kylin-recorder_mainwindow_QToolButton_m_minButton");
    m_minButton->setFixedSize(30, 30);
    m_minButton->setToolTip(tr("Minimize"));
    m_minButton->setProperty("isWindowButton", 0x1);
    m_minButton->setProperty("useIconHighlightEffect", 0x2);
    m_minButton->setAutoRaise(true);
    m_minButton->setIcon(QIcon::fromTheme("window-minimize-symbolic")); //主题库的最小化图标

    //关闭按钮
    m_closeButton = new QToolButton(this);
    m_closeButton->setAccessibleName("kylin-recorder_mainwindow_QToolButton_m_closeButton");
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setAutoRaise(true);

    m_firstRecordButton = new QPushButton(this);
    m_firstRecordButton->setAccessibleName("kylin-recorder_QPushButton_m_firstRecordButton");
    m_firstRecordButton->setToolTip(tr("Recording Ctrl+R"));
    m_firstRecordButton->setFixedSize(QSize(102, 102));
    m_firstRecordButton->setIconSize(QSize(102, 102));
    m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                       "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                       "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

    m_thirdRecordButton = new QPushButton(this);
    m_thirdRecordButton->setAccessibleName("kylin-recorder_QPushButton_m_thirdRecordButton");
    m_thirdRecordButton->setToolTip(tr("Recording Ctrl+R"));
    m_thirdRecordButton->setFixedSize(QSize(102, 102));
    m_thirdRecordButton->setIconSize(QSize(102, 102));
    m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                       "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                       "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");


    connect(m_miniButton, &QToolButton::clicked, this, &MainWindow::miniShow); // mini窗口
    connect(m_minButton, &QToolButton::clicked, this, &MainWindow::minShow);   //最小化窗口
    //    connect(maxButton,    &QToolButton::clicked, this, &MainWindow::maxShow);//最大化窗口
    connect(m_closeButton, &QToolButton::clicked, this, &MainWindow::closeWindow); //关闭
    connect(m_actionSet, &QAction::triggered, this, &MainWindow::goset);           //跳转设置界面
    //主界面按钮
    connect(m_firstRecordButton, &QPushButton::clicked, this, &MainWindow::switchPage); //首页按钮绑定
    connect(m_thirdRecordButton, &QPushButton::clicked, this, &MainWindow::switchPage); //第三个页面按钮的绑定
    connect(m_pStackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::monitorStackChanged);

    //快捷键
    connect(this, &MainWindow::keySpaceSignal, this, &MainWindow::keySpaceEvent);
    connect(this, &MainWindow::keyMSignal, this, &MainWindow::keyMEvent);
    connect(this, &MainWindow::keyCSignal, this, &MainWindow::keyCEvent);
    connect(this, &MainWindow::keyOSignal, this, &MainWindow::keyOEvent);
    connect(this, &MainWindow::switchPageSignal, this, &MainWindow::switchPage);
    connect(this, &MainWindow::stop_clickedSignal, this, &MainWindow::stop_clicked);
}

void MainWindow::monitorStackChanged(int index)
{
    m_index = index;
    qDebug() << "当前页面索引:" << index;
    if (!m_themeData) {
        qDebug() << "从未实例化过，就实例化一次";
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }
    m_theme = m_themeData->get("styleName").toString();
    if (m_index == 2) {
        if (m_theme == "ukui-dark" || m_theme == "ukui-black") {
            m_titleRightColor = ".QWidget{background-color:#121212;}";
        } else {
            m_titleRightColor = ".QWidget{background-color:#FFFFFF;}";
        }
    } else {
        m_titleRightColor = "";
    }
    //m_titleRightWidget->setStyleSheet(m_titleRightColor);
}

//设置第二个页面
void MainWindow::setTwoPageWindow()
{

    //第二个页面的控件初始化
    m_delayTimer = new QTimer; // QTimer不可以在线程的构造函数里
    m_showTimeLabel = new QLabel(this);
    m_showTimeLabel->setAccessibleName("kylin-recorder_QLabel_m_showTimeLabel");

    m_finishButton = new kdk::KPushButton(this);
    m_finishButton->setAccessibleName("kylin-recorder_mainwindow_KPushButton_m_finishButton");
    m_finishButton->setIcon(QIcon::fromTheme("media-playback-stop-symbolic"));
    m_finishButton->setIconHighlight(true);
    m_finishButton->setButtonType(kdk::KPushButton::CircleType);
    m_finishButton->setFixedSize(56, 56);
    m_finishButton->setIconSize(QSize(20, 20));
    m_finishButton->setToolTip(tr("Stop/Finish Esc"));
    m_finishButton->setTranslucent(true);


    m_pauseButton = new kdk::KPushButton(this);
    m_pauseButton->setAccessibleName("kylin-recorder_mainwindow_KPushButton_m_pauseButton");
    m_pauseButton->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));
    m_pauseButton->setFixedSize(56, 56);
    m_pauseButton->setButtonType(kdk::KPushButton::CircleType);
    m_pauseButton->setIconSize(QSize(23, 23));
    m_pauseButton->setTranslucent(true);

    m_continueButton = new kdk::KPushButton();
    m_continueButton->setAccessibleName("kylin-recorder_mainwindow_KPushButton_m_continueButton");
    m_continueButton->setIcon(QIcon::fromTheme("ukui-play-full-symbolic"));
    m_continueButton->setFixedSize(56, 56);
    m_continueButton->setButtonType(kdk::KPushButton::CircleType);
    m_continueButton->setIconSize(QSize(23, 23));
    m_continueButton->setTranslucent(true);


    m_flagButton = new kdk::KPushButton(this);
    m_flagButton->setAccessibleName("kylin-recorder_mainwindow_KPushButton_m_flagButton");
    m_flagButton->setIcon(QIcon::fromTheme("ukui-label-fill-symbolic"));
    m_flagButton->setFixedSize(56, 56);
    m_flagButton->setToolTip(tr("Flag"));
    m_flagButton->setButtonType(kdk::KPushButton::CircleType);
    m_flagButton->setIconSize(QSize(20, 20));
    m_flagButton->setTranslucent(true);

    leftButton = new QToolButton(this);
    leftButton->setAccessibleName("kylin-recorder_mainwindow_QToolButton_leftButton");
    leftButton->setToolTip(tr("Left"));
    leftButton->setFixedWidth(20);
    leftButton->setIcon(QIcon::fromTheme("ukui-start-symbolic"));
    leftButton->setProperty("isWindowButton", 0x1);
    leftButton->setProperty("useIconHighlightEffect", 0x2);
    leftButton->setAutoRaise(true);
    rightButton = new QToolButton(this);
    rightButton->setAccessibleName("kylin-recorder_mainwindow_QToolButton_rightButton");
    rightButton->setToolTip(tr("Right"));
    rightButton->setFixedWidth(20);
    rightButton->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
    rightButton->setProperty("isWindowButton", 0x1);
    rightButton->setProperty("useIconHighlightEffect", 0x2);
    rightButton->setAutoRaise(true);
    leftButton->hide();
    rightButton->hide();

    m_pageTwoWid = new QWidget(this);    //第二个页面wid
    m_pageTwoWid->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_pageTwoWid");
    m_showTimelbWid = new QWidget(this); //时间wid
    m_showTimelbWid->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_showTimelbWid");
    m_waveWidget = new QWidget(this);    //波形图wid
    m_waveWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_waveWidget");
    m_controlPlayAndPauseWid = new QWidget(this);
    m_controlPlayAndPauseWid->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_controlPlayAndPauseWid");
    flagWidget = new QWidget(this);    //标记Wid
    flagWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_flagWidget");
    elementWidget = new QWidget(this); //元素Wid
    elementWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_elementWidget");

    m_showTimelbLayout = new QHBoxLayout();
    m_controlPlayAndPauseLayout = new QHBoxLayout();
    flagLayout = new QHBoxLayout();    //标记布局
    elementLayout = new QHBoxLayout(); //元素布局
    m_pageTwoLayout = new QVBoxLayout();
    m_pageTwoLayout->addSpacing(20);
    // m_mpvPlayer = new MMediaPlayer;  // mpv播放组件
    // m_playList = new QMediaPlaylist; //播放列表


    //主界面右侧listWidget
    m_fileListTitleLabel = new QLabel(this);
    m_fileListTitleLabel->setAccessibleName("kylin-recorder_QLabel_m_fileListTitleLabel");
    m_fileListTitleLabel->setText(tr("File List"));

    QPalette pal = QApplication::palette();
    QColor aColor = pal.color(QPalette::Active,QPalette::Base);
    QColor iColor = pal.color(QPalette::Inactive,QPalette::Base);
    pal.setColor(QPalette::Active,QPalette::Background,aColor);
    pal.setColor(QPalette::Inactive,QPalette::Background,iColor);

    m_fileListTitleLabelWidget = new QWidget();
    m_fileListTitleLabelWidget->setAccessibleName("kylin-recorder_mainwindow_QWidget_m_fileListTitleLabelWidget");
    m_fileListTitleLabelLayout = new QHBoxLayout();
    m_fileListTitleLabelWidget->setPalette(pal);
    m_fileListTitleLabelWidget->setAutoFillBackground(true);
    m_zeroFileMessageLabel = new QLabel(this);
    m_zeroFileMessageLabel->setAccessibleName("kylin-recorder_QLabel_m_zeroFileMessageLabel");

    m_seatLabel = new QLabel(this);
    m_seatLabel->setAccessibleName("kylin-recorder_QLabel_m_seatLabel");
    m_seatLabel->setText("  "); //占位

    m_list = new QListWidget(this);
    m_list->setAccessibleName("kylin-recorder_mainwindow_QListWidget_m_list");
    m_list->installEventFilter(this); //安装事件过滤器
    m_list->setSortingEnabled(true);
    m_list->sortItems(Qt::DescendingOrder);
    m_list->setViewMode(QListView::ListMode);
    m_list->setFrameShape(QListWidget::NoFrame);                           //去掉list的边框
    m_list->verticalScrollBar()->setStyleSheet(".QScrollBar{width:8px;}"); //设置整个滑动条宽度
    m_list->setStyleSheet(".QListWidget::item:hover{background:transparent;}");
    m_list->setSelectionMode(QAbstractItemView::NoSelection); // list取消原装的选中功能

    connect(m_delayTimer, &QTimer::timeout, m_subThread, &MyThread::fun);
    connect(this, &MainWindow::startRecord, m_subThread, &MyThread::record_pressed);
    connect(this, &MainWindow::reRecordSignla, m_subThread, &MyThread::reRecordSlots);
    connect(m_subThread, &MyThread::recordPaint, this, &MainWindow::recordPaint); //绘波形图
    connect(m_subThread, &MyThread::changeVoicePicture, this, &MainWindow::changeVoicePicture);
    connect(this, &MainWindow::stopRecord, m_subThread, &MyThread::stopBtnPressed);
    connect(this, &MainWindow::playRecord, m_subThread, &MyThread::playRecord);
    connect(this, &MainWindow::pauseRecord, m_subThread, &MyThread::pauseRecord);
    connect(m_subThread, &MyThread::recordDurationChanged, this, &MainWindow::updateRecordDuration);

    connect(m_finishButton, &kdk::KPushButton::clicked, this, &MainWindow::stop_clicked);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::play_pause_clicked);
    connect(m_continueButton, &QPushButton::clicked, this, &MainWindow::play_pause_clicked);
    connect(m_flagButton, &QPushButton::clicked, this, &MainWindow::handleInsertSlot);
    connect(leftButton, &QToolButton::clicked, this, &MainWindow::findLeftSlot);
    connect(rightButton, &QToolButton::clicked, this, &MainWindow::findRightSlot);
}

void MainWindow::findLeftSlot()
{
#ifndef WHITE_TEST
    QPushButton *firstDisplayBtn = qobject_cast<QPushButton *>(elementLayout->itemAt(0)->widget());
    QPushButton *flagListfirstBtn = m_flagList.at(0);
    QString firstDisplayStr = firstDisplayBtn->text();
    QString flagListfirstStr = flagListfirstBtn->text();
    qDebug() << "能显示的起始flag：" << firstDisplayBtn->text() << "flagList的起始的flag：" << flagListfirstBtn->text();
    if (firstDisplayStr.compare(flagListfirstStr) != 0) {
        QString tempStr;
        int indexOfFirstFlag = 0;
        for (int i = 0; i < m_flagList.count(); i++) {
            tempStr = m_flagList.at(i)->text();
            if (firstDisplayStr.compare(tempStr) == 0) {
                indexOfFirstFlag = i;
                break;
            }
        }
        if (indexOfFirstFlag > 0) {
            indexOfFirstFlag--;
            qDebug() << "elementLayout的控件个数:" << elementLayout->count();
            while (elementLayout->count()) {
                QWidget *p = elementLayout->itemAt(0)->widget();
                elementLayout->removeWidget(p);
                p->hide();
            }
            for (int j = indexOfFirstFlag; j <= indexOfFirstFlag + MAX_FLAG_NUM - 1; j++) {
                elementLayout->addWidget(m_flagList.at(j));
                m_flagList.at(j)->show();
            }
            elementWidget->setLayout(elementLayout);
        }

    } else {
        qDebug() << "已经到开始了！";
    }
#endif
}

void MainWindow::findRightSlot()
{
#ifndef WHITE_TEST
    QPushButton *endDisplayBtn = qobject_cast<QPushButton *>(elementLayout->itemAt(MAX_FLAG_NUM - 1)->widget());
    QPushButton *flagListEndBtn = m_flagList.at(m_flagList.count() - 1);
    QString endDisplayStr = endDisplayBtn->text();
    QString flagListEndStr = flagListEndBtn->text();
    qDebug() << "能显示的最后一个flag：" << endDisplayBtn->text() << "flagList的最后的flag：" << flagListEndBtn->text();
    if (endDisplayStr.compare(flagListEndStr) != 0) {

        QString tempStr;
        int indexOfEndFlag = 0;
        for (int i = 0; i < m_flagList.count(); i++) {
            tempStr = m_flagList.at(i)->text();
            if (endDisplayStr.compare(tempStr) == 0) {
                indexOfEndFlag = i;
                break;
            }
        }
        if (indexOfEndFlag < m_flagList.count()) {

            indexOfEndFlag++;
            while (elementLayout->count()) {
                QWidget *p = elementLayout->itemAt(0)->widget();
                elementLayout->removeWidget(p);
                p->hide();
            }
            qDebug() << "elementLayout是否是空" << elementLayout->count();
            for (int j = indexOfEndFlag - (MAX_FLAG_NUM - 1); j <= indexOfEndFlag; j++) {
                elementLayout->addWidget(m_flagList.at(j));
                m_flagList.at(j)->show();
            }
            qDebug() << "indexOfEndFlag:" << indexOfEndFlag;
            elementWidget->setLayout(elementLayout);
        }


    } else {
        qDebug() << "已经到最后了！";
    }
#endif
}

void MainWindow::removeFlagLayoutWidget()
{
#ifndef WHITE_TEST
    // 删除布局中所有的控件
    leftButton->hide();
    rightButton->hide();
    elementWidget->hide();
    while (elementLayout->count()) {
        QWidget *p = elementLayout->itemAt(0)->widget();
        elementLayout->removeWidget(p);
        p->deleteLater(); // 清除内存
    }
    m_flagList.clear();
#endif
}

void MainWindow::sendFlagView(){
#ifndef WHITE_TEST

	KCustomProperty property[1];
	property[0].key = (char *)"flagName";
	property[0].value = (char *)"leftButton";
	KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
	kdk_dia_append_custom_property(node, property, 1);
	kdk_dia_upload_default(node, (char *)"audio_flag_event", (char *)"mainwindow");
	kdk_dia_data_free(node);
#endif
}

void MainWindow::handleInsertSlot()
{
#ifndef WHITE_TEST
    recorderBuriedPoint(kdk::kabase::BuriedPoint::PT::KylinRecorderSign);
    QFileInfo fileInfo(m_subThread->getFileName());
    qDebug() << "获取当前无后缀的文件名：" << fileInfo.baseName() << "获取当前时间:" << m_timeStr;
    QStringList Hms = m_timeStr.split(":");
    int flagTime = Hms.at(0).toInt()*3600 + Hms.at(1).toInt()*60 + Hms.at(2).toInt();
    if(flagTime / 21600 == 0){
        elementWidget->show();
        emit DATABASE_INSTANCE.insertDataSignal(fileInfo.absoluteFilePath(), m_timeStr);
        if (DATABASE_INSTANCE.getStatus() == DB_OP_SUCC) {
            qDebug() << "录音时插入标记成功!";
	    this->sendFlagView();
            kdk::KBorderlessButton *flagPicBtn = new kdk::KBorderlessButton(QIcon::fromTheme("ukui-label-symbolic"), m_timeStr, this);
            flagPicBtn->setAccessibleName("kylin-recorder_mainwindow_KBorderlessButton_flagPicBtn");
            flagPicBtn->setObjectName("flag");
            flagPicBtn->setFixedWidth(100);
            //flagPicBtn->setDisabled(true);
            m_flagList.append(flagPicBtn);
            if (m_flagList.count() > 0 && m_flagList.count() < MAX_FLAG_NUM) {
                elementLayout->addWidget(flagPicBtn);
            } else if (m_flagList.count() >= MAX_FLAG_NUM) {
                leftButton->show();
                rightButton->show();
                while (elementLayout->count()) {
                    QWidget *p = elementLayout->itemAt(0)->widget();
                    p->hide();
                    elementLayout->removeWidget(p);
                }
                for (int i = m_flagList.count() - MAX_FLAG_NUM; i < m_flagList.count(); i++) {
                    m_flagList.at(i)->show();
                    elementLayout->addWidget(m_flagList.at(i));
                }
            }
            elementWidget->setLayout(elementLayout);
            qDebug() << "flagPicBtn个数:" << m_flagList.count();
        }
    }else{
        int N = flagTime / 21600;
        int realFlagTime = flagTime - N * 21600;
        QString realTimeStr =  QTime(0, 0, 0, 0).addSecs(realFlagTime).toString("hh:mm:ss");
        QString realAbsoluteFilePath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "[" + QString::number(N) + "]." + fileInfo.completeSuffix();
        elementWidget->show();
        emit DATABASE_INSTANCE.insertDataSignal(realAbsoluteFilePath, realTimeStr);
        if (DATABASE_INSTANCE.getStatus() == DB_OP_SUCC) {
            qDebug() << "录音时插入标记成功!";
            kdk::KBorderlessButton *flagPicBtn = new kdk::KBorderlessButton(QIcon::fromTheme("ukui-label-symbolic"), m_timeStr, this);
            flagPicBtn->setAccessibleName("kylin-recorder_mainwindow_KBorderlessButton_flagPicBtn");
            flagPicBtn->setObjectName("flag");
            flagPicBtn->setFixedWidth(100);
            //flagPicBtn->setDisabled(true);
            m_flagList.append(flagPicBtn);
            if (m_flagList.count() > 0 && m_flagList.count() < MAX_FLAG_NUM) {
                elementLayout->addWidget(flagPicBtn);
            } else if (m_flagList.count() >= MAX_FLAG_NUM) {
                leftButton->show();
                rightButton->show();
                while (elementLayout->count()) {
                    QWidget *p = elementLayout->itemAt(0)->widget();
                    p->hide();
                    elementLayout->removeWidget(p);
                }
                for (int i = m_flagList.count() - MAX_FLAG_NUM; i < m_flagList.count(); i++) {
                    m_flagList.at(i)->show();
                    elementLayout->addWidget(m_flagList.at(i));
                }
            }
            elementWidget->setLayout(elementLayout);
            qDebug() << "flagPicBtn个数:" << m_flagList.count();
        }
    }

#endif
}

int MainWindow::command_Control(QString cmd1)
{
#ifndef WHITE_TEST
    if (m_miniWindow.isHidden() && (isHidden() || !isActiveWindow())) {
        kdk::UkuiStyleHelper::self()->removeHeader(this);
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
        activateWindow();
        setFocus();
    }

    if(!m_miniWindow.isHidden()){
        m_miniWindow.show();
        m_miniWindow.activateWindow();
        m_miniWindow.setFocus();
    }

    qDebug() << "命令:" << cmd1;
    // 990需求命令行控制
    if (m_isFirstObject && !QFileInfo::exists(cmd1)) //首个实例不接受文件以外的参数
    {
        if (cmd1 == "-c" || cmd1 == "-close") {
            qDebug() << "-c" << cmd1;
            this->close();
            exit(0);
        }
        qDebug() << "首个实例不接受文件以外的参数" << cmd1;
        m_isFirstObject = false; //可以接收其他命令
        return 0;
    }else
    if (cmd1 == "") //无参数，单例触发
    {
        // kwin接口唤醒
        KWindowSystem::forceActiveWindow(this->winId());
        qDebug() << "窗口置顶";
        return 0;
    }else
    if (cmd1 == "-s" || cmd1 == "-start") //开始录音
    {
        //------录音------
        qDebug() << "isRecording = " << m_isRecording << " " << m_stratOrPause;
        if (!m_isRecording && !m_stratOrPause) {
            qDebug() << "可以录音";
            switchPage();
        }
        return 0;
    }else
    if (cmd1 == "-p" || cmd1 == "-pause") //暂停
    {
        //------暂停------
        if (m_isRecording) {
            qDebug() << m_isRecording << " " << m_stratOrPause;
            qDebug() << "录音暂停";
            play_pause_clicked(); //检测到正在录音时才可以暂停录制
        } else {
            if (m_stratOrPause) //因为暂停录音时strat_pause会置true
            {
                play_pause_clicked(); //开始录制
            }
        }
        return 0;
    }else
    if (cmd1 == "-f" || cmd1 == "-finish") //结束录音
    {
        qDebug() << "isRecording = " << m_isRecording << "strat_pause = " << m_stratOrPause;
        if (m_isRecording || m_stratOrPause) {
            qDebug() << "可以结束";
            stop_clicked();
        }
        return 0;
    }else
    if (cmd1 == "-c" || cmd1 == "-close") //结束录音
    {
        m_mainThread->quit();
        m_mainThread->wait();
        this->close();
        m_miniWindow.close();
        return 0;
    }else{
        QMessageBox::warning(this, tr("Alert"), tr("Recording does not support opening file in open mode."));
    }
#endif
    return 0; //重要:int类型的函数一定要加返回值
}

//对于台式机，可以收到耳机插拔输入设备的DBus信号
void MainWindow::inputDevice_get(QString str)
{
#ifndef WHITE_TEST

    qInfo()<<"m_source的值为:"<<m_source;
    qDebug() << "輸入設備插拔";

    if(m_source ==2){
        if (m_pStackedWidget->currentIndex() == 1 )  {
            m_finish = true;
            stop_clicked();
        }
    }

    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    if (inputDevice.deviceName().contains("monitor")) {
        if (m_source == 2) {
            m_firstRecordButton->setEnabled(true);
            m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

            m_thirdRecordButton->setEnabled(true);
            m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

            m_miniWindow.m_miniRecordButton->setEnabled(true);
            m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");


        } else {
            //只有在录音状态下才应该停止录音，也就是在录音界面时，停止录音
            if (m_pStackedWidget->currentIndex() == 1 )  {
                m_finish = true;
                stop_clicked();
            }
            m_firstRecordButton->setEnabled(false);
            m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

            m_thirdRecordButton->setEnabled(false);
            m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

            m_miniWindow.m_miniRecordButton->setEnabled(false);
            m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}");
        }
    } else {
        if (m_source == 1) {
            if (m_pStackedWidget->currentIndex() == 1 )  {
                m_finish = true;
                stop_clicked();
            }            
        }
        qDebug() << "inputDevice:" << inputDevice.deviceName();
        m_firstRecordButton->setEnabled(true);
        m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");


        m_thirdRecordButton->setEnabled(true);
        m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

        m_miniWindow.m_miniRecordButton->setEnabled(true);
        m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");

    }
#endif
}

void MainWindow::outputDevice_get(QString str)
{
#ifndef WHITE_TEST

    qDebug() << "輸出設備插拔";
    if (m_source == 2) {
        //stop_clicked();
	if (m_pStackedWidget->currentIndex() == 1 )  {
            m_finish = true;
            stop_clicked();
        }
    } else {
    }
#endif
}

void MainWindow::onPrepareForShutdown(bool Shutdown)
{
    //目前只做事件监听，不处理
    qDebug() << "onPrepareForShutdown" << Shutdown;
}

//监听系统睡眠信号
void MainWindow::onPrepareForSleep(bool isSleep)
{
#ifndef WHITE_TEST

    if (isSleep) //睡眠分支
    {
        qDebug() << "睡眠！！！";
        if (m_isRecording) {
            stop_clicked();
        }
    } else //唤醒分支
    {
        qDebug() << "唤醒！！！";
    }
#endif
}

void MainWindow::handlingSlot()
{
    m_tipWindow->tipLabel->setText(tr("Analysis..."));
    m_tipWindow->move(this->geometry().center() - m_tipWindow->rect().center());
    m_tipWindow->show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100); //防止界面假死

    qDebug() << "接收到界面消息!";
}

void MainWindow::closeWindow()
{
#ifndef WHITE_TEST

    PlayControl::getInstance().stop();
    //无论是正常模式窗口还是mini模式窗口，都能立刻将缓存的raw文件转化为最终音频并保存到本地路径
    updateGsetting_ListWidget();
    if (m_mainThread->isRunning()) {
        m_mainThread->quit();
        m_mainThread->wait();
    }
    kdk_dia_upload_program_exit();
    this->close();
    m_miniWindow.close();
#endif
}

// 实现键盘响应
void MainWindow::keyPressEvent(QKeyEvent *event)
{
#ifndef WHITE_TEST
    if (event->key() == Qt::Key_Space) {
        qDebug() << " key 空格 开始/停止录音";
        Q_EMIT keySpaceSignal();
    } else if (event->key() == Qt::Key_M) {
        qDebug() << "key M: 添加标记";
        Q_EMIT keyMSignal();
    } else if (event->key() == Qt::Key_C) {
        qDebug() << "key C: 剪辑录音";
        Q_EMIT keyCSignal();
    } else if (event->key() == Qt::Key_O) {
        qDebug() << "key O: 在文件夹中显示";
        Q_EMIT keyOSignal();
    } else if (event->key() == Qt::Key_Escape) {
        qDebug() << "key Esc: 停止/完成录音";
        if (m_isRecording || m_stratOrPause) {
            Q_EMIT stop_clickedSignal();
        }
    }

    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        if (!m_DaemonIpcDbus->daemonIsNotRunning()) {
            // F1快捷键打开用户手册，如kylin-recorder
            //由于是小工具类，下面的showGuide参数要填写"kylin-recorder"
            m_DaemonIpcDbus->showGuide("kylin-recorder");
        }
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_R) {
        qDebug() << "key: ctrl + R";
        if (!m_isRecording && !m_stratOrPause) {
            qDebug() << "可以录音";
            Q_EMIT switchPageSignal();
        } else if (m_isRecording || m_stratOrPause) {
            qDebug() << "可以结束";
            Q_EMIT stop_clickedSignal();
        }
    }
#endif
}

MainWindow::~MainWindow()
{
#ifndef WHITE_TEST

    qDebug() << "析构MainWindow";
    QList<FileItem *> List = this->findChildren<FileItem *>();
    for (FileItem *item : List) {
        item->deleteLater();
    }
    QList<myWave *> list = this->findChildren<myWave *>();
    for (myWave *tmp : list) {
        tmp->deleteLater();
    }
    m_tipWindow->deleteLater();
    if (m_subThread != nullptr) {
        if (m_mainThread->isRunning()) {
            qDebug() << "关闭还在运行的线程";
            m_mainThread->quit();
            m_mainThread->wait();
        }
        // delete myThread; // 最后有没有都行！
    }
    m_DaemonIpcDbus->deleteLater();
#endif
}

void MainWindow::isFileNull(int n)
{
    if (n == 0) {
        qDebug() << "文件个数：" << n;
        m_zeroFileMessageLabel->setParent(m_fileListWidget);
        m_zeroFileMessageLabel->show();
        changePage(0);
        //        defaultPathData->set("samefilenum",1);//列表里无文件时归1
    } else {
        changePage(2);;
        m_zeroFileMessageLabel->hide();
    }
}

void MainWindow::MainWindowLayout()
{
    m_zeroFileMessageLabel->setFixedSize(200, 50);
    m_zeroFileMessageLabel->setText(tr("None of the Recording File")); //列表无录音文件时
    m_zeroFileMessageLabel->move((m_fileListWidget->width() - m_zeroFileMessageLabel->width() / 2) / 3,
                                 (m_fileListWidget->height() - m_zeroFileMessageLabel->height() / 3) / 3);

    m_fileListTitleLabelLayout->addWidget(m_fileListTitleLabel);
    m_fileListTitleLabelLayout->setContentsMargins(30, 0, 0, 0); // 31,0,0,0改为31,0,0,10即不多不少显示6个
    m_fileListTitleLabelWidget->setLayout(m_fileListTitleLabelLayout);

    m_fileListWidgetLayout->addWidget(m_fileListTitleLabelWidget);
    //m_fileListTitleLabelWidget->hide();
    m_fileListWidgetLayout->addWidget(m_list);
//    m_list->setMinimumHeight(420);
    m_fileListWidgetLayout->setContentsMargins(0, 0, 4, 0);
    m_fileListWidgetLayout->setSpacing(0);
    //m_fileListWidgetLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    m_fileListWidget->setLayout(m_fileListWidgetLayout);

    m_titleLeftLayout->addWidget(m_appPicture);
    m_titleLeftLayout->addWidget(m_appLabel);
    m_titleLeftLayout->setSpacing(8); //内部按钮间距8px
    m_titleLeftLayout->setContentsMargins(8, 8, 0, 0);
    m_titleLeftWidget->setLayout(m_titleLeftLayout);
    m_titleLeftWidget->setFixedWidth(308);

    m_titleRightLayout->addWidget(m_seatLabel); //占位用的lb
    m_titleRightLayout->addWidget(m_menuModule->m_menuButton);
    m_titleRightLayout->addWidget(m_miniButton);
    m_titleRightLayout->addWidget(m_minButton);
    //    titleRightLayout->addWidget(maxButton);//2020.11.20禁止最大化功能
    m_titleRightLayout->addWidget(m_closeButton);
    m_titleRightLayout->setSpacing(4); //内部按钮间距4px
    m_titleRightLayout->setContentsMargins(0, 4, 4, 0);
    m_titleRightWidget->setLayout(m_titleRightLayout);
    QPalette pal = QApplication::palette();
    QColor aColor = pal.color(QPalette::Active,QPalette::Base);
    QColor iColor = pal.color(QPalette::Inactive,QPalette::Base);
    pal.setColor(QPalette::Active,QPalette::Background,aColor);
    pal.setColor(QPalette::Inactive,QPalette::Background,iColor);
    m_titleRightWidget->setAutoFillBackground(true);
    m_titleRightWidget->setPalette(pal);
    m_fileListWidget->setAutoFillBackground(true);
    m_fileListWidget->setPalette(pal);
    m_titleLayout->addWidget(m_titleLeftWidget);
    m_titleLayout->addWidget(m_titleRightWidget);
    m_titleLayout->setSpacing(0);
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    m_titleWidget->setLayout(m_titleLayout);

    m_recordButtonLayout = new QVBoxLayout();
    m_recordButtonLayout->addWidget(m_thirdRecordButton, 0, Qt::AlignCenter); //录音按钮所在Wid
    m_recordButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_recordButtonWidget->setLayout(m_recordButtonLayout);

    m_pStackedWidget->addWidget(m_firstPageWidget);
    m_pStackedWidget->addWidget(m_pageTwoWid);
    m_pStackedWidget->addWidget(m_listPageWidget);

    m_leftMainWidLayout->addWidget(m_recordButtonWidget, 0, Qt::AlignCenter);
    m_leftMainWidLayout->setContentsMargins(0, 0, 0, 0); //间距30看起来就比较居中
    m_leftMainWidget->setLayout(m_leftMainWidLayout);
    m_leftMainWidget->setFixedWidth(308); // sp2的leftMain宽度

    m_rightMainWidLayout->addWidget(m_fileListWidget);
    m_rightMainWidLayout->setSpacing(0);
    m_rightMainWidLayout->setMargin(0);
    m_rightMainWidget->setLayout(m_rightMainWidLayout);

    m_firstPageLayout->addWidget(m_firstRecordButton, 0, Qt::AlignCenter); //录音按钮所在Wid
    m_firstPageLayout->setContentsMargins(0, 0, 0, 0);                    //间距30看起来就比较居中
    m_firstPageWidget->setLayout(m_firstPageLayout);

    m_listPageLayout->addWidget(m_leftMainWidget);
    m_listPageLayout->addWidget(m_rightMainWidget);
    m_listPageLayout->setSpacing(0);
    m_listPageLayout->setContentsMargins(0, 0, 0, 0);
    m_listPageWidget->setLayout(m_listPageLayout);

    m_mainLayout->addWidget(m_titleWidget);
    m_mainLayout->addWidget(m_pStackedWidget);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_mainLayout);
}

//最小化
void MainWindow::minShow()
{
    this->showMinimized();
}

// mini模式
void MainWindow::miniShow()
{
#ifndef WHITE_TEST

    qDebug() << "切换到mini模式";
    recorderBuriedPoint(kdk::kabase::BuriedPoint::PT::KylinRecorderTopicSwitch);
    m_miniWindow.showNormal();
    kdk::UkuiStyleHelper::self()->removeHeader(&m_miniWindow);
    // 右上角定位（使用“可用区域”避免顶栏/任务栏遮挡）
    QRect avail = qApp->primaryScreen()->availableGeometry();
    m_miniWindow.move(avail.width()-40-m_miniWindow.width(), 40);
    m_miniWindow.activateWindow();
    m_miniWindow.setFocus();
    this->hide();
#endif
}

void MainWindow::maxShow()
{
#ifndef WHITE_TEST

    if (m_isMax) {
        qDebug() << "还原窗口";
        this->showNormal();
        m_isMax = false;
    } else {
        qDebug() << "最大化窗口";
        this->showFullScreen();
        m_isMax = true;
    }
#endif
}
void MainWindow::themeWindow(QString themeColor)
{
    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {
        //主界面
        this->setObjectName("mainWid"); //设置命名空间
        //设置界面
        m_setWindow.m_mainWidget->setObjectName("setMainWid");                             //设置命名空间
        m_setWindow.m_mainWidget->setStyleSheet("#setMainWid{background-color:#131314;}"); //自定义窗体(圆角+背景色)
        if (m_index == 2) {
            m_titleRightColor = ".QWidget{background-color:#121212;}";
        } else {
            m_titleRightColor = "";
        }
    } else {
        //三联好使了
        //m_rightMainWidget->setStyleSheet(".QWidget{background-color:#FFFFFF;}");
        if (m_index == 2) {
            m_titleRightColor = ".QWidget{background-color:#FFFFFF;}";
        } else {
            m_titleRightColor = "";
        }
        //m_titleRightWidget->setStyleSheet(m_titleRightColor);
        //设置界面
        m_setWindow.m_mainWidget->setObjectName("setMainWid");                             //设置命名空间
        m_setWindow.m_mainWidget->setStyleSheet("#setMainWid{background-color:#FFFFFF;}"); //自定义窗体(圆角+背景色)
        //设置界面
        this->setObjectName("mainWid"); //设置命名空间
    }
    QPalette pal = QApplication::palette();
    QColor aColor = pal.color(QPalette::Active,QPalette::Base);
    QColor iColor = pal.color(QPalette::Inactive,QPalette::Base);
    pal.setColor(QPalette::Active,QPalette::Background,aColor);
    pal.setColor(QPalette::Inactive,QPalette::Background,iColor);
    m_fileListTitleLabelWidget->setPalette(pal);
    m_titleRightWidget->setPalette(pal);
    m_fileListWidget->setPalette(pal);
}
void MainWindow::themeButton(QString themeColor)
{
    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {
        m_miniWindow.changeMiniCoutinuePauseBtnThemeColor(themeColor);
        m_miniWindow.m_miniCloseButton->setIcon(QIcon(":/svg/svg/dark-window-close.svg"));
        m_zeroFileMessageLabel->setStyleSheet("font-size:14px;color:#FFFFFF");
        //关闭按钮
        m_closeButton->setFixedSize(30, 30);

    } else {
        m_miniWindow.changeMiniCoutinuePauseBtnThemeColor(themeColor);
        m_miniWindow.m_miniCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
        m_zeroFileMessageLabel->setStyleSheet("font-size:14px;color:#404040");
    }
}
// ffmpeg计算时长
QString MainWindow::playerTotalTime(QString filePath)
{
    FFUtil fu;
    fu.open(filePath);
    int t_duration = fu.getDuration(filePath);
//    fu.close();
    //    qDebug()<<"t_duration"<<t_duration;
    QFile file(filePath);
    qint64 fileSize;
    qint64 time;

    QString durLength;
    fileSize = file.size();

    if (t_duration / 1000 > 30000) {
        qDebug() << "ffmpeg计算出错,用文件大小计算";
        time = fileSize / 64000;
        QTime totalTime(time / 3600, (time % 3600) / 60, time % 60);
        durLength = totalTime.toString("hh:mm:ss");
    } else {
        durLength = Tools::formatMillisecond(t_duration);
    }
    file.close();
    return durLength;
}



void MainWindow::themeFontStyle(QString fontType, int fontSize)
{
    int side = 18 * fontSize / 15;
    QFont font(fontType, side);
    font.setBold(true);
    m_fileListTitleLabel->setFont(font);

    if (fontSize <= 15) {
        qDebug() << "字体类型:" << fontType << "字体大小:" << fontSize;
        QFont fontStyle(fontType, fontSize);
        m_appLabel->setFont(fontStyle);
        m_setWindow.m_setWindowLabel->setFont(fontStyle);
        m_setWindow.m_storeLabel->setFont(fontStyle);
        m_setWindow.m_pathLabel->setFont(fontStyle);
        m_setWindow.m_formatLabel->setFont(fontStyle);
        m_setWindow.m_formatDownList->setFont(fontStyle);
        m_setWindow.m_sourceLabel->setFont(fontStyle);
        m_setWindow.m_sourceDownList->setFont(fontStyle);
        m_setWindow.m_alterButton->setFont(fontStyle);
        emit itemFontStyleSignal(fontStyle); //改变itemswindow字体

    } else {
        qDebug() << "字体大小最大为15";
        QFont fontStyleLimit(fontType, 15);
        m_appLabel->setFont(fontStyleLimit);
        m_setWindow.m_setWindowLabel->setFont(fontStyleLimit);
        m_setWindow.m_storeLabel->setFont(fontStyleLimit);
        m_setWindow.m_pathLabel->setFont(fontStyleLimit);
        m_setWindow.m_formatLabel->setFont(fontStyleLimit);
        m_setWindow.m_formatDownList->setFont(fontStyleLimit);
        m_setWindow.m_sourceLabel->setFont(fontStyleLimit);
        m_setWindow.m_sourceDownList->setFont(fontStyleLimit);
        m_setWindow.m_alterButton->setFont(fontStyleLimit);
        emit itemFontStyleSignal(fontStyleLimit); //改变itemswindow字体
    }
}

void MainWindow::sourceInput(int index)
{
    m_source = m_defaultPathData->get("source").toInt();
    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
//    qDebug () << inputDevice.deviceName();
//    auto l =  QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
//    for(auto d : l){
//        qDebug() << d.deviceName();
//    }
    if (inputDevice.deviceName().contains("monitor")) {
        if (index == 1) {
            m_firstRecordButton->setEnabled(false);
            m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

            m_thirdRecordButton->setEnabled(false);
            m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

            m_miniWindow.m_miniRecordButton->setEnabled(false);
            m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}");

        } else {
            m_firstRecordButton->setEnabled(true);
            m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

            m_thirdRecordButton->setEnabled(true);
            m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

            m_miniWindow.m_miniRecordButton->setEnabled(true);
            m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}"
                                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}"
                                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");
        }

    } else {
        m_firstRecordButton->setEnabled(true);
        m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

        m_thirdRecordButton->setEnabled(true);
        m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

        m_miniWindow.m_miniRecordButton->setEnabled(true);
        m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}"
                                           "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}"
                                           "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");
    }
}

void MainWindow::themeStyle(QString themeColor)
{
    qDebug() << "主题颜色" << themeColor;
    themeButton(themeColor); //控件类切换
    themeWindow(themeColor); //窗体类切换
}

void MainWindow::checkSingle(QStringList path) //检查单例模式
{
    QString str;
    if (path.size() > 1) {
        str = path[1];
    }

    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    //兼容VNC的单例模式
    QString lockPath = homePath.at(0) + "/.config/kylin-recorder-lock";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        exit(1);
    }
    if (lockf(fd, F_TLOCK, 0)) {

        QDBusInterface interface("org.ukui.kylin_recorder", "/org/ukui/kylin_recorder", "org.ukui.kylin_recorder",
                                 QDBusConnection::sessionBus());
        if (path.size() == 1)
            interface.call("command_Control", str);
        if (path.size() == 2)
            interface.call("command_Control", str);

        qDebug() << "Can't lock single file, kylin-recorder is already running!";
        exit(0);
    }
    m_isFirstObject = true; //我是首个对象
    m_argName << str;
    qDebug() << "单例参数argName:" << m_argName << "str:" << str << "path:" << path;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
#ifndef WHITE_TEST

//    m_pressPosRecord = event->pos();
    //只能是鼠标左键移动和改变大小
    if (event->button() == Qt::LeftButton) {
        m_mousePress = true;
        this->setFocus();
    }
#endif
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
#ifndef WHITE_TEST

    m_pressPosRecord = QPoint(0, 0);
    Q_UNUSED(event);
    m_mousePress = false;
#endif
}

bool MainWindow::event(QEvent *event)
{
    //使用X11接管窗口移动事件后，鼠标离开事件不触发，此处作为备用方案
    // if (m_mousePress) {
    //     if (event->type() == QEvent::Move) {
    //         x11EventEnd();
    //     }
    // }
    return QWidget::event(event);
}

// void MainWindow::x11EventEnd()
// {
//     double device = QGuiApplication::primaryScreen()->devicePixelRatio();
//     if (device <= 0) {
//         device = 1;
//     }
//     QPoint globalPoints = QCursor::pos();

//     XEvent xEvent;
//     memset(&xEvent, 0, sizeof(XEvent));
//     Display *display = QX11Info::display();
//     xEvent.type = ButtonRelease;
//     xEvent.xbutton.button = Button1;
//     xEvent.xbutton.window = this->effectiveWinId();
//     if (globalPoints.y() < 5) {
//         xEvent.xbutton.x = globalPoints.x() * device;
//         xEvent.xbutton.y = globalPoints.y() * device;
//     } else {
//         xEvent.xbutton.x = this->mapFromGlobal(globalPoints).x() * device;
//         xEvent.xbutton.y = this->mapFromGlobal(globalPoints).y() * device;
//     }
//     xEvent.xbutton.x_root = globalPoints.x() * device;
//     xEvent.xbutton.y_root = globalPoints.y() * device;
//     xEvent.xbutton.display = display;

//     XEvent xEvent2 = xEvent;
//     xEvent2.type = ButtonPress;
//     XSendEvent(display, this->effectiveWinId(), False, ButtonPressMask, &xEvent2);
//     XFlush(display);

//     XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xEvent);
//     XFlush(display);

// }

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
#ifndef WHITE_TEST
    //手指长按时，过滤2像素差的鼠标移动事件
    if (qAbs(event->pos().x() - m_pressPosRecord.x()) < 2 && qAbs(event->pos().y() - m_pressPosRecord.y()) < 2) {
        return;
    }

    if (!m_mousePress) {
        return;
    }
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }

    QPoint pos = this->mapToGlobal(event->pos());

    //处理触摸屏-拖动后，触摸释放信号不被发出问题-如果是触摸
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果没有正在捕获鼠标
        if (!this->mouseGrabber()) {
            //抓取鼠标
            this->grabMouse();
            //进行释放--避免是触摸的时候，事件一直无法被释放，无法进行后续拖拽点击等事件
            this->releaseMouse();
        }
    }
    // XEvent xEvent;
    // memset(&xEvent, 0, sizeof(XEvent));

    // Display *display = QX11Info::display();
    // xEvent.xclient.type = ClientMessage;
    // xEvent.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    // xEvent.xclient.display = display;
    // xEvent.xclient.window = (XID)(this->winId());
    // xEvent.xclient.format = 32;
    // xEvent.xclient.data.l[0] = pos.x() * device;
    // xEvent.xclient.data.l[1] = pos.y() * device;
    // xEvent.xclient.data.l[2] = 8;
    // xEvent.xclient.data.l[3] = Button1;
    // xEvent.xclient.data.l[4] = 1;

    // XUngrabPointer(display, CurrentTime);
    // XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()), False,
    //            SubstructureNotifyMask | SubstructureRedirectMask, &xEvent);
    // XFlush(display);
    event->accept();

#endif
}

void MainWindow::changeContinuePauseState(bool isContinue)
{
    if (isContinue) {
        m_controlPlayAndPauseLayout->removeWidget(m_continueButton);
        m_continueButton->setParent(NULL);
        m_controlPlayAndPauseLayout->insertWidget(1, m_pauseButton);
        m_pauseButton->setToolTip(tr("Pause Space"));
    } else {
        m_controlPlayAndPauseLayout->removeWidget(m_pauseButton);
        m_pauseButton->setParent(NULL);
        m_controlPlayAndPauseLayout->insertWidget(1, m_continueButton);
        m_continueButton->setToolTip(tr("Start Space"));
    }
}

//开始和暂停
void MainWindow::play_pause_clicked()
{
    static QTime pauseTime;
    if (m_stratOrPause) {

        if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
            QMessageBox::warning(this, tr("Warning"), tr("Audio is playing, please stop and record again!"));
            return;
        }
        m_isRecording = true; //开始时正在录音的标记值为true,其为true时禁止Item的悬浮特效
        changeContinuePauseState(m_isRecording);
        m_miniWindow.changeMiniContinuePauseState(m_isRecording);
        m_miniWindow.changeMiniCoutinuePauseBtnThemeColor(m_themeData->get("style-name").toString());
        emit playRecord();

        QTime base_time(0, 0, 0);
        int run_time = this->get_sys_runtime(1);
        m_diffValueTime = base_time.addSecs(run_time);    //记录开始时的时间
        int t = pauseTime.secsTo(m_diffValueTime); //点击暂停时时间与点击恢复计时的时间差值
        m_baseTime = m_baseTime.addSecs(t);
        m_miniWindow.m_miniBaseTime = m_miniWindow.m_miniBaseTime.addSecs(t);
        m_miniWindow.m_miniStartOrPause = false;
        m_stratOrPause = false;
    } else {
        qDebug() << "pause录音";
        m_isRecording = false; //暂停时正在录音的标记值为false,其为false时Item的悬浮特效可以被开启
        changeContinuePauseState(m_isRecording);
        m_miniWindow.changeMiniContinuePauseState(m_isRecording);
        m_miniWindow.changeMiniCoutinuePauseBtnThemeColor(m_themeData->get("style-name").toString());

        emit pauseRecord();
        QTime base_time(0, 0, 0);
        int run_time = this->get_sys_runtime(1);
        pauseTime = base_time.addSecs(run_time); //记录一下当前暂停时的时间
        qDebug() << pauseTime;
        m_miniWindow.m_miniStartOrPause = true;
        m_stratOrPause = true;
    }
}


void MainWindow::recordPaint(int value)
{
    m_maxNum.prepend(value / 100); //将元素插入到Vector的开始
    m_maxNum = m_maxNum.mid(0, rectangleCount);
    for (int i = 0; i < rectangleCount; i++)
        m_customWave.at(i)->setValue(m_maxNum.at(i));
}

void MainWindow::stop_clicked() //停止按钮
{
    m_tipWindow->tipLabel->setText(tr("Generating..."));
    m_tipWindow->setFixedWidth(198);
    m_tipWindow->tipLabel->setFixedWidth(198);
    //m_tipWindow->setWindowFlags(Qt::FramelessWindowHint);//不显示窗口阴影

    m_tipWindow->show();
    m_tipWindow->move(this->geometry().center() - m_tipWindow->rect().center());
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
    QTimer::singleShot(100, this, [](){qDebug() << "生成弹窗" ;});

    if (m_finish) {
        m_isRecording = false; //停止录音时此值为false,其为false时Item的悬浮特效可以被开启
        while(1){
            QString time = m_showTimeLabel->text();
            QStringList strs = time.split(":");
            int length = (QString(strs[0]).toInt() * 3600) + (QString(strs[1]).toInt() * 60) + (QString(strs[2]).toInt());
            if(length > 1){
                emit stopRecord();
                break;
            }else{
                QTimer::singleShot(1000, this, [this](){emit stopRecord();});
                break;
            }
        }
        removeFlagLayoutWidget();
        m_delayTimer->stop();
        if (m_stratOrPause) {
            m_stratOrPause = false;
            m_miniWindow.m_miniStartOrPause = false;
        }
        //停止按钮触发后的界面
        m_showTimeLabel->setText("00:00:00");              //主界面计时停止
        m_miniWindow.m_miniTimeLabel->setText("00:00:00"); // mini模式计时停止
        QString systemFont = m_themeData->get("systemFont").toString();
        int systemFontSize = m_themeData->get("systemFontSize").toInt();
        QFont fontStyle(systemFont, systemFontSize);
        qDebug() << "systemFont" << systemFont << "systemFontSize" << systemFontSize;
        emit itemFontStyleSignal(fontStyle); //改变FileItem字体
        m_finish = false;
        if (m_pStackedWidget->currentIndex() == 0) {
            changePage(0);
        } else if (m_pStackedWidget->currentIndex() == 2) {
            changePage(2);
        }
        m_miniWindow.m_recordStackedWidget->setCurrentIndex(0);
        m_timeTag = 0; //只有在停止时让计时归零
    }
 }

void MainWindow::updateDisplay()
{
    QTime showTime(0, 0, 0);
    showTime = showTime.addMSecs(m_recordDurationMs);
    this->m_timeStr = showTime.toString("hh:mm:ss");
    //    qDebug()<<"秒："<<showTime.second();
    m_timeTag = showTime.minute();
    m_showTimeLabel->setText(m_timeStr);
    m_miniWindow.m_miniTimeLabel->setText(m_timeStr);
    if (m_recordDurationMs >= m_nextRecordSplitMs) {
        m_nextRecordSplitMs += 21600000;
//    if(seconds % 10 == 0 && seconds != 0){ //测试时用
        emit reRecordSignla();
    }
}

void MainWindow::updateRecordDuration(qint64 durationUs)
{
    if(!m_isRecording && !m_stratOrPause){
        return;
    }
    m_recordDurationMs = durationUs / 1000;
    updateDisplay();
}

void MainWindow::mainWindow_page2()
{
    m_recordDurationMs = 0;
    m_nextRecordSplitMs = 21600000;
    m_maxNum.clear();
    for (int i = 0; i < rectangleCount; i++)
        m_maxNum.append(0);
    m_pauseButton->setEnabled(true); //按下后，开始录音可以按暂停
    m_miniWindow.changeMiniCoutinuePauseBtnThemeColor(m_themeData->get("style-name").toString());

    m_showTimeLabel->setText("00:00:00");
    m_showTimeLabel->setStyleSheet("font: bold; font-size:30px;");

    m_showTimelbLayout->addWidget(m_showTimeLabel, 0, Qt::AlignCenter); //在布局的中央
    m_showTimelbWid->setLayout(m_showTimelbLayout);
    m_showTimelbWid->setFixedHeight(45);
    m_monitorWaveLayout = new QHBoxLayout(); //波形图所在的布局
    for (int i = 0; i < rectangleCount; i++) //频率直方图
    {
        myWave *wave = new myWave(false, this); //每次都要初始化一个矩形框
        wave->setAccessibleName("kylin-recorder_mainwindow_myWave_wave");
                                                //        wave=new myWave(this);
                                                //        wave->setMaximumHeight(100);
        wave->setRange(0, 100);
        m_customWave.push_back(wave);
        m_monitorWaveLayout->addWidget(wave);
    }
    m_monitorWaveLayout->setSpacing(2);
    m_waveWidget->setLayout(m_monitorWaveLayout);
    m_waveWidget->setFixedSize(304, 100);

    m_controlPlayAndPauseLayout->addWidget(m_finishButton);
    m_controlPlayAndPauseLayout->addWidget(m_pauseButton);
    //    controlPlayAndPauseLayout->removeWidget(continueButton);//切换至录音时要先去除continueButton所在wid
    m_controlPlayAndPauseLayout->addWidget(m_flagButton);
    m_controlPlayAndPauseWid->setLayout(m_controlPlayAndPauseLayout);
    m_controlPlayAndPauseWid->setFixedSize(304, 64);

    flagLayout->addWidget(leftButton);
    flagLayout->addWidget(elementWidget);
    flagLayout->addWidget(rightButton);
    flagWidget->setLayout(flagLayout);
    m_pageTwoLayout->addWidget(m_showTimelbWid, 0, Qt::AlignCenter);
    m_pageTwoLayout->addWidget(m_waveWidget, 0, Qt::AlignCenter);
    m_pageTwoLayout->addWidget(m_controlPlayAndPauseWid, 0, Qt::AlignCenter);
    m_pageTwoLayout->addWidget(flagWidget, 0, Qt::AlignCenter);
    m_pageTwoLayout->setContentsMargins(0, 0, 0, 0);
    //    m_pageTwoLayout->setContentsMargins(248, 60, 248, 108);

    m_pageTwoWid->setLayout(m_pageTwoLayout);
}

void MainWindow::changePage(int no)
{
    switch(no){
    case 0:
        m_pStackedWidget->setCurrentIndex(no);
        m_titleRightWidget->setAutoFillBackground(false);
        break;
    case 1:
        m_pStackedWidget->setCurrentIndex(no);
        m_titleRightWidget->setAutoFillBackground(false);
        break;
    case 2:
        m_pStackedWidget->setCurrentIndex(no);
        m_titleRightWidget->setAutoFillBackground(true);
        break;
    default:
        qDebug () << "failed to check page no"<<no;
        break;
    }
}

QStringList MainWindow::getFilePathList()
{
    return m_filePathList;
}

void MainWindow::clearFilePathList()
{
    m_filePathList.clear();
}

void MainWindow::removePathInFilePathList(QString path)
{
    m_filePathList.removeOne(path);
}
long MainWindow::get_sys_runtime(int type)
{
    struct timespec times = {0, 0};
    long time;

    clock_gettime(CLOCK_MONOTONIC_RAW, &times);
    // 根据需求反馈秒级和毫秒级时间
    if (1 == type){
        time = times.tv_sec;
    }else{
        time = times.tv_sec * 1000 + times.tv_nsec / 1000000;
    }
    return time;
}

//与堆叠布局搭配使用效果更佳
void MainWindow::switchPage()
{
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Record", (char *)"mainwindow");
    kdk_dia_data_free(node);
    qDebug() << "播放状态*********************" << PlayControl::getInstance().getAudioState();
    if (PlayControl::getInstance().getAudioState()
        != QMediaPlayer::PlayingState) //判断是否有音频在播放，若无音频播放则可以录音
    {
        m_finish = true; // stop按钮默认是true代表停止中
        m_isRecording = true; //正在录音时此标记为true，此为true时悬浮特效被禁止,这一行一定要在前面
        changePage(1);
        m_miniWindow.m_recordStackedWidget->setCurrentIndex(1); //切换至录音按钮
        /*
         *修复波形图刷新异常问题;
         * */
        for (int i = 0; i < rectangleCount; i++) //频率直方图
        {
            if (m_customWave.at(i) != nullptr) {
                m_customWave.at(i)->setValue(0);
            }
        }
        //if (m_source == 1) {
            //如果是麦克风则弹窗提示
        //    showNotify(tr("A microphone device is being used"));
        //}
        emit startRecord();
        recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderRecording);
        m_delayTimer->start(150); //每隔xms检测一次，波形图缓慢刷新

        changeContinuePauseState(m_isRecording);
        m_miniWindow.changeMiniContinuePauseState(m_isRecording);
        mainWindow_page2(); //必须加
        //刚开始点击按钮时才可以开启定时器
//        QTime base_time(0, 0, 0);
//        int run_time = this->get_sys_runtime(1);
//        m_baseTime = base_time.addSecs(run_time);
//        m_miniWindow.m_miniBaseTime = base_time.addSecs(run_time);
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Audio is playing, please stop and record again!"));

        return;
    }
}

void MainWindow::changeVoicePicture() {}

void MainWindow::goset()
{
    m_setWindow.m_mainWidget->setParent(this, Qt::Tool);
    m_setWindow.m_mainWidget->installEventFilter(this);
    m_setWindow.m_mainWidget->show();
    m_setWindow.m_mainWidget->move(this->geometry().center() - m_setWindow.m_mainWidget->rect().center());
    //    if(isRecording||strat_pause)
    //    {
    //        qDebug()<<"可以结束";
    //        stop_clicked();
    //    }
    if (m_isRecording || m_stratOrPause) {
        m_setWindow.m_alterButton->setEnabled(false);
        m_setWindow.m_formatDownList->setEnabled(false);
        m_setWindow.m_sourceDownList->setEnabled(false);

    } else {
        m_setWindow.m_alterButton->setEnabled(true);
        m_setWindow.m_formatDownList->setEnabled(true);
        m_setWindow.m_sourceDownList->setEnabled(true);
    }
}

void MainWindow::updateGsetting_ListWidget() //初始化时配置文件刷新出,传""时为应用开始时的刷新
{
    qDebug() << "刷新文件列表";
    m_list->clear();
    m_filePathList.clear();
    QStringList filters;
    filters << "*.raw";
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QDir tempRawFileDir = str + "/.cache";
    QFileInfoList rawFileInfoList;
    rawFileInfoList << tempRawFileDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    QString oldName;
    QString newName;
    QString rawFilePath;
    QString outputFilePath;
    QString type = "mp3";
    if (m_defaultPathData->get("type").toInt() == 1) {
        type = "mp3";
    } else if (m_defaultPathData->get("type").toInt() == 2) {
        type = "m4a";
    } else {
        type = "wav";
    }

    if (!rawFileInfoList.isEmpty()) {
        rawFilePath = rawFileInfoList.at(0).absoluteFilePath();
        oldName = rawFileInfoList.at(0).absoluteFilePath().split("/").last();
        int pos = oldName.indexOf(".");
        newName = oldName.replace(pos + 1, 3, type);
        qDebug() << "tempRawFilePath" << rawFileInfoList.at(0).absoluteFilePath() << "newName" << newName;
        QString timeStr =  QDateTime::fromString(newName.mid(0,14), "yyyyMMddhhmmss").toString("yyyy-MM-dd hh:mm:ss");
        outputFilePath = m_defaultPathData->get("path").toString()+ "/" + newName;
        int num = MainWindow::mutual->m_subThread->toConvertWAV(rawFilePath, outputFilePath.toLocal8Bit().data());
        if (num >= 0) {
            qDebug() << "异常退出找到raw,且转码成功!";
            //记录录音文件到数据库
            emit DATABASE_INSTANCE.addRecordSignal(outputFilePath,timeStr);
        }
    }
    QStringList files;
    QStringList times;
    DATABASE_INSTANCE.getAllFiles(files,times);
    if(files.count() != times.count()){
        qDebug() << "Error: database count do not match...";
    }
    int fileCount = 0;
    for(auto i = 0; i < files.count();i++) {
        QFileInfo info(files.at(i));
        if(!info.exists()){
            qDebug () << "File is not exist...";
            continue;
        }

        tm *timeFormat = new tm();
        QDateTime dateTime = QDateTime::fromString(times.at(i), "yyyy-MM-dd hh:mm:ss");
        //qDebug() << ">>>>> " << QDateTime::fromString(times.at(i), "yyyy-MM-dd hh:mm:ss");
        timeFormat->tm_year = dateTime.date().year();
        timeFormat->tm_mon = dateTime.date().month();
        timeFormat->tm_mday = dateTime.date().day();

        timeFormat->tm_hour = dateTime.time().hour();
        timeFormat->tm_min = dateTime.time().minute();
        timeFormat->tm_sec = dateTime.time().second();

        QString d = kdk_system_shortformat_transform(timeFormat);
        d.append(" ").append(dateTime.time().toString("hh:mm:ss"));

        slotListItemAdd(files.at(i), dateTime);
        fileCount ++ ;
    }
    qDebug() << fileCount;
    isFileNull(fileCount);
}

//void MainWindow::slotListItemAdd(QString fileAbsolutePath, QString recordTime)
void MainWindow::slotListItemAdd(QString fileAbsolutePath, QDateTime recordTime) 
{
    QFileInfo fileinfo(fileAbsolutePath);
    m_filePathList << fileinfo.filePath();
    m_fileItem = new FileItem(fileAbsolutePath);
    m_fileItem->setAccessibleName("kylin-recorder_mainwindow_FileItem_m_fileItem");
    if ((fileinfo.suffix() == "mp3" || fileinfo.suffix() == "m4a" || fileinfo.suffix() == "wav") && !fileAbsolutePath.contains("Clip")) {
        m_fileItem->setName_Time_Length(tr("Recording") + fileAbsolutePath.split("/").last(), recordTime,
                                        playerTotalTime(fileAbsolutePath));
    } else if ((fileinfo.suffix() == "mp3" || fileinfo.suffix() == "m4a" || fileinfo.suffix() == "wav") && fileAbsolutePath.contains("Clip")) {
        m_fileItem->setName_Time_Length(fileAbsolutePath.split("/").last(), recordTime,
                                        playerTotalTime(fileAbsolutePath));
    }
    QListWidgetItem *aItem = new QListWidgetItem(m_list); //添加自定义的item
    aItem->setData(Qt::AccessibleTextRole, "kylin-recorder_mainwindow_QListWidgetItem_fileListItem");
    aItem->setSizeHint(QSize(0, m_fileItem->height()));
    m_list->addItem(aItem);
    m_list->setItemWidget(aItem, m_fileItem);
    aItem->setData(Qt::UserRole, fileAbsolutePath);
    isFileNull(m_list->count()); // item个数从0开始增加时文件列表不应该显示"文件列表空"的字样
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_list) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            if (wheelEvent->delta() > 0) {
            } else {
            }
        }
    } else if (obj == this) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_F1) //将event转换为QKeyEvent,然后判断是否键
        {
            qDebug() << "按压了F1";
            if (!m_DaemonIpcDbus->daemonIsNotRunning()) {
                m_DaemonIpcDbus->showGuide("kylin-recorder");
            }
            return true;
        }
    }
    else if (obj == m_setWindow.m_mainWidget){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            m_setWindow.m_mainWidget->close();
        }
    }

    QFocusEvent *  focusEvent = static_cast<QFocusEvent*> (event);
    if (focusEvent->gotFocus()) {
        if (!this->hasFocus()) {
            this->setFocus();
            qDebug() << "mainwindow focus = " << this->hasFocus();
        }
    }
    if (obj == this && (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) || event->type() == QEvent::ActivationChange ||event->type() == QEvent::WindowActivate ||event->type() == QEvent::WindowDeactivate ) {
               if ( event->type() == QEvent::WindowActivate ) {
                   // 窗口激活
                   m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal") ;
                   m_ukuiWindowHelper->setBlurEffect(QRegion(), 0, true);
               } else if(event->type() == QEvent::WindowDeactivate){
                   // 窗口失激活
                   m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive") ;
                   m_ukuiWindowHelper->setBlurEffect(QRegion(), 0, false);
               }
               adjustBackground();
               update();
       }
    return QObject::eventFilter(obj, event);
}

//软件应用使用麦克风时提示正在使用的弹窗
void MainWindow::showNotify(QString message)
{
    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    if (inputDevice.deviceName().contains("multichannel-input")) {
        qDebug() << "多聲道輸入設備";
        m_devMessage = tr("Using multichannel device");
    } else {
        m_devMessage = tr("Recording");
    }
    std::string str = m_devMessage.toStdString();
    const char *ch = str.c_str();
    qDebug() << "彈窗";
    QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications", QDBusConnection::sessionBus());
    QList<QVariant> args;
    QMap <QString,QVariant> hint;
    hint.insert("sound-name","muted");
    args << (tr("Recorder")) << ((unsigned int)0)
         << QString("/usr/share/icons/ukui-icon-theme-default/48x48/apps/kylin-recorder.png")
         << tr(ch)  //显示的是什么类型的信息
         << message //显示的具体信息
         << QStringList() 
	 //<< QVariantMap() 
	 << hint
	 << (int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}

void MainWindow::transChange()
{
    m_effect = m_pGsettingControlTrans->get("effect").toBool();
    m_tran = m_pGsettingControlTrans->get("transparency").toDouble() * 255;
    qDebug () << m_effect <<m_tran;
    this->update();
}

void MainWindow::keySpaceEvent()
{
    // 如果在录音页面，空格键用于暂停/继续录制
    if (m_pStackedWidget->currentIndex() == 1) {
        play_pause_clicked();
        return;
    }
    if(m_pStackedWidget->currentIndex() == 2) {
        qDebug()<< " Space Event";
        QList<FileItem *> List = this->findChildren<FileItem *>();
        for (FileItem *item : List) {
            if (item->m_mouseState && item->currentWidget() != nullptr && item->currentIndex() == 0) {
                qDebug() << "m_playRecorder = " << item->m_playRecorder << " m_pauseState = " << item->m_pauseState;
                if (item->m_playRecorder && !item->m_pauseState)   {
                    item->handlePause();
                } else {
                    item->handlePlay();
                }
                return;
            }

            if (item->m_mouseState && item->currentWidget() != nullptr && item->currentIndex() == 1) {
                qDebug()<< " 剪辑页面---- Key Space " ;
                QList<ClipWidget *> clip_list = item->findChildren<ClipWidget *>("clipWid");
                if (clip_list.isEmpty() ||  clip_list.count() != 1 )
                    return ;

                if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
                    clip_list.first()->pause_ClipFile();
                } else {
                    clip_list.first()->play_ClipFile();
                }
            }
        }
    }
    return;
}


// M 添加标记
void MainWindow::keyMEvent()
{
#ifndef WHITE_TEST
    if(m_pStackedWidget->currentIndex() == 2) {
        qDebug()<< "key M event";
        QList<FileItem *> List = this->findChildren<FileItem *>();
        for (FileItem *item : List) {
            if (item->m_mouseState && item->m_playRecorder && !item->m_pauseState) {
                item->toFlagFile();
                return;
             }
        }
    }

    if (m_pStackedWidget->currentIndex() == 1) {
        // 主界面增加标签的函数
        handleInsertSlot();
    }
#endif
}

// C 剪辑录音
void MainWindow::keyCEvent()
{
#ifndef WHITE_TEST

    if(m_pStackedWidget->currentIndex() == 2) {
        qDebug()<< "key C event";
        QList<FileItem *> List = this->findChildren<FileItem *>();
        for (FileItem *item : List) {
            if (item->m_mouseState) {
                if (item->currentWidget() != nullptr && item->currentIndex() == 0) {
                    item->toClipPage();
                }
            }
        }
    }
#endif
}

// O 在文件夹中显示
void MainWindow::keyOEvent()
{
    if(m_pStackedWidget->currentIndex() == 2) {
        qDebug()<< "key O event";
        QList<FileItem *> List = this->findChildren<FileItem *>();
        for (FileItem *item : List) {
            if (item->m_mouseState) {
                item->actionOpenFolderSlot();
            }
        }
    }
}

void MainWindow::initShortcutStyle()
{
    //不接受焦点。解决程序初始化时，空格键控制 播放/暂停 获取不到（Qt::Key_Space）事件
    QList<QPushButton*> list = this->findChildren<QPushButton*>();

    for(QPushButton *btn :list) {
        btn->setFocusPolicy(Qt::NoFocus);
    }

    QList<QToolButton*> lists = this->findChildren<QToolButton*>();

    for(QToolButton *btn :lists) {
        btn->setFocusPolicy(Qt::NoFocus);
    }
}

//透明度按UI规范修改
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
//    qDebug() << "Paint Event" << palette().color(QPalette::Window);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QPalette pal(QApplication::palette());
    this->setPalette(pal);
    QColor color = palette().color(QPalette::Window);
    if (m_effect)
        color.setAlpha(m_tran);
    else
        color.setAlpha(255); // 不透明
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), m_pSideColor);
    QWidget::paintEvent(event);


    return QWidget::paintEvent(event);
}


void MainWindow::recorderBuriedPoint(kdk::kabase::BuriedPoint::PT pt)
{

}

void MainWindow::updateDateFormat(QString dateFormat)
{
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        m_date_format = dateFormat;
    }
}

void MainWindow::onVolumeDeviceAdjust(int type){

    isDeviceAdjust();
}

void MainWindow::onVolumeDeviceChanged(int type, const QString &portName, const QString &cardName){
    // 仅处理输入设备(type==0)，输出设备变化直接返回
    if (type == 0){
        qInfo()<<"输入设备变化";
        if(isDeviceAdjust()){
            // 存在可用输入设备时，沿用原有逻辑刷新UI（含 monitor 设备的置灰/恢复）
            qInfo()<<"设备未被禁用情况下判断耳机插拔";
            inputDevice_get(portName);
        }
    }
    else if (type == 1) {
        qInfo() << "输出设备变化";
        if(m_source == 2){
            if (m_pStackedWidget->currentIndex() == 1) {
                m_finish = true;
                stop_clicked();
            }
        }
        return;
    }
}

void MainWindow::onAudioInputError(QAudio::Error error){
    qInfo()<<"进入了麦克风禁用弹窗逻辑";
    stop_clicked();
    if(m_pStackedWidget->currentIndex() == 1){
        changePage(2);
    }
    QMessageBox::warning(this, tr("Warning"), tr("The microphone has been disabled,please go to the Security Center to turn it on!"));
}

bool MainWindow::isDeviceAdjust(){
    if(!QGSettings::isSchemaInstalled(KYLINRECORDER)){
        qInfo()<<"Gsetting对象中没有KYLINRECORDER这个schema";
        return false;
    }
    QGSettings m_Data(KYLINRECORDER);
    int sourceType = m_Data.get("source").toInt();
    if(sourceType == 2){
        qInfo()<<"录音来源为系统内部，跳过输入设备检测逻辑";
        return false;
    }

    // 使用声音模块提供的 isValidDevice(0) 判断是否存在有效输入设备
    QDBusInterface iface("org.ukui.volume.control",
                         "/org/ukui/volume/control",
                         "org.ukui.volume.control",
                         QDBusConnection::sessionBus());
    QDBusReply<bool> rep = iface.call("isValidDevice", 0);
    bool hasInput = rep.value();

    if (!hasInput) {
        qInfo() << "无可用输入设备";
        // 若当前在录音页，先安全停止
        if (m_pStackedWidget->currentIndex() == 1) {
            m_finish = true;
            stop_clicked();
        }

        // 在按钮状态被修改前，记录当前的使能状态
        bool wasEnabled = m_firstRecordButton->isEnabled();

        // 置灰所有录音入口（主窗体与 mini 窗）
        m_firstRecordButton->setEnabled(false);
        m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

        m_thirdRecordButton->setEnabled(false);
        m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-gray.svg);}");

        m_miniWindow.m_miniRecordButton->setEnabled(false);
        m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-gray.svg);}");

        // 判断如果之前是未置灰状态，才触发弹窗提示
        if (wasEnabled) {
            // 明确提示“未接入输入设备”
            if (!m_warnBox) {
                m_warnBox = new QMessageBox(QMessageBox::Warning,
                                          tr("Warning"),
                                          tr("No input device is connected!"),
                                          QMessageBox::Ok,
                                          this);
                m_warnBox->setAttribute(Qt::WA_DeleteOnClose, true);
                QObject::connect(m_warnBox, &QMessageBox::destroyed, [&]() {
                    m_warnBox = nullptr;
                });
                m_warnBox->show();
            } else if (m_warnBox->isVisible()) {
                m_warnBox->raise();
                m_warnBox->activateWindow();
            }
        }
        return false;
    } else {
        // 当设备重新变为可用时，恢复 UI 状态
        qInfo() << "输入设备恢复可用，恢复录音按钮";

        m_firstRecordButton->setEnabled(true);
        m_firstRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

        m_thirdRecordButton->setEnabled(true);
        m_thirdRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/rec-default.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/rec-hover.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/rec-click.svg);}");

        m_miniWindow.m_miniRecordButton->setEnabled(true);
        m_miniWindow.m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");

        return true;
    }
}

void MainWindow::adjustBackground(){
    QWidget *mainWindow = this->window();
    setAutoFillBackground(true);
    if(mainWindow->isActiveWindow())
    {
        m_pSideColor.setAlphaF(tranSparency);
    }
    update();
}

void MainWindow::blurEffect(){
    m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal");
    this->setAttribute(Qt::WA_TranslucentBackground);
    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_pTransparency = new QGSettings("org.ukui.control-center.personalise");
        tranSparency = m_pTransparency->get("transparency").toDouble();
        connect(m_pTransparency, &QGSettings::changed, this, [=](QString value) {
            if (value == "transparency") {
                tranSparency = m_pTransparency->get("transparency").toDouble();
                adjustBackground();
            }
        });
    }
    adjustBackground();
    setAutoFillBackground(true);
    if (ThemeController::widgetTheme() != ClassicTheme) {
        this->setAttribute(Qt::WA_TranslucentBackground, true); // 透明
    }
    installEventFilter(this);
}
