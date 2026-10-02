/*
 * Peony-Qt
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "peony-desktop-application.h"

#include "fm-dbus-service.h"
#include "desktop-menu-plugin-manager.h"

#include "volume-manager.h"

#include "global-settings.h"
#include "desktop-menu.h"
#include "global-settings.h"
#include "file-enumerator.h"
#include "advanced-desktop-item-model.h"
#include "desktop-window-manager.h"
#include "windowmanager/windowmanager.h"
#include "screen-manager.h"
#include "image-mount-manager.h"
#include "background-manager.h"

#include <widget-loader.h>
#include <widget-container.h>
#include "widgets-proxy-model.h"

#include <QMessageBox>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTimer>
#include "theme-style.h"
#include "desktop-widgets-manager.h"
#include "file-action-controler.h"

#include "qml-desktop/pag/PAGView.h"

#ifdef TEST_PEONY_DESKTOP_WIDGETS

#include <QInputDialog>
#include "platformwindow.h"
#include <widget-loader.h>
#endif

#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QtConcurrent>

#include <QTranslator>
#include <QLocale>

#include <QSystemTrayIcon>
#include <QGSettings>

#include <gio/gio.h>

#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>

#include <QScreen>
#include <QTimeLine>

#include <QSessionManager>
#include <KWindowSystem>
#include <QX11Info>
#include <X11/Xlib.h>

#define KYLIN_USER_GUIDE_PATH "/"
#define KYLIN_USER_GUIDE_SERVICE QString("com.kylinUserGuide.hotel_%1").arg(getuid())
#define KYLIN_USER_GUIDE_INTERFACE "com.guide.hotel"

#define DESKTOP_MEDIA_HANDLE "org.gnome.desktop.media-handling"

static bool has_desktop = false;
static bool has_daemon = false;
static bool has_background = false;
static QRect max_size = QRect(0, 0, 0, 0);
static Peony::AdvancedDesktopItemModel *desktop_model = nullptr;
static DesktopWindowManager *desktopManger = nullptr;
static bool g_emitFinish = false;

//record of desktop start time
qint64 PeonyDesktopApplication::peony_desktop_start_time = 0;



void trySetDefaultFolderUrlHandler() {
    //NOTE:
    //There is a bug in qt concurrent. If we use QtConcurrent::run()
    //to start a async function in the init stage, the qapplication will
    //not quit when we call "peony-qt-desktop -q" in command line.
    //
    //To solve this problem, the simplest way is delaying a while to execute the
    //async lambda function.
    QTimer::singleShot(1000, []() {
        auto thread = QThread::create([=]() {
            GList *apps = g_app_info_get_all_for_type("inode/directory");
            bool hasPeonyQtAppInfo = false;
            GList *l = apps;
            while (l) {
                GAppInfo *info = static_cast<GAppInfo*>(l->data);
                QString cmd = g_app_info_get_executable(info);
                if (cmd.contains("peony")) {
                    hasPeonyQtAppInfo = true;
                    g_app_info_set_as_default_for_type(info, "inode/directory", nullptr);
                    break;
                }
                l = l->next;
            }
            if (apps) {
                g_list_free_full(apps, g_object_unref);
            }

            if (!hasPeonyQtAppInfo) {
                GAppInfo *peony_qt = g_app_info_create_from_commandline("peony",
                                     nullptr,
                                     G_APP_INFO_CREATE_SUPPORTS_URIS,
                                     nullptr);
                g_app_info_set_as_default_for_type(peony_qt, "inode/directory", nullptr);
                g_object_unref(peony_qt);
            }
            return;
        });
        thread->start(QThread::IdlePriority);
    });
}

//task#147972, story#21858, GreatWall svn and SE9215 controler, all file provide trash option
void greatWallDeviceInit()
{
    auto thread = QThread::create([=]() {
        QProcess process;
        //process 中管道命令不生效，需要自己对读取的字符串进行处理
        //process.start("lspci | grep -c 88SE9215");
        process.start("lspci");
        process.waitForFinished();
        QString result = process.readAllStandardOutput();
        QStringList retList = result.split("\n");
        qDebug() << "greatWallDeviceInit lspci:"<<result;
        bool isTargetController = false;
        for(QString match : retList){
            if (match.indexOf("88SE9215") >=0){
                qDebug() << "match info:"<<match.indexOf("88SE9215")<<match;
                isTargetController = true;
                break;
            }
        }

        //not SE9215 controler
        if (! isTargetController)
            return;

        QProcess process1;
        process1.start("cat /sys/class/dmi/id/modalias");
        process1.waitForFinished();
        QString ret = process1.readAllStandardOutput();
        QStringList infoList = ret.split(":");
        for(QString info:infoList){
            if (info.startsWith("cvn", Qt::CaseInsensitive)){
                QString manufacturer = info.replace("cvn", "", Qt::CaseInsensitive);
                qDebug() << "greatWallDeviceInit manufacturer:"<<manufacturer;
                if (manufacturer.toUpper() == "GREATWALL")
                {
                    //setGsetting value
                    GlobalSettings::getInstance()->setValue(TRASH_MOBILE_FILES, true);
                }
                break;
            }
        }
    });
    thread->start(QThread::IdlePriority);
}

QRect caculateVirtualDesktopGeometry() {
    QRegion screensRegion;
    for (auto screen : qApp->screens()) {
        screensRegion += screen->geometry();
    }

    auto rect = screensRegion.boundingRect();
    if (rect.size().width()> rect.size().height())
    {
        rect = QRect(0, 0, rect.size().width(), rect.size().width());
    }
    else
    {
        rect = QRect(0, 0, rect.size().height(), rect.size().height());
    }
    if (rect.size().width()> max_size.size().width())
    {
        max_size = rect;
    }
    return max_size;
}

PeonyDesktopApplication::PeonyDesktopApplication(int &argc, char *argv[], const QString &applicationName) : QtSingleApplication (applicationName, argc, argv)
{
    bool isWayland = QString(qgetenv("XDG_SESSION_DESKTOP")).contains("kylin-wlcom") ||
            QString(qgetenv("XDG_SESSION_DESKTOP")).contains("ukui-wayland");

    setProperty("isWayland", isWayland);
    setProperty("isDesktop", true);

    GlobalSettings::getInstance()->setDesktopStartUp(true);
    // fix #172774
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths()<<"/usr/share/pixmaps");

    qDebug()<<"peony desktop constructor start";
    setApplicationVersion(QString("v%1").arg(VERSION));
    //setApplicationDisplayName(tr("Peony-Qt Desktop"));

    QTranslator *t = new QTranslator(this);
    t->load("/usr/share/libpeony-qt/libpeony-qt_"+QLocale::system().name());
    QApplication::installTranslator(t);
    QTranslator *t2 = new QTranslator(this);
    t2->load("/usr/share/peony-qt-desktop/peony-qt-desktop_"+QLocale::system().name());
    QApplication::installTranslator(t2);
    QTranslator *t3 = new QTranslator(this);
    t3->load("/usr/share/qt5/translations/qt_"+QLocale::system().name());
    QApplication::installTranslator(t3);
    QTranslator *sdkTrans = new QTranslator(this);
    if (sdkTrans->load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        QApplication::installTranslator(sdkTrans);
    }
    setApplicationName(tr("peony-qt-desktop"));
    setWindowIcon(QIcon::fromTheme("system-file-manager"));

    //added for session call interactive
    QGuiApplication::setFallbackSessionManagementEnabled(true);
    QGuiApplication::setQuitOnLastWindowClosed(false);
    // fix #145378, logout background displayment issue.
    QObject::connect(this, &QGuiApplication::saveStateRequest, this, [=](QSessionManager &manager){
        if (KWindowSystem::isPlatformX11()) {
            if (manager.allowsErrorInteraction()) {
                qInfo()<<"session about to logout, clear the root window background";
//                XSetWindowBackground(QX11Info::display(), QX11Info::appRootWindow(), 0);
//                XSync(QX11Info::display(), false);
                manager.release();
            } else {
                qInfo()<<"session not support interaction or not x11 platform";
//                XSetWindowBackground(QX11Info::display(), QX11Info::appRootWindow(), 0);
//                XSync(QX11Info::display(), false);
            }
        }
    });

    // global settings
    if (QGSettings::isSchemaInstalled (FONT_SETTINGS)) {
        //font monitor
        QGSettings *fontSetting = new QGSettings (FONT_SETTINGS, QByteArray(), this);
        connect (fontSetting, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == "systemFont" || key == "systemFontSize") {
                QFont font = this->font();
                for(auto widget : qApp->allWidgets()) {
                    widget->setFont(font);
                }
            }
        });
    }

    if (!this->isRunning()) {
        qDebug()<<"isPrimary screen";
        getDesktopWindowManager();

        //check if is great wall device and init settings
        connect(desktopManger, &DesktopWindowManager::emitFinish, this , [=](){
            if (g_emitFinish)
                return;
            g_emitFinish = true;
            GlobalSettings::getInstance()->setDesktopStartUp(false);/* 桌面启动结束 */
            QDBusMessage message = QDBusMessage::createMethodCall("org.gnome.SessionManager",
                                                                  "/org/gnome/SessionManager",
                                                                  "org.gnome.SessionManager",
                                                                  "startupfinished");
            QList<QVariant> args;
            args.append("peony-qt-desktop");
            args.append("startupfinished");
            message.setArguments(args);
            QDBusConnection::sessionBus().send(message);

            greatWallDeviceInit();
        });

        connect(this, &QtSingleApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });
        QFile file(":/desktop-icon-view.qss");
        file.open(QFile::ReadOnly);
        setStyleSheet(QString::fromLatin1(file.readAll()));
        file.close();

        //add 5 seconds delay to load plugins
        //try to fix first time enter desktop right menu not show open terminal issue
        //fix need wait 5 seconds open terminal issue, link to bug#75654
        //目前新的加载流程不需要等待那么久，加载出桌面时，应用都注册好了
//        QTimer::singleShot(1000, [=]() {
//            Peony::DesktopMenuPluginManager::getInstance();
//        });

        /*
        QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
        auto volumeManager = Peony::VolumeManager::getInstance();
        connect(volumeManager, &Peony::VolumeManager::driveConnected, this, [=](const std::shared_ptr<Peony::Drive> &drive){
            trayIcon->show();
            trayIcon->showMessage(tr("Drive"), tr("%1 connected").arg(drive->name()));
        });
        connect(volumeManager, &Peony::VolumeManager::driveDisconnected, this, [=](const std::shared_ptr<Peony::Drive> &drive){
            trayIcon->show();
            trayIcon->showMessage(tr("Drive"), tr("%1 disconnected").arg(drive->name()));
        });

        connect(trayIcon, &QSystemTrayIcon::messageClicked, trayIcon, &QSystemTrayIcon::hide);
        */

//        if(!GlobalSettings::getInstance()->isDesktopStartUp()){
//            // auto mount local driver
//            qDebug()<<"auto mount local volumes";
//            GVolumeMonitor* vm = g_volume_monitor_get ();
//            if (vm) {
//                GList* drives = g_volume_monitor_get_connected_drives(vm);
//                if (drives) {
//                    for (GList* i = drives; nullptr != i; i = i->next) {
//                        GDrive * d = static_cast<GDrive*>(i->data);
//                        if (G_IS_DRIVE(d)) {
//                            GList* volumes = g_drive_get_volumes(d);
//                            if (volumes) {
//                                for (GList* j = volumes; nullptr != j; j = j->next) {
//                                    GVolume* v = static_cast<GVolume*>(j->data);
//                                    if (G_IS_VOLUME(v)) {
//                                        g_autofree char* uuid = g_volume_get_uuid(v);
//                                        if (0 != g_strcmp0("2691-6AB8", uuid)) {
//                                            g_volume_mount(v, G_MOUNT_MOUNT_NONE, nullptr, nullptr, volume_mount_cb, nullptr);
//                                        }
//                                        g_object_unref(v);
//                                    }
//                                }
//                                g_list_free(volumes);
//                            }
//                            g_object_unref(d);
//                        }
//                    }
//                    g_list_free(drives);
//                }
//            }
//            g_object_unref(vm);

//            g_signal_connect (g_volume_monitor_get(), "mount-added", G_CALLBACK(mount_added_cb), nullptr);
//            g_signal_connect (g_volume_monitor_get(), "mount-changed", G_CALLBACK(mount_added_cb), nullptr);
//        }

        // enumerat network:///
        // 适配2503内存优化任务，桌面不拉起gvfsd-dnssd和gvfsd-network
        // note: 不后台启动可能会导致访问网络变慢，可能需要实际验证是否对用户体验有影响
//        QThread* t = QThread::create ([=] () {
//            FileEnumerator e;
//            e.setEnumerateDirectory ("network:///");
//            e.enumerateSync();
//            e.getChildrenUris ();
//        });
//        connect (t, &QThread::finished, t, &QObject::deleteLater);
//        t->start ();
    }

    //parse cmd
    qDebug()<<"parse cmd";
    auto message = this->arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());

//    if(!GlobalSettings::getInstance()->isDesktopStartUp()){
//        qDebug()<<"monitor volumes change";
//        auto volumeManager = Peony::VolumeManager::getInstance();
//        connect(volumeManager,&Peony::VolumeManager::mountAdded,this,[=](const std::shared_ptr<Peony::Mount> &mount){
//            // auto open dir for inserted dvd.
//            GMount* newMount = (GMount*) g_object_ref(mount->getGMount());
//            //special Volumn of 839 M upgrade part can not mount
//            if (mount->uuid() != "2691-6AB8")
//                g_mount_guess_content_type(newMount, FALSE, NULL, guessContentTypeCallback, NULL);
//            // mount
//        });
//        connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,&PeonyDesktopApplication::volumeRemovedProcess);
//    }

    // 获取max_size初始值
    //caculateVirtualDesktopGeometry();

    connect(IMAGE_MOUNT_MANAGER, &Peony::ImageMountManager::deviceMounted, this, [this](const QString &mountPoint) {
        if (QFileInfo(mountPoint).exists()) {
            QUrl url(QString("file://" + mountPoint));
            QString uri = url.toEncoded();
            QStringList args;
            args << uri;
            QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.setProgram("/usr/bin/peony");
            p.setArguments(QStringList()<<"--show-folders"<<args);
            p.startDetached();
#else
            p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<<args);
#endif
        }
    });
    connect(IMAGE_MOUNT_MANAGER, &Peony::ImageMountManager::mountError, this, [this](const QString &errMsg) {
        if (!errMsg.isEmpty()) {
            qDebug() << "iso image unmount or mount error: " << errMsg;
        }
    });
    qDebug()<<"peony desktop application constructor end";
}

Peony::AdvancedDesktopItemModel *PeonyDesktopApplication::getModel()
{
    if (!desktop_model)
        desktop_model =  new AdvancedDesktopItemModel();
    return desktop_model;
}

DesktopWindowManager *PeonyDesktopApplication::getDesktopWindowManager()
{
    if (!desktopManger)
        desktopManger =  DESKTOP_MANAGER;
    return desktopManger;
}

bool PeonyDesktopApplication::userGuideDaemonRunning()
{
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.isConnected())
        return false;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", KYLIN_USER_GUIDE_SERVICE);

    return reply != "";
}

void PeonyDesktopApplication::showGuide(const QString &appName)
{
    if (!userGuideDaemonRunning()) {
        QUrl url = QUrl("help:ubuntu-kylin-help", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
        return;
    }

    bool bRet  = false;

    QDBusMessage m = QDBusMessage::createMethodCall(KYLIN_USER_GUIDE_SERVICE,
                     KYLIN_USER_GUIDE_PATH,
                     KYLIN_USER_GUIDE_INTERFACE,
                     "showGuide");
    m << appName;

    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    if (response.type()== QDBusMessage::ReplyMessage)
    {
        //bRet = response.arguments().at(0).toBool();
    }
    else {
        QUrl url = QUrl("help:ubuntu-kylin-help", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
    }
}

void PeonyDesktopApplication::gotoSetBackground()
{
    QProcess p;
    p.setProgram("/usr/bin/ukui-control-center");
    //old version use -b, new version use -m Wallpaper as para
    p.setArguments(QStringList()<<"-m" <<"Wallpaper");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/ukui-control-center", QStringList()<<"-m" <<"Wallpaper");
#endif
    p.waitForFinished(-1);
}

void PeonyDesktopApplication::gotoSetResolution()
{
    QProcess p;
    p.setProgram("/usr/bin/ukui-control-center");
    p.setArguments(QStringList()<<"-m" <<"display");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/ukui-control-center", QStringList()<<"-m" <<"display");
#endif
    p.waitForFinished(-1);

}

void PeonyDesktopApplication::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Close the peony desktop window"));
    parser.addOption(quitOption);

    QCommandLineOption daemonOption(QStringList()<<"d"<<"daemon", tr("Take over the dbus service."));
    parser.addOption(daemonOption);

    QCommandLineOption desktopOption(QStringList()<<"w"<<"desktop-window", tr("Take over the desktop displaying"));
    parser.addOption(desktopOption);

    QCommandLineOption backgroundOption(QStringList()<<"b"<<"background", tr("Setup backgrounds"));
//    parser.addOption(backgroundOption);

    QCommandLineOption clearIconsOption(QStringList()<<"c"<<"clear-icons", tr("Clear standard icons"));
    parser.addOption(clearIconsOption);

    QCommandLineOption layoutOption(QStringList()<<"l"<<"layout-items", tr("Layout item with top to bottom left to right"));
    parser.addOption(layoutOption);

    //story28307, 28308, force update background
    QCommandLineOption UpdateOption(QStringList()<<"u"<<"update-background", tr("Force update backgrounds"));
    parser.addOption(UpdateOption);

#ifdef TEST_PEONY_DESKTOP_WIDGETS
    QCommandLineOption addWidgetOption(QStringList()<<"a"<<"add-widgets", tr("Add desktop widget to desktop"));
    parser.addOption(addWidgetOption);
#endif

    if (isPrimary) {
        if (m_first_parse) {
            auto helpOption = parser.addHelpOption();
            auto versionOption = parser.addVersionOption();
            m_first_parse = false;
        }

        const QStringList args = QString(msg).split(' ');

        parser.process(args);
        if (parser.isSet(quitOption)) {
            QTimer::singleShot(1000, this, [=]() {
                if (desktopManger) {
                    delete desktopManger;
                    desktopManger = nullptr;
                }
                qApp->quit();
            });
            return;
        }

        if (parser.isSet(daemonOption)) {
            if (!has_daemon) {
                has_daemon = true;
                qDebug()<<"-d";

                trySetDefaultFolderUrlHandler();

                //FIXME: take over org.freedesktop.FileManager1
                Peony::FMDBusService *service = new Peony::FMDBusService(this);
                connect(service, &Peony::FMDBusService::showItemsRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/peony");
                    p.setArguments(QStringList()<<"--show-items"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/peony", QStringList()<<"--show-items"<<urisList, nullptr);
#endif
                });
                connect(service, &Peony::FMDBusService::showFolderRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/peony");
                    p.setArguments(QStringList()<<"--show-folders"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<<urisList, nullptr);
#endif
                });
                connect(service, &Peony::FMDBusService::showItemPropertiesRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/peony");
                    p.setArguments(QStringList()<<"--show-properties"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/peony", QStringList()<<"--show-properties"<<urisList, nullptr);
#endif
                });
            }
            has_daemon = true;
        }

        if (parser.isSet(desktopOption)) {
            if(!has_background) {
                QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
                watcher->setConnection(QDBusConnection::sessionBus());
                watcher->addWatchedService("org.ukui.KWin");
                connect(watcher, &QDBusServiceWatcher::serviceRegistered, desktopManger, &DesktopWindowManager::raiseWid);
            }
            setupBgAndDesktop();
        }

        if (parser.isSet(backgroundOption)) {
            setupBgAndDesktop();
        }

        if (parser.isSet(clearIconsOption)) {
            if (has_desktop) {
                clearIcons(args);
            }
        }
        if (parser.isSet(layoutOption)) {
            desktopManger->layoutViewItems();
        }

        if (parser.isSet(UpdateOption)) {
            //force update background use gsettings pictureFile
            //DesktopBackgroundManager::globalInstance()->forceUpdateBackground();
            BackgroundManager::globalInstance()->forceUpdateBackground();
        }



#ifdef TEST_PEONY_DESKTOP_WIDGETS
        if (parser.isSet(addWidgetOption)) {
            QInputDialog dialog;
            dialog.setWindowTitle("Select widget to add");
            dialog.setLabelText("Widget id:");
            QStringList widgetIds;
            UkuiQuick::WidgetLoader::globalLoader().addWidgetSearchPath(":/");
            UkuiQuick::WidgetLoader::globalLoader().addWidgetSearchPath(":/desktop-widgets/internal-widgets");
            UkuiQuick::WidgetLoader::globalLoader().addWidgetSearchPath("/usr/share/ukui/widgets");
            auto widgetMetadatas = UkuiQuick::WidgetLoader::globalLoader().widgetsMetadata();
            for (auto metadata : widgetMetadatas) {
                widgetIds << metadata.id();
            }
            qDebug() << "available widgets:" << widgetIds;
            dialog.setComboBoxItems(widgetIds);
            dialog.exec();
            QString id = dialog.textValue();
            if (!id.isEmpty()) {
                int screenId = QInputDialog::getInt(0, "Input screen id for adding widget to", "Screen id:", 0, 0, qApp->screens().count() - 1);
                if (screenId >= 0) {
                    for (auto platformwindow : PlatformWindow::allWindows()) {
                        if (platformwindow->id() == screenId) {
                            platformwindow->mainView()->addWidget(id, DesktopWidgetsManager::getInstance()->getNextInstanceId());
                        }
                    }
                }
            }
        }
#endif

        connect(this, &QApplication::paletteChanged, this, [=](const QPalette &pal) {
            for (auto w : allWidgets()) {
                w->setPalette(pal);
                w->update();
            }
        });
    }
    else {
        auto helpOption = parser.addHelpOption();
        auto versionOption = parser.addVersionOption();

        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());

        sendMessage(msg);
    }
}

void PeonyDesktopApplication::clearIcons(const QStringList &args)
{
    //定制需求
    int index = -1;
    if (args.contains("-c")) {
        index = args.indexOf("-c");

    } else if (args.contains("--clear-icons")) {
        index = args.indexOf("--clear-icons");
    }

    index++;
    if ((index <= 0) || (index >= args.count())) {
        return;
    }

    const QString &operationStr = args.value(index);
    bool isSuccess = false;
    int operationNum = operationStr.toInt(&isSuccess);
//    for (auto window : m_bg_windows) {
//        if (!window->getIconView()) {
//            return;
//        }
//    }
    if (isSuccess) {
        switch (operationNum) {
            case 1:
                GlobalSettings::getInstance()->setValue(DISPLAY_STANDARD_ICONS, false);
                break;
            default:
                GlobalSettings::getInstance()->setValue(DISPLAY_STANDARD_ICONS, true);
        }
    }
}

void PeonyDesktopApplication::setupDesktop()
{
    DesktopWidgetsManager::getInstance()->getNextInstanceId();
    qmlRegisterType<WidgetsProxyModel>("org.ukui.peony.desktop", 1, 0, "WidgetsProxyModel");
    qmlRegisterType<Peony::DesktopItemProxyModel>("org.ukui.peony.desktop", 1, 0, "DesktopItemProxyModel");
    qmlRegisterSingletonInstance<DesktopOrganizationManager>("org.ukui.peony.desktop", 1, 0, "DesktopOrganizationManager", DesktopOrganizationManager::instance());
    qmlRegisterSingletonInstance<AdvancedDesktopItemModel>("org.ukui.peony.desktop", 1, 0, "DesktopItemModelInstance", PeonyDesktopApplication::getModel());
    qmlRegisterSingletonInstance<ThemeStyle>("org.ukui.peony.desktop", 1, 0, "ThemeStyle",ThemeStyle::instance());
    qmlRegisterSingletonInstance<FileActionController>("org.ukui.peony.desktop", 1, 0, "Controller",FileActionController::getInstance());
    qmlRegisterSingletonInstance<DesktopUKUIpluginnManager>("org.ukui.peony.desktop", 1, 0, "DesktopUKUIpluginnManager", DesktopUKUIpluginnManager::instance());
    // WidgetContainer 与 loadWidget/widgetsMetadata 使用的 globalLoader 需配置相同搜索路径，
    auto configureWidgetSearchPaths = [](UkuiQuick::WidgetLoader &loader) {
        loader.addWidgetSearchPath(QStringLiteral(":/"));
        loader.addWidgetSearchPath(QStringLiteral(":/desktop-widgets/internal-widgets"));
        loader.addWidgetSearchPath(QStringLiteral("/usr/share/ukui/widgets"));
        const QByteArray extra = qgetenv("UKUI_WIDGET_SEARCH_PATH");
        if (!extra.isEmpty()) {
            const QStringList parts = QString::fromLocal8Bit(extra).split(QLatin1Char(':'), QString::SkipEmptyParts);
            for (const QString &dir : parts) {
                const QFileInfo fi(dir);
                if (fi.isDir()) {
                    loader.addWidgetSearchPath(fi.absoluteFilePath());
                }
            }
        }
    };
    configureWidgetSearchPaths(UkuiQuick::WidgetContainer::widgetLoader());
    configureWidgetSearchPaths(UkuiQuick::WidgetLoader::globalLoader());
    // Register PAGView for dynamic wallpaper rendering (play-once, fills parent viewport)
    qmlRegisterType<pag::PAGView>("org.ukui.peony.desktop.pag", 1, 0, "PAGView");
    //DesktopBackgroundManager::globalInstance();
    DESKTOP_MANAGER->initializeWithScreens();
}

void PeonyDesktopApplication::setupBgAndDesktop()
{
    if (!has_background) {
        setupDesktop();
    }
    has_background = true;
}

void PeonyDesktopApplication::clearViewCache()
{
//    for (auto bgWindow : m_bg_windows) {
//        auto view = bgWindow->getIconView();
//        view->clearCache();
//    }
}
