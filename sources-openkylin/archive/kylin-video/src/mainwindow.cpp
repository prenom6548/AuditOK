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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "mainwindow.h"

#include <QEvent>
#include <QTimer>
#include <QWindow>
#include <QThread>
#include <QProcess>
#include <QScreen>
#include <QMimeData>
#include <QDBusReply>
#include <QApplication>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QDBusConnection>
#include <QFileSystemWatcher>
#include <QWindowStateChangeEvent>
#include <QMessageBox>

#include <KF5/KWindowSystem/kwindowsystem.h>
#include <KWindowEffects>
#include "core/dbusadapter.h"
#include "widget/kmenu.h"
#include "widget/homepage.h"
#include "widget/contralbar.h"
#include "widget/musicwidget.h"
#include "widget/titlewidget.h"
#include "widget/playxwidget.h"
#include "widget/playglwidget.h"
#include "widget/urleditwidget.h"
#include "widget/minimodeshade.h"
#include "widget/systemtrayicon.h"
#include "widget/playlistwidget.h"
#include "widget/mediainfodialog.h"
#include "widget/setup/setupdialog.h"

#include "global/global.h"
#include "global/functions.h"
#include "global/translator.h"
#include "global/extensions.h"
#include "global/globalconfig.h"

#include <kaboutdialog.h>
#include <ukui-log4qt.h>
#include <fcntl.h>

using namespace kdk;
using namespace Global;

MainWindow::MainWindow(QStringList list, QWidget *parent) :
    QWidget(parent),
    m_exit_flag(false),
    m_is_mini_mode(false),
    m_is_maximized(false),
    m_is_fullscreen(false),
    m_stacked_widget(nullptr),
    m_home_page(nullptr),
    m_title_menu(nullptr),
    m_title_widget(nullptr),
    m_music_widget(nullptr),
    m_playlist_widget(nullptr),
    m_mini_mode_shade(nullptr),
    m_can_hide_all(true),
    m_need_play_when_s34(false),
    m_window_id(0),
    m_inputList(list)
{
    setWindowTitle(tr("Video Player"));
    if (m_inputList.size() > 0) {
        m_needOpenFileWhenStart = true;
    }
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
        if (getpid() == kdk::WindowManager::getPid(window_id) && m_windowId.isEmpty())
        {
            m_windowId = window_id.toString();
            qInfo() << "the windowAdded signal has been received, windowID is" << m_windowId;
        }
    });
    Single(m_inputList);
    m_stacked_widget = new QStackedWidget(this);
    m_stacked_widget->setAccessibleName("kylin-video_QStackedWidget_m_stacked_widget");
    initLayout();
}

MainWindow::~MainWindow()
{

}

void MainWindow::showList()
{

    if (QLocale::system().name() == "ug_CN"
        || QLocale::system().name() == "ky_KG"
        || QLocale::system().name() == "kk_KZ"){
        Global::isHaWeiKe = true;
    }else{
        Global::isHaWeiKe = false;
    }

    setAcceptDrops(true);

    Global::initGlobal();
    g_shortcut->setParent(this);
    g_shortcut->initData();
    {
        double scale = 1.0;
        m_play_widget = new PlayGLWidget(nullptr, scale);
        m_play_widget->setAccessibleName("kylin-video_PlayGLWidget_m_play_widget");
        connect(dynamic_cast<PlayGLWidget*>(m_play_widget), &PlayGLWidget::mousePressed, this, &MainWindow::slotPlayWidgetClicked);
    }
    m_title_widget = new TitleWidget(this);

    initTitleWidget();

    m_music_widget = new MusicWidget(m_play_widget);
    m_music_widget->setAccessibleName("kylin-video_mainwindow_MusicWidget_m_music_widget");
    m_music_widget->installEventFilter(m_play_widget);
    m_music_widget->hide();
    initContralBar();
    initPlayListWidget();
    resetLayout();


    initDBus();
    initMiniModeShade();

    initGlobalSig();
    initCore();
    initMenu();
    initTrayIcon();
    QTimer::singleShot(1000, [this]() {
        kdk_dia_upload_program_startup();
    });

    m_home_page = new HomePage;
    m_home_page->setAccessibleName("kylin-video_mainwindow_HomePage_m_home_page");
    m_stacked_widget->addWidget(m_home_page);
    m_stacked_widget->addWidget(m_play_widget);
 
    if (!m_needOpenFileWhenStart) {
        m_stacked_widget->setCurrentWidget(m_home_page);
    } else {
        m_stacked_widget->setCurrentWidget(m_play_widget);
    }
    m_stacked_widget->removeWidget(m_initWidget);

    // if (!Functions::isVirtualSuppotGpu() && GlobalConfig::getInstance()->hardwareType() != GlobalConfig::Nvidia_VDPAU_COPY)
    //     qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
    if (m_inputList.size() > 0)
    {
        if (m_inputList.at(0).startsWith("http"))
            slotPlayFile(m_inputList.at(0), 0);
        else
            g_user_signal->addFiles(m_inputList);
    }
    else {
        // 播放宣传片
        QString pf = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation).append("/银河麒麟操作系统V10-sp1宣传视频.mp4");
        QFileInfo fi(pf);
        if (fi.exists())
            g_user_signal->addFiles(QStringList() << pf);
    }
}

void MainWindow::kylin_video_play_request(QStringList file_list)
{
    // 此处相当于双击打开，需要active一下
    if (isHidden() || !isActiveWindow()) {
        kdk::UkuiStyleHelper::self()->removeHeader(this);
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
        kdk::WindowManager::activateWindow(m_windowId);
    }

    if (file_list.size() > 0)
    {
        // 播放
        if (file_list.at(0).startsWith("http"))
            slotPlayFile(file_list.at(0), 0);
        else
            g_user_signal->addFiles(file_list);
    }
}

void MainWindow::onPrepareForSleep(bool isSleep)
{
    if (isSleep) {
        if (g_playstate > 0) {
            if(g_config->keepStateWhenWakeup.first && g_playstate == Mpv::Playing)
                m_need_play_when_s34 = true;
            m_core->Pause();
        }
    }
    else {
        // 唤醒之后要重新，不然显示会有问题，共性问题，此方法规避（引入问题有点多，只对有问题显卡进行处理，但是现在没有问题的列表，遇到之后再做处理）
        if (true) {

        }
        else {
            m_core->Restart();
        }
        if (m_need_play_when_s34) {
            // 判断是否锁屏了，如果锁屏等解锁后再播放
            if(m_is_screen_locked)
                return;
            if (true) {
                m_core->Play();
            }
            else {
                // 跳转之后才能继续播放,不然跳转之后就又暂停了
                QThread::create([this]{
                    int try_times = 9;
                    while (!m_core->getRestartState() || try_times--) {
                        QThread::msleep(200);
                    }
                    m_core->Play();
                })->start();
            }
            m_need_play_when_s34 = false;
        }
    }
}

void MainWindow::onScreenLock()
{
    if (m_is_screen_locked)
        return;
    m_is_screen_locked = true;
    // 只有 wayland 才需要如此
    g_shortcut->makeAllInvalid();
    if(g_playstate == Mpv::Playing && m_need_play_when_s34 == false)
    {
        m_core->Pause();
        if(g_config->keepStateWhenWakeup.first)
            m_need_play_when_s34 = true;
    }
}

void MainWindow::onScreenUnlock()
{
    m_is_screen_locked = false;
    g_shortcut->makeAllValid();
    if(m_need_play_when_s34)
    {
        m_core->Play();
        if(g_config->keepStateWhenWakeup.first)
            m_need_play_when_s34 = false;
    }
}

void MainWindow::onLidWatcherMessage()
{
    QGSettings power_setting("org.ukui.power-manager");
    if (power_setting.keys().contains("button-lid-battery") || power_setting.get("button-lid-battery").toString() == "nothing")
        return;

    KyInfo() << "[Lid " << QDateTime::currentDateTime() << "]" << " receive PropertiesChanged ****************************";
    QDBusInterface iface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "LidIsClosed");
    if (reply.isValid()) {
        // true 关屏、false 开屏
        if (reply.value().toBool())
        {
            KyInfo() << "[Lid " << QDateTime::currentDateTime() << "]" << " close, screen lock : " << m_is_screen_locked;
            g_shortcut->makeAllInvalid();
            if(g_playstate == Mpv::Playing && m_need_play_when_s34 == false)
            {
                m_core->Pause();
                if(g_config->keepStateWhenWakeup.first)
                    m_need_play_when_s34 = true;
            }
        }
        else
        {
            KyInfo() << "[Lid " << QDateTime::currentDateTime() << "]" << " open, screen lock : " << m_is_screen_locked;
            if (!m_is_screen_locked)
            {
                onScreenUnlock();
            }
        }
    } else {
        KyInfo() << "[Lid " << QDateTime::currentDateTime() << "]" << " Failed to get lid closed event!";
    }
}

void MainWindow::onWiredControl(QString data)
{
    int key = data.split(':').first().toInt();
    int num = data.split(':').last().toInt();
    if(num == 1)
        g_user_signal->play_pause();
    else if(num == 2 || key == 163)
        g_user_signal->playNext(true);
    else if(num == 3 || key == 165)
        g_user_signal->playPrev(true);
}

void MainWindow::slotPrepareForSwitchuser()
{
    if (m_core)
        m_core->Pause();
}

void MainWindow::inputDeviceGet(QString device)
{
    // 接收到耳机插拔信号就暂停
    KyInfo() << "playState : " << g_playstate << ", out device change " << device;
    if(g_playstate == Mpv::Playing) {
        m_core->Pause();
    }
}

void MainWindow::uiModeChange(bool isTabletMode)
{
    // ui 模式改变（电脑模式/平板模式）
    Global::isTablet = isTabletMode;
    m_contral_bar->setUIMode(isTabletMode);
    m_title_widget->setUIMode(isTabletMode);
}

void MainWindow::initLayout()
{
    setMinimumSize(NormalModeSize);
    resize(NormalModeSize);
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    setMouseTracking(true);

    m_initWidget = new QWidget(this);
    // 关键：让widget自动填充父容器
    m_initWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 使用中央布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_initWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 去除边距

    QLabel *imageLabel = new QLabel(m_initWidget);
    imageLabel->setFixedSize(100, 100);
    QPixmap pixmap(QIcon::fromTheme("kylin-video").pixmap(imageLabel->size()));


    imageLabel->setAlignment(Qt::AlignCenter); // 标签内容居中
    imageLabel->setPixmap(pixmap);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(imageLabel);

    mainLayout->addLayout(layout);
    m_initWidget->setLayout(mainLayout);

    // 确保堆叠窗口占满主窗口
    if (m_stacked_widget) {
        // 如果m_stacked_widget没有布局，给它设置一个
        if (!m_stacked_widget->layout()) {
            QVBoxLayout *stackLayout = new QVBoxLayout(m_stacked_widget);
            stackLayout->setContentsMargins(0, 0, 0, 0);
        }
        m_stacked_widget->addWidget(m_initWidget);
        m_stacked_widget->setCurrentIndex(0);
        // 让堆叠窗口也填充父容器
        m_stacked_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    // 最后确保主窗口有布局来管理m_stacked_widget
    if (!this->layout()) {
        QVBoxLayout *mainWindowLayout = new QVBoxLayout(this);
        mainWindowLayout->setContentsMargins(0, 0, 0, 0);
        mainWindowLayout->addWidget(m_stacked_widget);
    }
}

void MainWindow::initTrayIcon()
{
    m_tray_icon = new SystemTrayIcon(this);
    m_tray_icon->setObjectName("kylin-video_SystemTrayIcon_m_tray_icon");
    connect(m_tray_icon, &SystemTrayIcon::sigQuit, this, &MainWindow::appQuit);
    connect(m_tray_icon, &SystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason){
        switch (reason) {
        case QSystemTrayIcon::Trigger:
            if (isHidden() || !isActiveWindow()) {
                kdk::UkuiStyleHelper::self()->removeHeader(this);
                setWindowState(windowState() & ~Qt::WindowMinimized);
                show();
                kdk::WindowManager::activateWindow(m_windowId);
            }
            break;
        default:
            break;
        }
    });
    m_tray_icon->show();
}

void MainWindow::initDBus()
{
    m_dbus_interface = new QDBusInterface("org.gnome.SessionManager",
                                          "/org/gnome/SessionManager",
                                          "org.gnome.SessionManager",
                                          QDBusConnection::sessionBus());
    m_dbus_interface->setObjectName("kylin-video_QDBusInterface_m_dbus_interface");

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(sessionBus.registerService("org.ukui.kylinvideo"))
    {
        sessionBus.registerObject("/org/ukui/kylinvideo",this,
                                  QDBusConnection::ExportAllContents);
        KyInfo() << "初始化DBUS成功";
    }
    else
    {
        KyInfo("init dbus error");
    }

    QDBusMessage m = QDBusMessage::createMethodCall(QString("com.kylin.statusmanager.interface"),
                                                    QString("/"),
                                                    QString("com.kylin.statusmanager.interface"),
                                                    QString("get_current_tabletmode"));

    QDBusMessage res = sessionBus.call(m);
    if (res.type() == 1 || res.type() == 2)
        m_is_tablet_mode = res.arguments().first().toBool();
    else
        m_is_tablet_mode = false;

    //S3 S4策略
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForShutdown"), this,
                                         SLOT(onPrepareForSleep(bool)));
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));

    // 锁屏
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                          QString("/"),
                                          QString("org.ukui.ScreenSaver"),
                                          QString("lock"), this,
                                          SLOT(onScreenLock()));
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                          QString("/"),
                                          QString("org.ukui.ScreenSaver"),
                                          QString("unlock"), this,
                                          SLOT(onScreenUnlock()));

    // 关屏
    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         "/org/freedesktop/UPower",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onLidWatcherMessage(void)));

    // 耳机插拔
    QDBusConnection::sessionBus().connect(QString(""),
                                          QString("/"),
                                          QString("org.ukui.media"),
                                          QString("sinkPortChanged"), this,
                                          SLOT(inputDeviceGet(QString)));

    // 线控
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/"),
                                         QString("com.monitorkey.interface"),
                                         QString("monitorkey"), this,
                                         SLOT(onWiredControl(QString)));

    //切换用户
    QDBusConnection::sessionBus().connect(QString("org.gnome.SessionManager"),
                                          QString("/org/gnome/SessionManager"),
                                          QString("org.gnome.SessionManager"),
                                          QString("PrepareForSwitchuser"), this,
                                          SLOT(slotPrepareForSwitchuser()));

    //切换模式
    QDBusConnection::sessionBus().connect(QString("com.kylin.statusmanager.interface"),
                                          QString("/"),
                                          QString("com.kylin.statusmanager.interface"),
                                          QString("mode_change_signal"), this,
                                          SLOT(uiModeChange(bool)));

    // 因为重写了 closeEvent，影响注销
    QGuiApplication::setFallbackSessionManagementEnabled(false);

    DbusAdapter *dbs_adapter = new DbusAdapter;

}

void MainWindow::initMenu()
{
    m_right_click_menu = new KRightClickMenu(this);
    m_right_click_menu->setAccessibleName("kylin-video_KRightClickMenu_m_right_click_menu");

    connect(m_right_click_menu, SIGNAL(sigOpenFile()), this, SLOT(slotOpenFile()));
    connect(m_right_click_menu, SIGNAL(sigOpenDir()), this, SLOT(slotOpenDir()));
    connect(m_right_click_menu, SIGNAL(sigOpenUrl()), this, SLOT(slotOpenUrl()));
    connect(m_right_click_menu, SIGNAL(sigToTop(bool)), this, SLOT(slotToTop(bool)));
    connect(m_right_click_menu, SIGNAL(sigPlayOrder(int)),this, SLOT(slotPlayOrder(int)));
    connect(m_right_click_menu, &KRightClickMenu::sigDefaultFrame, [this](){
        m_video_aspect = Mpv::AUTO;
    });
    connect(m_right_click_menu, &KRightClickMenu::sig4Div3Frame, [this](){
        m_video_aspect = Mpv::DIV_4_3;
    });
    connect(m_right_click_menu, &KRightClickMenu::sig16Div9Frame, [this](){
        m_video_aspect = Mpv::DIV_16_9;
    });
    connect(m_right_click_menu, &KRightClickMenu::sigFullFrame, [this](){
        m_video_aspect = Mpv::FULL;
        //        updateAspect();
    });

    connect(m_right_click_menu, &KRightClickMenu::sigRestoreFrame, [this](){
        m_video_aspect = Mpv::AUTO;
    });

    connect(m_right_click_menu, &KRightClickMenu::sigAlongRotate, [this](){
        // 旋转后需要判断是否时满屏画面，如果是的话需要改变比例
        //        if(m_video_aspect == Mpv::FULL)
        //            updateAspect();
    });
    connect(m_right_click_menu, &KRightClickMenu::sigInverseRotate, [this](){
        //        if(m_video_aspect == Mpv::FULL)
        //            updateAspect();
    });
    connect(m_right_click_menu, &KRightClickMenu::sigMediaInfo, [this](){
        if(g_playstate < 0)
            return;
        QWidget* show_parent = nullptr;
        if (isWayland)
            show_parent = this;
        // 媒体信息弹窗
        if (!m_mediaInfoDialog) {
            m_mediaInfoDialog = new MediaInfoDialog(show_parent);
        }
        m_mediaInfoDialog->setModal(true);
        m_mediaInfoDialog->setTitle(m_file_info.media_title);
        m_mediaInfoDialog->setType(m_file_info.file_type);
        m_mediaInfoDialog->setSize(m_file_info.file_size);
        m_mediaInfoDialog->setDuration(Functions::timeToStr(m_file_info.length, false));
        m_mediaInfoDialog->setPath(m_file_info.file_path);
        m_mediaInfoDialog->updateDate();
        connect(g_core_signal, &GlobalCoreSignal::sigFileInfoChange, m_mediaInfoDialog, [this](Mpv::FileInfo file_info){
            m_file_info = file_info;
            //            m_mediaInfoDialog->setData(m_core->getMediaInfo());
            m_mediaInfoDialog->setTitle(m_file_info.media_title);
            m_mediaInfoDialog->setType(m_file_info.file_type);
            m_mediaInfoDialog->setSize(m_file_info.file_size);
            m_mediaInfoDialog->setDuration(Functions::timeToStr(m_file_info.length, false));
            m_mediaInfoDialog->setPath(m_file_info.file_path);
            m_mediaInfoDialog->updateDate();
        });
        kdk::UkuiStyleHelper::self()->removeHeader(m_mediaInfoDialog);
//        KWindowSystem::setState(m_mediaInfoDialog->winId(), NET::SkipPager | NET::SkipSwitcher);
//        m_mediaInfoDialog->move(geometry().center() - m_mediaInfoDialog->geometry().center());
        m_mediaInfoDialog->show();
        m_mediaInfoDialog->exec();
    });
}

void MainWindow::initCore()
{
    m_core = new MpvCore(m_play_widget);
    m_core->setObjectName("kylin-video_MpvcCore_m_core");
    connect(m_core, &MpvCore::sigShowText, m_music_widget, &MusicWidget::showOsdText);
}

void MainWindow::initTitleWidget()
{
    m_title_menu = new TitleMenu;
    m_title_menu->setAccessibleName("kylin-video_TitleMenu_m_title_menu");
    connect(m_title_menu, &TitleMenu::sigQuit, this, &MainWindow::slotQuit);

    m_title_widget->setAccessibleName("kylin-video_TitleWidget_m_title_widget");
    m_title_widget->setUIMode(m_is_tablet_mode);
    m_title_widget->show();
    m_title_widget->move(0, 0);
    m_title_widget->setTitle(tr("Video Player"));

    connect(m_title_widget, &TitleWidget::sigMaxSize,    this, &MainWindow::slotChangeMaxState);
    connect(m_title_widget, &TitleWidget::sigMiniSize,   this, &MainWindow::slotChangeMiniState);
    connect(m_title_widget, &TitleWidget::sigClose,      this, &MainWindow::slotClose);
    connect(m_title_widget, &TitleWidget::sigShowMenu,   this, &MainWindow::slotShowTitleMenu);
    connect(m_title_widget, &TitleWidget::sigMiniMode,   this, &MainWindow::slotShowMiniMode);
    connect(m_title_widget, &TitleWidget::sigCanHide, [this](bool canHide){m_can_hide_all = canHide;});
    connect(this, &MainWindow::windowActivated, m_title_widget, &TitleWidget::onWindowActivated);
    connect(this, &MainWindow::windowDeactivated, m_title_widget, &TitleWidget::onWindowDeactivated);
}

void MainWindow::initPlayListWidget()
{
    m_playlist_widget = new PlayListWidget(this);
    m_playlist_widget->setAccessibleName("kylin-video_PlayListWidget_m_playlist_widget");
    m_playlist_widget->show();
    m_playlist_widget->move(width()-16, 0);

    // 点清理痕迹调用清空列表函数
    connect(m_title_menu, &TitleMenu::sigClearList, m_playlist_widget, &PlayListWidget::clearList);

    // 标题栏跟随列表展开改变大小
    if (Global::isHaWeiKe){
        connect(m_playlist_widget, &PlayListWidget::sigMove, this, [this](int distance){
            m_title_widget->setGeometry(distance, 0, width() - distance, m_title_widget->height());
        });
    }else{
        connect(m_playlist_widget, &PlayListWidget::sigMove, this, [this](int distance){
            m_title_widget->setGeometry(0, 0, width() - distance, m_title_widget->height());
        });
    }

    // 列表显示的时候隐藏标题栏和控制栏
    connect(g_user_signal, &GlobalUserSignal::sigShowPlayList, [&](){
        if(m_stacked_widget->currentIndex() == 1) {
            // 防止列表弹出的时候鼠标在播放界面上移动导致标题栏和控制栏弹出
            m_play_widget->setMouseTracking(false);
            QTimer::singleShot(400, [this](){m_play_widget->setMouseTracking(true);});

            m_can_hide_all = true;
            hideAll(true);
        }
    });

    // 正在播放的文件改变，书签需要刷新
    connect(m_playlist_widget->getPlayList(), &PlayList::sigPlayingFileMarkUpdate, [this](QVector<MarkItem> marks){
        m_contral_bar->clearMark();
        foreach (MarkItem item, marks) {
            m_contral_bar->addMark(item.m_markPos, item.m_describe);
        }
        m_contral_bar->updateMarks();
    });

    connect(m_playlist_widget->getPlayList(), &PlayList::sigDeleteMark, [this](int mark_pos){
        m_contral_bar->deleteMark(mark_pos);
    });

    // 正在播放时插入书签，需要刷新进度条书签
    connect(m_playlist_widget->getPlayList(), &PlayList::sigInsertMark, [this](MarkItem mark){
        m_contral_bar->insertMark(mark.m_markPos, mark.m_describe);
    });
}

void MainWindow::initMiniModeShade()
{
    m_mini_mode_shade = new MiniModeShade(this);
    m_mini_mode_shade->setAccessibleName("kylin-video_MiniModeShade_m_mini_mode_shade");
    m_mini_mode_shade->hide();

    connect(m_mini_mode_shade, &MiniModeShade::sigClose,      this, &MainWindow::close);
    connect(m_mini_mode_shade, &MiniModeShade::sigShowNormal, this, &MainWindow::slotShowNormalMode);
    connect(m_mini_mode_shade, &MiniModeShade::sigPlayPause, [&](){
        g_user_signal->sigPlayPause();
    });
}

void MainWindow::initContralBar()
{
    m_contral_bar = new ContralBar(this);
    m_contral_bar->setAccessibleName("kylin-video_ContralBar_m_contral_bar");
    m_contral_bar->setUIMode(m_is_tablet_mode);
    m_contral_bar->hide();
    m_contral_bar->raise();

    connect(m_contral_bar, &ContralBar::sigFullScreen, this, &MainWindow::slotShowFullScreen);
    connect(m_contral_bar, &ContralBar::sigCanHide, [this](bool canHide){m_can_hide_all = canHide;});
}

void MainWindow::updateAspect()
{
    double s;
    if(m_core->getRotate() / 90 % 2 == 0)
        s = (double)width() / (double)height();
    else
        s = (double)height() / (double)width();
    m_core->SetAspect(QString::number(s));
}

void MainWindow::resetLayout()
{
    static bool firstShow = true;
    if (firstShow){
        firstShow = false;
        return;
    }
    if (!m_is_mini_mode && !m_is_maximized && !m_is_fullscreen)
        m_normal_rect = geometry();
    if(m_title_widget != nullptr){
        m_title_widget->setGeometry(0, 0, width(), m_title_widget->height());
        m_title_widget->raise();
    }
    if(m_contral_bar != nullptr){
        m_contral_bar->setGeometry(120, height()-72, width()-240, 48);
        m_contral_bar->setPreviewSize(size());
    }
    if(m_music_widget != nullptr){
        m_music_widget->setGeometry(0, 0, width(), height());
    }
    if(m_play_widget != nullptr){
        m_play_widget->setGeometry(0, 0, width(), height());
    }
    if(m_stacked_widget != nullptr){
        m_stacked_widget->setGeometry(0, 0, width(), height());
    }
    if(m_playlist_widget != nullptr){
        m_playlist_widget->updateHideIcon();
        if (Global::isHaWeiKe){
            m_playlist_widget->move(-(m_playlist_widget->width()) + 16, 0);
        }else{
            m_playlist_widget->move(width() - 16, 0);
        }
        m_playlist_widget->resize(m_playlist_widget->width(), height());
    }
    if(m_stacked_widget != nullptr){
        if (m_stacked_widget->currentIndex() == 1)
            hideAll(true);
    }
    if (m_mini_mode_shade)
        m_mini_mode_shade->resize(size());

}

void MainWindow::hideAll(bool hide)
{
    if (hide && !m_music_widget->isHidden())
        return;
    // NEEDFIX:如果是默认音乐图标也不要隐藏
    if (m_stacked_widget->currentIndex() == 0)
        return;
    if (hide)
    {
        if(m_can_hide_all)
        {
            m_contral_bar->setHide();
            m_title_widget->setHide();
            if (!m_playlist_widget->isShow())
                m_playlist_widget->setShowButton(false);
            // 播放列表是否需要隐藏 ?
        }
        else
        {
            // 鼠标要显示出来
            setCursor(Qt::ArrowCursor);
            m_play_widget->setCursor(Qt::ArrowCursor);
            m_title_widget->setCursor(Qt::ArrowCursor);
        }
    }
    else
    {
        if (!m_playlist_widget->isShow()) {
            if (m_stacked_widget->currentIndex() == 1)
                m_contral_bar->setShow();
            if (!m_is_mini_mode)
                m_playlist_widget->setShowButton(true);
            if(!m_is_fullscreen){
                m_title_widget->setShow();
                m_title_widget->raise();
            }
        }
    }
}

void MainWindow::appQuit()
{
    // 判断退出时是否需要清空播放列表，无痕播放关闭的时候也清空列表
    if (g_config->clearListWhenExit.first || g_config->seamlessBrowsing.first)
        g_user_signal->clearPlayList();
    m_exit_flag = true;
    if (g_playstate < 0) {
        exit(0);
    }
    m_core->Stop();
}

void MainWindow::Single(QStringList filelist)
{
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString lockPath = homePath.at(0) + "/.config/kylin-video-lock";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd < 0) { exit(1); }
    if (lockf(fd, F_TLOCK, 0))
    {
        QDBusInterface interface( "org.ukui.kylinvideo",
                                 "/org/ukui/kylinvideo",
                                 "org.ukui.kylinvideo.play",
                                 QDBusConnection::sessionBus());
        QDBusReply<int> reply = interface.call( "kylin_video_play_request", filelist);
        qDebug() << "file path is " << filelist;
        if ( reply.isValid() && reply.value() == 0)
        {
            KyInfo() << "call kylin_video_play_request ok";
        }
        else
        {
            KyInfo() << "fail";
        }
        KyInfo("The app is already running");
        exit(0);
    }
}

void MainWindow::check4Kscreen()
{
#ifndef WHITE_TEST
    QScreen *screen = this->windowHandle()->screen();
    if (!screen) {
        qDebug() << "无法获取窗口所在的屏幕";
    }else{
        // 获取屏幕的全屏尺寸（通常就是屏幕的几何尺寸）
        QRect screenGeometry = screen->geometry();
        int width = screenGeometry.width();
        int height = screenGeometry.height();
        qDebug() << "ActiveWindow size:" << width << "x" << height;
        if(width > 3000 || height > 2000){
            is4Kscreen = true;
        }else{
            is4Kscreen = false;
        }
    }
#endif
}

void MainWindow::windowStateChange()
{
    if (windowState() & Qt::WindowMaximized  || windowState() & Qt::WindowFullScreen){
        m_title_widget->updateMaxButtonStatus(true);
    }
    else{
        m_title_widget->updateMaxButtonStatus(false);
    }
    m_is_active = windowState() & Qt::WindowActive;
}

// x11关闭合成器逻辑，有些显卡性能较低，可以通过关闭合成器的方式提高流畅度，compositor=1 关闭合成器
// void MainWindow::updateCompositor(long compositor)
// {
//     //合成器改变函数
//     Atom _NET_WM_BYPASS_COMPOSITOR = XInternAtom(QX11Info::display(), "_NET_WM_BYPASS_COMPOSITOR", false);
//     XChangeProperty(QX11Info::display(), this->winId(), _NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL,
//                     32, XCB_PROP_MODE_REPLACE, (const unsigned char*)&compositor, 1);
// }
void MainWindow::notifyHdr()
{
    g_user_signal->stop();
    slotShowDefault();
    // 手动停止之后需要显示默认界面

    QMessageBox::warning(this, tr("Video Player"), tr("Doesn't support HDR"), QMessageBox::Ok);

}

void MainWindow::slotQuit()
{
    close();
    //    if(g_config->clearListWhenExit.first || g_config->seamlessBrowsing.first)
    //        g_user_signal->clearPlayList();
    //    m_exit_flag = true;
    //    if (g_playstate < 0) {
    //        exit(0);
    //    }
    //    m_core->Stop();
}

void MainWindow::slotClose()
{
    close();
}

void MainWindow::slotShowMiniMode()
{
    if (g_config->videoOutputType() == GlobalConfig::VO_WID)
        return;
    if (m_is_mini_mode || isFullScreen())
        return;

    if (m_is_maximized) {
        showNormal();
        m_is_maximized = false;
        m_title_widget->updateMaxButtonStatus(false);
    }

    m_is_mini_mode = true;
    m_is_mining_mode = true;
    m_contral_bar->setMiniMode(true);
    m_title_widget->setMiniMode(true);
    m_title_widget->hide();
    m_contral_bar->hide();
    m_playlist_widget->hide();
    emit g_user_signal->sigMiniMode(true);
    QTimer::singleShot(20, [this](){
        m_is_mining_mode = false;
        setMinimumSize(MiniModeSize);
        resize(MiniModeSize);
        dynamic_cast<PlayGLWidget*>(m_play_widget)->setMouseUsed(false);
        if (m_mini_mode_shade) {
            m_mini_mode_shade->show();
            m_mini_mode_shade->raise();

            QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
            move(screenGeometry.width() - 40 - m_mini_mode_shade->width(), 40);
        }
    });
}

void MainWindow::slotShowNormalMode()
{
    if (g_config->videoOutputType() == GlobalConfig::VO_WID)
        return;
    if (!m_is_mini_mode || m_is_mining_mode)
        return;
    setMinimumSize(NormalModeSize);
    setMaximumSize(99999, 99999);
    setGeometry(m_normal_rect);

    m_contral_bar->setMiniMode(false);
    m_title_widget->setMiniMode(false);
    m_playlist_widget->show();
    if (!m_music_widget->isHidden()) {
        m_title_widget->show();
        m_contral_bar->show();
    }

    dynamic_cast<PlayGLWidget*>(m_play_widget)->setMouseUsed(true);

    if (m_mini_mode_shade)
        m_mini_mode_shade->hide();

    m_is_mini_mode = false;
    emit g_user_signal->sigMiniMode(false);
}

void MainWindow::slotShowFullScreen(bool full)
{
    static qint64 lastTime = 0;
    if (QDateTime::currentMSecsSinceEpoch() - lastTime < 300
        || m_is_fullscreen == full
        || m_is_mini_mode)
        return;
    lastTime = QDateTime::currentMSecsSinceEpoch();

    if (full) {
        if(windowState() & Qt::WindowMinimized){
            qDebug() << "Can't perform this operation through DBus in minimized state!" ;
            return;
        }
        if (isWayland) {
            setWindowState(windowState() | Qt::WindowFullScreen);
        }
        else {
            KWindowSystem::forceActiveWindow(this->winId());
            KWindowSystem::setState(winId(), NET::FullScreen);
        }
        if(m_stacked_widget->currentIndex() == 1)
            m_title_widget->hide();
        m_is_fullscreen = true;
    } else {
        if (isWayland) {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        } else {
            KWindowSystem::clearState(winId(), NET::FullScreen);
        }
        m_title_widget->show();
        m_is_fullscreen = false;
    }
    m_contral_bar->setFullScreenState(m_is_fullscreen);
    m_can_hide_all = true;
    hideAll(true);
}

void MainWindow::slotShowTitleMenu()
{
    m_title_menu->exec(QPoint(m_title_widget->getMenuBtnX()+mapToGlobal(QPoint(0,0)).x(),
                              mapToGlobal(QPoint(0,0)).y()+m_title_widget->height()-5));
}

void MainWindow::slotChangeMaxState()
{
    // 如果是全屏的话退出全屏
    if (m_is_fullscreen) {
        slotShowFullScreen(false);
        return;
    }

    if (windowState() & Qt::WindowMaximized) {
        showNormal();
        m_is_maximized = false;
    } else {
        showMaximized();
        m_is_maximized = true;
    }
}

void MainWindow::slotChangeMiniState()
{
    showMinimized();
}

void MainWindow::slotOpenHelpDoc()
{
    QString serviceName = KYLIN_USER_GUIDE_SERVICE + QString("%1%2").arg("_").arg(QString::number(getuid()));
    QDBusInterface *iface = new QDBusInterface(serviceName,
                                               KYLIN_USER_GUIDE_PATH,
                                               KYLIN_USER_GUIDE_INTERFACE,
                                               QDBusConnection::sessionBus(),
                                               this);
    iface->call(QString("showGuide"), "kylin-video");
    delete iface;
}

void MainWindow::slotPlayStateChange()
{
    static int last_state = Mpv::Idle;
    // 如果是 Paused -> Playing 不要进行切换
    if(g_playstate == Mpv::Playing && last_state != Mpv::Paused) {
        m_stacked_widget->setCurrentIndex(1);
        m_title_widget->setHomePage(false);
    }
    last_state = g_playstate;

    // 停止之后画面比例还原（如果需要保持比例需要重新做功能）
    if (g_playstate < 0) {
        m_video_aspect = Mpv::AUTO;
    }

    if (g_playstate == Mpv::Playing)
    {
        if(m_core->getVid() >= 0) {
            m_music_widget->hide();
        }
        else if(m_core->getAid() >= 0){
            m_music_widget->show();
            m_music_widget->raise();
        }
    }

    // 如果正在播放，阻止锁屏
    if (m_dbus_interface->isValid()) {
        if (g_playstate == Mpv::Playing)
        {
            QDBusMessage reply = m_dbus_interface->call(QDBus::Block, "Inhibit", "kylin-video", (quint32)0, "video is playing", (quint32)8);
            m_inhibit_value = reply.arguments().takeFirst().toUInt();
        }
        else
        {
            m_dbus_interface->call("Uninhibit", m_inhibit_value);
        }
    }
    else {
        KyInfo("org.gnome.SessionManager is invalid!");
    }

    // 停止后是否需要退出
    if (g_playstate == Mpv::Idle && m_exit_flag) {
        exit(0);
    }
}

void MainWindow::slotThemeChange(int theme)
{
    switch (theme) {
    case 0:
        g_settings->setValue("General/follow_system_theme", true);
        break;
    case 1:
        g_settings->setValue("General/follow_system_theme", false);
        g_settings->setValue("General/theme", 0);
        break;
    case 2:
        g_settings->setValue("General/follow_system_theme", false);
        g_settings->setValue("General/theme", 1);
        break;
    default:
        break;
    }
}

void MainWindow::slotShowDefault()
{
    if (m_is_mini_mode) {
        slotShowNormalMode();
    }
    // 当前默认为拼音拼播放界面
    m_music_widget->hide();
    m_title_widget->setTitle(tr("Video Player"));
    m_title_widget->setShow();
    hideAll(false);
    m_contral_bar->hide();
    m_stacked_widget->setCurrentIndex(0);
    m_title_widget->setHomePage(true);
}

void MainWindow::slotChangePause()
{
    if(m_playlist_widget->isShow())
        m_playlist_widget->slotHide();

    if(g_playstate == Mpv::Playing)
        m_core->Pause();
    else if(g_playstate == Mpv::Paused)
        m_core->Play();
}

/**
 * @brief       : 打开文件
 * @param [in]  :
 * @param [out] :
 * @return      :
 */
void MainWindow::slotOpenFile()
{
#ifndef WHITE_TEST
    Extensions e;
    QString last_path = g_settings->value("History/last_path").toString();
    if(last_path == "")
        last_path = QDir::homePath();
    QStringList files;
    {
        QWidget *parent_widget = nullptr;
        if (isWayland)
            parent_widget = this;
        QFileDialog fd(parent_widget);
        //        KWindowSystem::setState(QFileDialog.winId(), NET::SkipPager | NET::SkipSwitcher);
        fd.setModal(true);
        QList<QUrl> list = fd.sidebarUrls();
        int sidebarNum = 8;
        QString home = QDir::homePath().section("/", -1, -1);
        QString mnt = "/media/" + home + "/";
        QDir mntDir(mnt);
        mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList filist = mntDir.entryInfoList();
        QList<QUrl> mntUrlList;
        for(int i=0; i < sidebarNum && i < filist.size(); i++) {
            QFileInfo fi = filist.at(i);
            //华为990、9a0需要屏蔽最小系统挂载的目录
            if (fi.fileName() == "2691-6AB8")
                continue;
            mntUrlList << QUrl("file://" + fi.filePath());
        }
        QFileSystemWatcher fsw(&fd);
        fsw.addPath("/media/" + home + "/");
        connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd, [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path){
            QDir wmn_dir(path);
            wmn_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList wfilist = wmn_dir.entryInfoList();
            mntUrlList.clear();
            for(int i=0; i < sidebarNum && i < wfilist.size(); ++i) {
                QFileInfo fi = wfilist.at(i);
                //华为990、9a0需要屏蔽最小系统挂载的目录
                if (fi.fileName() == "2691-6AB8")
                    continue;
                mntUrlList << QUrl("file://" + fi.filePath());
            }
            fd.setSidebarUrls(list + mntUrlList);
            fd.update();
        });
        connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd](){
            fd.setSidebarUrls(list);
        });
        fd.setSidebarUrls(list + mntUrlList);
        fd.setDirectory(QDir(last_path));
        fd.setWindowTitle(tr("Video Player Choose a file"));

        QString setFilterM = tr("Multimedia") + (" (%1) ");
        setFilterM = setFilterM.arg(e.allPlayable().forFilter());

        QString setFilterV = tr("Video") + (" (%1) ");
        setFilterV = setFilterV.arg(e.video().forFilter());

        QString setFilterA = tr("Audio") + (" (%1) ");
        setFilterA = setFilterA.arg(e.audio().forFilter());

        fd.setNameFilters(QStringList()
                          << setFilterM
                          << setFilterV
                          << setFilterA);

        fd.setFileMode(QFileDialog::ExistingFiles);
        fd.setViewMode(QFileDialog::Detail);
        fd.setOption(QFileDialog::HideNameFilterDetails);

        g_shortcut->makeAllInvalid();
        kdk::UkuiStyleHelper::self()->removeHeader(&fd);
        if(fd.exec() == QFileDialog::Accepted)
        {
            files = fd.selectedFiles();
            qDebug() << files;
        }
        g_shortcut->makeAllValid();
    }

    if(files.size() <= 0)
        return;
    g_settings->setValue("History/last_path", files.first().left(files.first().lastIndexOf('/') + 1));

    g_user_signal->addFiles(files);
#endif
}

void MainWindow::slotOpenDir()
{
#ifndef WHITE_TEST
    QString last_path = g_settings->value("History/last_path").toString();
    // 打开文件夹
    QString url;
    {
        QWidget *parent_widget = nullptr;
        if (isWayland)
            parent_widget = this;
        QFileDialog fd(parent_widget);
        fd.setModal(true);
        int sidebarNum = 8;
        QList<QUrl> list = fd.sidebarUrls();
        QString home = QDir::homePath().section("/", -1, -1);
        QString mnt = "/media/" + home + "/";
        QDir mntDir(mnt);
        mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList filist = mntDir.entryInfoList();
        QList<QUrl> mntUrlList;
        for(int i=0; i < sidebarNum && i < filist.size(); i++) {
            QFileInfo fi = filist.at(i);
            //华为990、9a0需要屏蔽最小系统挂载的目录
            if (fi.fileName() == "2691-6AB8")
                continue;
            mntUrlList << QUrl("file://" + fi.filePath());
        }
        QFileSystemWatcher fsw(&fd);
        fsw.addPath("/media/" + home + "/");
        connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd, [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path){
            QDir wmnDir(path);
            wmnDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList wfilist = wmnDir.entryInfoList();
            mntUrlList.clear();
            for(int i=0; i < sidebarNum && i < wfilist.size(); ++i) {
                QFileInfo fi = wfilist.at(i);
                //华为990、9a0需要屏蔽最小系统挂载的目录
                if (fi.fileName() == "2691-6AB8")
                    continue;
                mntUrlList << QUrl("file://" + fi.filePath());
            }
            fd.setSidebarUrls(list + mntUrlList);
            fd.update();
        });
        connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd](){
            fd.setSidebarUrls(list);
        });
        fd.setSidebarUrls(list + mntUrlList);
        fd.setDirectory(QDir(last_path));
        fd.setWindowTitle(tr("Video Player Choose a directory"));
        fd.setFileMode(QFileDialog::DirectoryOnly);

        g_shortcut->makeAllInvalid();
        kdk::UkuiStyleHelper::self()->removeHeader(&fd);
        if(fd.exec() == QFileDialog::Accepted)
        {
            url = fd.selectedUrls().at(0).toString(QUrl::DecodeReserved);
            if(url.startsWith("file")) {
                url.remove(0, 7);
            }
        }
        g_shortcut->makeAllValid();
    }

    if (!url.isEmpty()) {
        g_user_signal->addDir(url);
    }
    g_settings->setValue("History/last_path", url);
#endif
}

void MainWindow::slotOpenUrl()
{
    if (!m_urlEdit) {
        m_urlEdit = new URLEditWidget(m_play_widget);
    }
    m_urlEdit->setModal(true);

    //改用SDK接口实现跳过任务栏
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_urlEdit->window());
    windowHelper->removeTitleBar();
    windowHelper->setSkipSwitcher(true);
    windowHelper->setSkipTaskBar(true);

    if (m_urlEdit->exec() > 0)
    {        
        QString url = m_urlEdit->getUrl();
        slotPlayFile(url, 0);
        isUrl = true;
    }
}

void MainWindow::slotPlayFile(QString file, int pos)
{
    if (file == "")
        return;

    MediaHandle mi(file);
    if (isD3000M && mi.getIsHdr()) {
        notifyHdr();
        return;
    }
    // 开始播放隐藏所有
    m_stacked_widget->setCurrentIndex(1);
    //    m_stacked_widget->setCurrentWidget(m_play_widget);
    m_title_widget->setHomePage(false);
    m_core->Open(file, pos);
    hideAll(true);
}

void MainWindow::slotPlayWidgetClicked()
{
#if 0
    if(m_playlist_widget->isShow())
        m_playlist_widget->slotHide();
    else
#endif
    {
        if(m_playlist_widget->isShow()) {
            m_playlist_widget->slotHide();
        }
        else {
            if(g_playstate == Mpv::Playing)
                m_core->Pause();
            else if(g_playstate == Mpv::Paused)
                m_core->Play();
        }
    }
}

void MainWindow::slotToTop(bool isTop)
{
    m_is_top = isTop;

    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    UkuiWindowHelper::States defaultState = UkuiWindowHelper::States(0xff);
    if(isTop){
        // 设置置顶
        windowHelper->setWindowState(defaultState | UkuiWindowHelper::State::KeepAbove);
    } else{
        // 取消置顶
        windowHelper->setWindowState(defaultState & ~UkuiWindowHelper::State::KeepAbove);
    }
}

void MainWindow::initGlobalSig()
{
    connect(g_user_signal, &GlobalUserSignal::sigShowStopFrame,     this, &MainWindow::slotShowDefault);
    connect(g_user_signal, &GlobalUserSignal::sigExit,              this, &MainWindow::close);
    connect(g_user_signal, &GlobalUserSignal::sigPlayWidgetClicked, this, &MainWindow::slotChangePause);
    connect(g_user_signal, &GlobalUserSignal::sigSelectFile,        this, &MainWindow::slotOpenFile);
    connect(g_user_signal, &GlobalUserSignal::sigSelectDir,         this, &MainWindow::slotOpenDir);
    connect(g_user_signal, &GlobalUserSignal::sigOpenUrl,           this, &MainWindow::slotOpenUrl);
    connect(g_user_signal, &GlobalUserSignal::sigDbusUrl,           this,[&](QString Url){
        slotPlayFile(Url, 0);
    });
    // 直接让 mpvcore 去播放有问题，必须先切换到播放页面
    connect(g_user_signal, &GlobalUserSignal::sigOpen,              this, &MainWindow::slotPlayFile);
    connect(g_user_signal, &GlobalUserSignal::sigPlayPause, [&](){
        if(g_playstate == Mpv::Playing)
            m_core->Pause();
        else if(g_playstate == Mpv::Paused)
            m_core->Play();
    });

    connect(g_user_signal, &GlobalUserSignal::sigOpenHelpDoc,   this, &MainWindow::slotOpenHelpDoc);
    connect(g_user_signal, &GlobalUserSignal::sigHideBar,       this, &MainWindow::hideAll);
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange,   this, &MainWindow::slotPlayStateChange);
    connect(g_core_signal, &GlobalCoreSignal::sigFileInfoChange, [&](Mpv::FileInfo fi){
        int tmp_duration = m_file_info.length;
        m_file_info = fi;
        m_file_info.length = tmp_duration;
        m_current_file = fi.file_path;
        m_title_widget->setTitle(fi.file_path.split("/").back());
    });

    connect(g_user_signal, &GlobalUserSignal::sigChangeShowMode, [this](){
        if (m_stacked_widget->currentIndex() == 0)
            return;
        if (m_is_mini_mode)
            slotShowNormalMode();
        else
            slotShowMiniMode();
    });

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if 1
    connect(g_user_signal, &GlobalUserSignal::sigTheme, this, &MainWindow::slotThemeChange);
#endif

    connect(g_user_signal, &GlobalUserSignal::sigRightMenuShow, [&](){
        //        if (!m_is_active && isWayland)
        //            return;
        if (QGuiApplication::focusWindow())
            m_right_click_menu->exec(QCursor::pos(QGuiApplication::focusWindow()->screen()) + QPoint(1,1));
    });

#if 0
    connect(g_user_signal, &GlobalUserSignal::sigShowPlayList, [&](){
        if(!m_playlist_widget->isShow() && ui->stackedWidget->currentIndex() == 1) {
            slotHideAll(true);
        }
    });
#endif

    connect(g_core_signal, &GlobalCoreSignal::sigDuration, [&](QString, double duration){
        m_file_info.length = (int)duration;
    });

    connect(g_user_signal, &GlobalUserSignal::sigShowSetup, [&](int index){
        QWidget* show_parent = nullptr;
        if (isWayland)
            show_parent = this;
        if (!m_setupDialog) {
            m_setupDialog = new SetUpDialog(show_parent);
        }
        m_setupDialog->setModal(true);
        m_setupDialog->setIndex(index);
        kdk::UkuiStyleHelper::self()->removeHeader(m_setupDialog);
        KWindowSystem::setState(m_setupDialog->winId(), NET::SkipPager | NET::SkipSwitcher);
        m_setupDialog->show();
        m_setupDialog->exec();
    });

    connect(g_user_signal, &GlobalUserSignal::sigShowAbout, [&](){
        QWidget* show_parent = nullptr;
        if (isWayland)
            show_parent = this;
        if (!m_aboutDialog) {
            m_aboutDialog = new kdk::KAboutDialog(show_parent);
        }
        m_aboutDialog->setModal(true);
        m_aboutDialog->setAppIcon(QIcon::fromTheme(qAppName()));
        m_aboutDialog->setAppName(tr("Video Player"));
        m_aboutDialog->setAppVersion(tr("version:").append(Functions::getVersion()));
        kdk::UkuiStyleHelper::self()->removeHeader(m_aboutDialog);
        KWindowSystem::setState(m_aboutDialog->winId(), NET::SkipPager | NET::SkipSwitcher);
        m_aboutDialog->show();
//        m_aboutDialog->move((width() - m_aboutDialog->width()) / 2 + x(), (height() - m_aboutDialog->height()) / 2 + y());
        m_aboutDialog->exec();


#if 0
        AboutDialog aboutDialog;
        aboutDialog.move(this->geometry().center() - aboutDialog.geometry().center());

        if(isWayland)
        {
            UKUIDecorationManager::getInstance()->removeHeaderBar(aboutDialog.windowHandle());
            UKUIDecorationManager::getInstance()->setCornerRadius(aboutDialog.windowHandle(), 12, 12, 12, 12);
            aboutDialog.setFixedSize(aboutDialog.size());
        }
        aboutDialog.exec();
#endif

    });

    connect(g_user_signal, &GlobalUserSignal::sigOpenApp,       this, [this]() {
        kylin_video_play_request(QStringList());
    });

    connect(g_core_signal, &GlobalCoreSignal::sigNotifyHdr, this, &MainWindow::notifyHdr);
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowStateChange:
        windowStateChange();
        break;
    case QEvent::ContextMenu:
        // 主界面也能右键呼出菜单
        if (QGuiApplication::focusWindow()) {
            g_user_signal->showRightMenu();
        }
        break;
    case QEvent::MouseMove:
        setCursor(Qt::ArrowCursor);
        break;
    case QEvent::MouseButtonPress:
        // 告诉控制栏坐标
        m_contral_bar->setMainWidgetClickPos(static_cast<QMouseEvent*>(event)->pos());
        break;
    default:
        break;
    }

    if (((isWayland && event->type() == QEvent::WindowActivate) || (!isWayland && event->type() == QEvent::Show)) && m_need_play_when_min) {
        m_core->Play();
        m_need_play_when_min = false;
    }

    // wayland 最小化的时候没有 Hide 事件
    if (event->type() == QEvent::Hide && g_config->pauseWhenMini.first && g_playstate == Mpv::Playing) {
        m_core->Pause();
        m_need_play_when_min = true;
    }

    return QWidget::event(event);
}
#ifndef WHITE_TEST
void MainWindow::enterEvent(QEvent *event)
{
    if(m_stacked_widget->currentIndex() == 0)
        return;
    else
        hideAll(false);
    return QWidget::enterEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    if (!m_is_mini_mode && !m_is_maximized && !m_is_fullscreen)
        m_normal_rect = geometry();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    resetLayout();
    return QWidget::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭的时候最小化到系统托盘 就是隐藏了就行
    if(g_config->miniToTray.first)
    {
        // 隐藏的时候有必要还原大小，不然再次显示会有问题
        if (isFullScreen()) {
            g_user_signal->fullScreen();
            QThread::msleep(300);
        }
        hide();
        event->ignore();
        resetLayout();
        return;
    }

    // 判断退出时是否需要清空播放列表
    if(g_config->clearListWhenExit.first || g_config->seamlessBrowsing.first)
        g_user_signal->clearPlayList();
    m_core->Stop();
    QThread::usleep(100000);

    // 关闭之前取消阻止锁屏
    m_dbus_interface->call("Uninhibit", m_inhibit_value);
    QWidget::closeEvent(event);
    kdk_dia_upload_program_exit();
    quick_exit(0);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
#if 0
    if (event->button() == Qt::LeftButton) {
        m_pressPos = event->pos();
        m_isMove = true;
    }else
#endif

    return QWidget::mousePressEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 按 esc 退出全屏
    if(event->key() == Qt::Key_Escape) {
        if(isFullScreen())
            g_user_signal->fullScreen();
    }
    return QWidget::keyPressEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            emit windowActivated();
        } else {
            emit windowDeactivated();
        }
    }
    QWidget::changeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{

    QList<QUrl> listUrl = event->mimeData()->urls();
    Extensions e;
    QRegExp rx_video(e.video().forRegExp());
    QRegExp rx_audio(e.audio().forRegExp());
    rx_video.setCaseSensitivity(Qt::CaseInsensitive);
    rx_audio.setCaseSensitivity(Qt::CaseInsensitive);
    for (QUrl url : listUrl)
    {
        QString path = url.path();

        // 不要 file:// 前缀只要绝对路径
        if(path.startsWith("file:"))
            path.remove(0, 7);

        // 拖入文件需要做类型判断
        QFileInfo fi(path);
        if (fi.isDir() || rx_video.indexIn(fi.suffix()) > -1 || rx_audio.indexIn(fi.suffix()) > -1) {
            event->acceptProposedAction();

        }
        else{
            setCursor(Qt::ForbiddenCursor);
        }
    }
    setCursor(Qt::ArrowCursor);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> listUrl = event->mimeData()->urls();
    QStringList filelist;
    Extensions e;
    QRegExp rx_video(e.video().forRegExp());
    QRegExp rx_audio(e.audio().forRegExp());
    rx_video.setCaseSensitivity(Qt::CaseInsensitive);
    rx_audio.setCaseSensitivity(Qt::CaseInsensitive);
    for (QUrl url : listUrl)
    {
        QString path = url.path();

        // 不要 file:// 前缀只要绝对路径
        if(path.startsWith("file:"))
            path.remove(0, 7);

        // 拖入文件需要做类型判断
        QFileInfo fi(path);
        if (fi.isDir()) {
            // 如果是文件夹的话添加文件夹
            g_user_signal->addDir(path);
        }
        else if (rx_video.indexIn(fi.suffix()) > -1 || rx_audio.indexIn(fi.suffix()) > -1) {
            filelist << path;
        }
    }
    if(filelist.count() == 0)
        return;
    g_user_signal->addFiles(filelist);
}
#endif
