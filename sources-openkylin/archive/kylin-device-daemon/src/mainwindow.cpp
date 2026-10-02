 /*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QColor>
#include <KWindowEffects>
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include <windowmanager/windowmanager.h>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>
#include <ukuistylehelper/ukuistylehelper.h> 
#include <kysdk/kysdk-system/libkysysinfo.h>
#include "ksoundeffects.h"
using namespace kdk;

#include "clickLabel.h"
#include "MacroFile.h"
#include "datacdrom.h"

#if UDFAUTOMOUNT
static void mount_cdrom (GDrive* drive, gpointer u);
static void eject_cdrom (GDrive* drive, gpointer u);

static QMap<QString, bool>* gMountCdrom = nullptr;
static QMap<QString, QString>* gMountCdromMountPoint = nullptr;
#endif

// ukey
static char const* const gSpecialUSB[] = {
    "FT ePass3000GM",
    "GASEC GAUKEY",
    NULL,
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , findPointMount(false)
  , telephoneNum(0)
  , driveVolumeNum(0)
  , m_strSysRootDev(QString(""))
{
    ui->setupUi(this);

    initThemeMode();
    initFlashDisk();// 初始化了m_basewidget
    initSlots();
    installEventFilter(this);
    initPanelGSettings(); //设置了m_basewidget对象内部用于控制划入动画的参数m_panelPosition：setPanelPosition(m_panelPosition);
    initPanelStyle();

    // get system root device
    getSystemRootDev();
    // underlying code to get the information of the usb device
    getDeviceInfo();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_transparency_gsettings) {
        delete m_transparency_gsettings;
        m_transparency_gsettings = nullptr;
    }

    if (ifsettings) {
        delete ifsettings;
        ifsettings = nullptr;
    }
    QMap<QString, FDFrame *>().swap(centralWidgetList);

    if(!mRepairDialog.isEmpty()) {
        qDeleteAll(mRepairDialog);
    }
    if(!centralWidgetList.isEmpty()) {
        qDeleteAll(centralWidgetList);
    }

}

void MainWindow::initFlashDisk()
{
    m_vtDeviveId.clear();

    // 框架的样式设置
    // set the style of the framework
    interfaceHideTime = new QTimer(this);
    interfaceHideTime->setSingleShot(true);

    initTransparentState();
    //ui->centralWidget = new FDFrame();
    //ui->centralWidget->setObjectName("centralWidget");
    //vboxlayout = new QVBoxLayout(this);
    //ui->centralWidget->setLayout(vboxlayout);

#if (QT_VERSION < QT_VERSION_CHECK(5,7,0))
    //this->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
#else
    //this->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
#endif
    //this->setAttribute(Qt::WA_TranslucentBackground);

    m_systray = new QSystemTrayIcon(this);
    m_systray->setIcon(QIcon::fromTheme("drive-removable-media-usb-symbolic"));
    //m_systray->setVisible(true);
    m_systray->setToolTip(tr("usb management tool"));

    m_basewidget = new baseWidget();

    // init the screen
    screen = qApp->primaryScreen();

    m_nAppStartTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_dataFlashDisk = FlashDiskData::getInstance();
}

void MainWindow::initSlots()
{
    connect(m_systray, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
    connect(this, &MainWindow::convertShowWindow, this, &MainWindow::onConvertShowWindow);
    connect(this, &MainWindow::convertUpdateWindow, this, &MainWindow::onConvertUpdateWindow);
    connect(this, &MainWindow::remountVolume, this, &MainWindow::onRemountVolume);
    connect(this, &MainWindow::checkDriveValid, this, &MainWindow::onCheckDriveValid);
    connect(this, &MainWindow::notifyDeviceRemoved, this, &MainWindow::onNotifyDeviceRemoved);
    connect(m_dataFlashDisk, &FlashDiskData::notifyDeviceRemoved, this, &MainWindow::onNotifyDeviceRemoved);

    QDBusConnection::sessionBus().connect(QString(), QString("/taskbar/click"), \
                                                  "com.ukui.panel.plugins.taskbar", "sendToUkuiDEApp", this, SLOT(on_clickPanelToHideInterface()));

    connect(this, SIGNAL(deviceError(GDrive*)), this, SLOT(onDeviceErrored(GDrive*)), (Qt::UniqueConnection));
    connect(this, SIGNAL(mountVolume(GVolume*)), this, SLOT(onMountVolume(GVolume*)));
    connect(this, SIGNAL(ejectVolumeForce(GVolume*)), this, SLOT(onEjectVolumeForce(GVolume*)));
    connect(interfaceHideTime, SIGNAL(timeout()), this, SLOT(onMaininterfacehide()));
    
    QDBusConnection::systemBus().connect(
        "org.freedesktop.UDisks2",
        QString(),
        "org.freedesktop.UDisks2.Filesystem",
        "UsbSpeedTested",
        this,
        SLOT(onUsbSpeedTested(QString, double, bool))
    );
    
    //监听屏幕改变的信号
    connect(QApplication::desktop(), &QDesktopWidget::resized, this, [=]() {
        QTimer::singleShot(200, this, [=]() {
            qInfo()<<"--------screen resize-------";
            if(!m_basewidget->isHidden()){
                m_basewidget->showBaseWindow();
                moveBaseWidgetGeo();
            }
        });
    });
}

void MainWindow::initPanelStyle() {
    if (!m_basewidget) {
        return ;
    }
    m_basewidget->setupWindowEffects();
}

void MainWindow::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
            m_panelGSettings = new QGSettings(id);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            m_basewidget->setPanelPosition(m_panelPosition);
        }
        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
                m_basewidget->setPanelPosition(m_panelPosition);
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
        });
    }

}

void MainWindow::onRequestSendDesktopNotify(QString message, QString strIcon, QString notifySound)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    QMap <QString,QVariant> hint;
    if(strIcon == ""){
        strIcon = "drive-removable-media-usb";    //加密盘无法获取图标，使用默认图标
    }
    hint.insert("sound-name",notifySound);
    args << (tr("kylin device daemon"))
         << ((unsigned int) 0)
         << strIcon
         << tr("kindly reminder") //显示的是什么类型的信息
         << message //显示的具体信息
         << QStringList()
         << hint
         << (int)-1;
    iface.callWithArgumentList(QDBus::NoBlock,"Notify",args);
}

void MainWindow::onInsertAbnormalDiskNotify(QString message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args << (tr("kylin device daemon"))
         << ((unsigned int) 0)
         << QString("media-removable-symbolic")
         << tr("wrong reminder") //显示的是什么类型的信息
         << message //显示的具体信息
         << QStringList()
         << QVariantMap()
         << (int)-1;
    iface.callWithArgumentList(QDBus::NoBlock,"Notify",args);
}

void MainWindow::onNotifyWnd(QObject* obj, QEvent *event)
{
     if (obj == m_basewidget){
            if(insertorclick){
                if (event->type() == QEvent::Enter) {
                    interfaceHideTime->stop();
                } else if (event->type() == QEvent::Leave) {
                    interfaceHideTime->start(2000);

                }
            }
     }
}

void MainWindow::initThemeMode()
{
    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd)) {
        QGSettings *qtSettings = new QGSettings(idd, QByteArray(), this);

        connect(qtSettings,&QGSettings::changed,this,[=](const QString &key) {
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        });
        currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    }

    const QByteArray id(AUTOLOAD);
    if(QGSettings::isSchemaInstalled(idd)) {
        ifsettings = new QGSettings(id);
    }
}

void MainWindow::on_clickPanelToHideInterface()
{
    if(!m_basewidget->isHidden()) {
        m_basewidget->hide();
    }
}

void MainWindow::getSystemRootDev()
{
    QString cmd = "df -l";
    QProcess *p = new QProcess();
    p->start(cmd);
    p->waitForFinished();
    while(p->canReadLine()) {
        QString str = p->readLine();
        QStringList infoList = str.split(QRegExp("\\s+"));
        if (infoList.size() >= 6) {
            if (infoList[5] == "/") {
                m_strSysRootDev = infoList[0];
                break;
            }
        }
    }
    delete p;
    p = nullptr;
}

bool MainWindow::isSystemRootDev(QString strDev)
{
    return m_strSysRootDev.startsWith(strDev);
}

bool MainWindow::ifSataDev(QString strDev)
{
    char* chDev;
    QByteArray ba = strDev.toLatin1();
    chDev=ba.data();

    QString cmd = "udevadm info -n ";
    cmd.append(strDev);
    QProcess *p = new QProcess();
    p->start(cmd);
    p->waitForFinished();
    while(p->canReadLine()) {
        QString str = p->readLine();
        if(g_str_has_prefix(str.toLatin1().data(),"E: ID_PATH")){
          QStringList infoList = str.split('-');
          QStringList::Iterator it = infoList.begin();
          for (; it != infoList.end(); it++){
              if((*it=="ata")&&(*it=="nvme")&&(!g_str_has_prefix(chDev,"/dev/sr"))){
                  qDebug()<<"---is sata or nvme-"<<(*it);
                  return true;
              }
          }
        }
    }

    delete p;
    p = nullptr;
    return false;
}

bool MainWindow::ifHasFsType(QString strDev)
{
    char* chDev;
    QByteArray ba = strDev.toLatin1();
    chDev=ba.data();

    QString cmd = "udevadm info -n ";
    cmd.append(strDev);
    QProcess *p = new QProcess();
    p->start(cmd);
    p->waitForFinished();

    while(p->canReadLine()) {
        QString str = p->readLine();
        QStringList infoList = str.split('=');
        QStringList::Iterator it = infoList.begin();
        for (; it != infoList.end(); it++){
            if(*it==("E: ID_FS_TYPE")){
                //qDebug()<<"---"<<strDev<<"have FS type-"<<(*it);
                return true;
            }
        }
    }

    delete p;
    p = nullptr;
    return false;
}

QString MainWindow::getFileSystemType(QString strVolId)
{
    QString fsType = "";

    QString dbusPath = convertToDBusPath(strVolId);
    qInfo() << "Getting filesystem type for" << strVolId << "using DBus path:" << dbusPath;

    if (! QDBusConnection::systemBus().isConnected())
        return fsType;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Block",
                                  QDBusConnection::systemBus());

    if(blockInterface.isValid())
        fsType = blockInterface.property("IdType").toString();

    return fsType;

}

void MainWindow::getDeviceInfo()
{
    // setting
    FILE *fp = NULL;
    int a = 0;
    char buf[128] = {0};

    fp = fopen("/proc/cmdline","r");
    if(fp) {
        while(fscanf(fp,"%127s",buf) >0 ) {
            if(strcmp(buf,"live") == 0) {
                a++;
            }
        }
        fclose(fp);
    }
    if(a > 0) {
        QProcess::startDetached("gsettings set org.kylin.device-daemon.autoload ifautoload false");
    }

    // callback function that to monitor the insertion and removal of the underlying equipment
    GVolumeMonitor *g_volume_monitor = g_volume_monitor_get();
    g_signal_connect (g_volume_monitor, "drive-connected", G_CALLBACK (drive_connected_callback), this);
    g_signal_connect (g_volume_monitor, "drive-disconnected", G_CALLBACK (drive_disconnected_callback), this);
    g_signal_connect (g_volume_monitor, "volume-added", G_CALLBACK (volume_added_callback), this);
    g_signal_connect (g_volume_monitor, "volume-removed", G_CALLBACK (volume_removed_callback), this);
    g_signal_connect (g_volume_monitor, "mount-added", G_CALLBACK (mount_added_callback), this);
    g_signal_connect (g_volume_monitor, "mount-removed", G_CALLBACK (mount_removed_callback), this);

    GList *lDrive = NULL, *lVolume = NULL, *lMount = NULL;
    // about drive
    GList *current_drive_list = g_volume_monitor_get_connected_drives(g_volume_monitor);
    for (lDrive = current_drive_list; lDrive != NULL; lDrive = lDrive->next) {
        GDrive *gdrive = (GDrive *)lDrive->data;
        char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            FDDriveInfo driveInfo;
            driveInfo.strId = devPath;
            char *strName = g_drive_get_name(gdrive);
            if (strName) {
                driveInfo.strName = strName;
                g_free(strName);
            }
            getDriveIconsInfo(gdrive, driveInfo);
            unsigned uVolumeSize = 0;
            GList* Volumes = g_drive_get_volumes(gdrive);
            if (Volumes) {
                uVolumeSize = g_list_length(Volumes);
                g_list_free(Volumes);
            }
            driveInfo.isCanEject = g_drive_can_eject(gdrive);
            driveInfo.isCanStop = g_drive_can_stop(gdrive);
            driveInfo.isCanStart = g_drive_can_start(gdrive);
            driveInfo.isRemovable = g_drive_is_removable(gdrive);
            driveInfo.lluVolSum = uVolumeSize;
            if(!isSystemRootDev(driveInfo.strId.c_str()) && !ifSataDev(driveInfo.strId.c_str()) && (driveInfo.isCanEject || driveInfo.isCanStop || driveInfo.isRemovable)) {
                if(g_str_has_prefix(devPath,"/dev/sr") || g_str_has_prefix(devPath,"/dev/bus") || g_str_has_prefix(devPath,"/dev/sd")
                    || g_str_has_prefix(devPath,"/dev/mmcblk")) {
                    GList* gdriveVolumes = g_drive_get_volumes(gdrive);
                    if (gdriveVolumes) {
                        for(lVolume = gdriveVolumes; lVolume != NULL; lVolume = lVolume->next){ //遍历驱动器上的所有卷设备
                            GVolume* volume = (GVolume *)lVolume->data;
                            FDVolumeInfo volumeInfo;
                            bool isValidMount = true;
                            char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                            if (volumeId) {
                                volumeInfo.strId = volumeId;
                                g_free(volumeId);
                            } else {
                                continue ;
                            }
                            char *volumeName = g_volume_get_name(volume);
                            if (volumeName) {
                                volumeInfo.strName = volumeName;
                                g_free(volumeName);
                            }
                            char *strDevName = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                            if (strDevName) {
                                volumeInfo.strDevName = strDevName;
                                g_free(strDevName);
                            }
                            getVolumeIconsInfo(volume, volumeInfo);
                            volumeInfo.isCanMount = g_volume_can_mount(volume);
                            volumeInfo.isCanEject = g_volume_can_eject(volume);
                            volumeInfo.isShouldAutoMount = g_volume_should_automount(volume);
                            GMount* mount = g_volume_get_mount(volume); //get当前卷设备的挂载信息
                            if (mount) {                  //该卷设备已挂载
                                volumeInfo.mountInfo.isCanEject = g_mount_can_eject(mount);
                                volumeInfo.mountInfo.isCanUnmount = g_mount_can_unmount(mount);
                                if (volumeInfo.mountInfo.isCanEject || volumeInfo.mountInfo.isCanUnmount) {
                                    char *mountId = g_mount_get_uuid(mount);
                                    if (mountId) {
                                        volumeInfo.mountInfo.strId = mountId;
                                        g_free(mountId);
                                    }
                                    char *mountName = g_mount_get_name(mount);
                                    if (mountName) {
                                        volumeInfo.mountInfo.strName = mountName;
                                        g_free(mountName);
                                    }
                                    // get mount total size
                                    GFile *fileRoot = g_mount_get_root(mount);
                                    if (fileRoot) {
                                        GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                                        if (info) {
                                            volumeInfo.mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                                            g_object_unref(info);
                                        }
                                        g_object_unref(fileRoot);
                                    }
                                    getDataCDRomCapacity(volumeInfo.strId.c_str(),volumeInfo.mountInfo.lluTotalSize);
                                    // get mount uri
                                    GFile *root = g_mount_get_default_location(mount);
                                    if (root) {
                                        volumeInfo.mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                                        char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                                        if (mountUri) {
                                            volumeInfo.mountInfo.strUri = mountUri;
                                            if (g_str_has_prefix(mountUri,"file:///data")) {
                                                isValidMount = false;
                                            } else {
                                                if (volumeInfo.mountInfo.strId.empty()) {
                                                    volumeInfo.mountInfo.strId = volumeInfo.mountInfo.strUri;
                                                }
                                            }
                                            g_free(mountUri);
                                        }
                                        char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                                        if (tooltip) {
                                            volumeInfo.mountInfo.strTooltip =tooltip;
                                            g_free(tooltip);
                                        }
                                        g_object_unref(root);
                                    }
                                    getMountIconsInfo(mount, volumeInfo.mountInfo);
                                }
                                g_object_unref(mount);
                            } else {
                                if(ifsettings->get(IFAUTOLOAD).toBool()) {
                                    g_volume_mount(volume,
                                            G_MOUNT_MOUNT_NONE,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr);
                                }
                            }
                            if (isValidMount) {
                                driveInfo.listVolumes[volumeInfo.strId] = volumeInfo;
                            }
                        }
                        g_list_free(gdriveVolumes);
                    }
                    m_dataFlashDisk->addDriveInfo(driveInfo);
#if UDFAUTOMOUNT
                    if (G_IS_DRIVE(gdrive) && !driveInfo.strId.empty() && g_str_has_prefix (driveInfo.strId.c_str(), "/dev/sr")) {
                        mount_cdrom (gdrive, NULL);
                        g_signal_connect(G_DRIVE(gdrive), "changed", G_CALLBACK (mount_cdrom), NULL);
                    }
#endif
                }
            }
            g_free(devPath);
        }
    }
    if (current_drive_list) {
        g_list_free(current_drive_list);
    }
    // about volume not associated with a drive
    GList *current_volume_list = g_volume_monitor_get_volumes(g_volume_monitor);
    if (current_volume_list) {
        for (lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next) {
            GVolume *volume = (GVolume *)lVolume->data;
            GDrive *gdrive = g_volume_get_drive(volume);
            if (!gdrive) {
                FDVolumeInfo volumeInfo;
                bool isValidMount = true;
                char *devPath = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                if (devPath) {
                    if (!(g_str_has_prefix(devPath,"/dev/sr") || g_str_has_prefix(devPath,"/dev/bus") || g_str_has_prefix(devPath,"/dev/sd")
                        || g_str_has_prefix(devPath,"/dev/mmcblk"))) {
                        g_free(devPath);
                        continue;
                    }
                    g_free(devPath);
                }
                char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                if (volumeId) {
                    volumeInfo.strId = volumeId;
                    g_free(volumeId);
                } else {
                    continue ;
                }
                char *volumeName = g_volume_get_name(volume);
                if (volumeName) {
                    volumeInfo.strName = volumeName;
                    g_free(volumeName);
                }
                char *strDevName = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                if (strDevName) {
                    volumeInfo.strDevName = strDevName;
                    g_free(strDevName);
                }
                getVolumeIconsInfo(volume, volumeInfo);
                volumeInfo.isCanMount = g_volume_can_mount(volume);
                volumeInfo.isCanEject = g_volume_can_eject(volume);
                volumeInfo.isShouldAutoMount = g_volume_should_automount(volume);
                GMount* mount = g_volume_get_mount(volume); //get当前卷设备的挂载信息
                if (mount) {                  //该卷设备已挂载
                    volumeInfo.mountInfo.isCanEject = g_mount_can_eject(mount);
                    volumeInfo.mountInfo.isCanUnmount = g_mount_can_unmount(mount);
                    if (volumeInfo.mountInfo.isCanEject || volumeInfo.mountInfo.isCanUnmount) {
                        char *mountId = g_mount_get_uuid(mount);
                        if (mountId) {
                            volumeInfo.mountInfo.strId = mountId;
                            g_free(mountId);
                        }
                        char *mountName = g_mount_get_name(mount);
                        if (mountName) {
                            volumeInfo.mountInfo.strName = mountName;
                            g_free(mountName);
                        }
                        // get mount total size
                        GFile *fileRoot = g_mount_get_root(mount);
                        if (fileRoot) {
                            GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                            if (info) {
                                volumeInfo.mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                                g_object_unref(info);
                            }
                            g_object_unref(fileRoot);
                        }
                        getDataCDRomCapacity(volumeInfo.strId.c_str(),volumeInfo.mountInfo.lluTotalSize);
                        // get mount uri
                        GFile *root = g_mount_get_default_location(mount);
                        if (root) {
                            volumeInfo.mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                            char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                            if (mountUri) {
                                volumeInfo.mountInfo.strUri = mountUri;
                                if (g_str_has_prefix(mountUri,"file:///data")) {
                                    isValidMount = false;
                                } else {
                                    if (volumeInfo.mountInfo.strId.empty()) {
                                        volumeInfo.mountInfo.strId = volumeInfo.mountInfo.strUri;
                                    }
                                }
                                g_free(mountUri);
                            }
                            char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                            if (tooltip) {
                                volumeInfo.mountInfo.strTooltip =tooltip;
                                g_free(tooltip);
                            }
                            g_object_unref(root);
                        }
                        getMountIconsInfo(mount, volumeInfo.mountInfo);
                    }
                    g_object_unref(mount);
                } else {
                    if (volumeInfo.isCanMount) {
                        if(ifsettings->get(IFAUTOLOAD).toBool()) {
                            g_volume_mount(volume,
                                    G_MOUNT_MOUNT_NONE,
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    nullptr);
                        }
                    }
                }
                if (isValidMount) {
                    m_dataFlashDisk->addVolumeInfo(volumeInfo);
                }
            } else {
                g_object_unref(gdrive);
            }
        }
        g_list_free(current_volume_list);
    }

    // about mount not associated with a volume
    GList *current_mount_list = g_volume_monitor_get_mounts(g_volume_monitor);
    if (current_mount_list) {
        for (lMount = current_mount_list; lMount != NULL; lMount = lMount->next) {
            GMount *gmount = (GMount *)lMount->data;
            GVolume *gvolume = g_mount_get_volume(gmount);
            if (!gvolume) {
                GDrive* gdrive = g_mount_get_drive(gmount);
                if (gdrive) {
                    char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
                    if (devPath != NULL) {
                        FDDriveInfo driveInfo;
                        driveInfo.strId = devPath;
                        char *strName = g_drive_get_name(gdrive);
                        if (strName) {
                            driveInfo.strName = strName;
                            g_free(strName);
                        }
                        getDriveIconsInfo(gdrive, driveInfo);
                        driveInfo.isCanEject = g_drive_can_eject(gdrive);
                        driveInfo.isCanStop = g_drive_can_stop(gdrive);
                        driveInfo.isCanStart = g_drive_can_start(gdrive);
                        driveInfo.isRemovable = g_drive_is_removable(gdrive);
                        if(!isSystemRootDev(driveInfo.strId.c_str()) && (driveInfo.isCanEject || driveInfo.isCanStop || driveInfo.isRemovable)) {
                            if(g_str_has_prefix(devPath,"/dev/sr") || g_str_has_prefix(devPath,"/dev/bus") || g_str_has_prefix(devPath,"/dev/sd")
                                || g_str_has_prefix(devPath,"/dev/mmcblk")) {
                                FDMountInfo mountInfo;
                                bool isValidMount = true;
                                mountInfo.isCanEject = g_mount_can_eject(gmount);
                                mountInfo.isCanUnmount = g_mount_can_unmount(gmount);
                                if (mountInfo.isCanEject || mountInfo.isCanUnmount) {
                                    char *mountId = g_mount_get_uuid(gmount);
                                    if (mountId) {
                                        mountInfo.strId = mountId;
                                        g_free(mountId);
                                    }
                                    char *mountName = g_mount_get_name(gmount);
                                    if (mountName) {
                                        mountInfo.strName = mountName;
                                        g_free(mountName);
                                    }
                                    // get mount total size
                                    GFile *fileRoot = g_mount_get_root(gmount);
                                    if (fileRoot) {
                                        GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                                        if (info) {
                                            mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                                            g_object_unref(info);
                                        }
                                        g_object_unref(fileRoot);
                                    }
                                    getDataCDRomCapacity(driveInfo.strId.c_str(),mountInfo.lluTotalSize);
                                    // get mount uri
                                    GFile *root = g_mount_get_default_location(gmount);
                                    if (root) {
                                        mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                                        char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                                        if (mountUri) {
                                            mountInfo.strUri = mountUri;
                                            if (g_str_has_prefix(mountUri,"file:///data")) {
                                                isValidMount = false;
                                            } else {
                                                if (mountInfo.strId.empty()) {
                                                    mountInfo.strId = mountInfo.strUri;
                                                }
                                            }
                                            g_free(mountUri);
                                        }
                                        char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                                        if (tooltip) {
                                            mountInfo.strTooltip =tooltip;
                                            g_free(tooltip);
                                        }
                                        g_object_unref(root);
                                    }
                                    getMountIconsInfo(gmount, mountInfo);
                                    if (isValidMount) {
                                        FDVolumeInfo volumeInfo;
                                        volumeInfo.mountInfo = mountInfo;
                                        m_dataFlashDisk->addMountInfoWithDrive(driveInfo, volumeInfo, mountInfo);
                                    }
                                }
                            }
                        }
                        g_free(devPath);
                    }
                    g_object_unref(gdrive);
                } else {
                    # if 0
                    FDMountInfo mountInfo;
                    bool isValidMount = true;
                    mountInfo.isCanEject = g_mount_can_eject(gmount);
                    mountInfo.isCanUnmount = g_mount_can_unmount(gmount);
                    if (mountInfo.isCanEject || mountInfo.isCanUnmount) {
                        char *mountId = g_mount_get_uuid(gmount);
                        if (mountId) {
                            mountInfo.strId = mountId;
                            g_free(mountId);
                        }
                        char *mountName = g_mount_get_name(gmount);
                        if (mountName) {
                            mountInfo.strName = mountName;
                            g_free(mountName);
                        }
                        // get mount total size
                        GFile *fileRoot = g_mount_get_root(gmount);
                        if (fileRoot) {
                            GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                            if (info) {
                                mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                                g_object_unref(info);
                            }
                            g_object_unref(fileRoot);
                        }
                        getDataCDRomCapacity(mountInfo.strId.c_str(),mountInfo.lluTotalSize);
                        // get mount uri
                        GFile *root = g_mount_get_default_location(gmount);
                        if (root) {
                            mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                            char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                            if (mountUri) {
                                mountInfo.strUri = mountUri;
                                if (g_str_has_prefix(mountUri,"file:///data")) {
                                    isValidMount = false;
                                } else {
                                    if (mountInfo.strId.empty()) {
                                        mountInfo.strId = mountInfo.strUri;
                                    }
                                }
                                g_free(mountUri);
                            }
                            char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                            if (tooltip) {
                                mountInfo.strTooltip =tooltip;
                                g_free(tooltip);
                            }
                            g_object_unref(root);
                        }
                        if (isValidMount) {
                            m_dataFlashDisk->addMountInfo(mountInfo);
                        }
                    }
                    #endif
                }
            } else {
                g_object_unref(gvolume);
            }
        }
        g_list_free(current_mount_list);
    }

    // determine the systray icon should be showed  or be hieded
    if(m_dataFlashDisk->getValidInfoCount() >= 1) {
        m_systray->show();
    } else {
        m_systray->hide();
    }
    m_dataFlashDisk->OutputInfos();
    
    autoMountLocalDrivers();
}

void MainWindow::onConvertShowWindow(QString strDriveId, QString strMountUri)
{
    // 进程启动时一定时间内不弹窗提示
    if (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_nAppStartTimestamp < NEWINFO_DELAYSHOW_TIME) {
        m_dataFlashDisk->resetAllNewState();
        return;
    }
    insertorclick = true;

    qDebug()<<"--------the frist insert--MainWindowShow----------";
    MainWindowShow();
    string strDeviceId = strDriveId.toStdString();
    if (std::find(m_vtDeviveId.begin(), m_vtDeviveId.end(), strDeviceId) == m_vtDeviveId.end()) {
        #if IFDISTINCT_DEVICON
        QString strIcon = m_dataFlashDisk->getVolumeIcon(strDriveId);
//        onRequestSendDesktopNotify(tr("Please do not pull out the storage device when reading or writing"),
//                                        strIcon,"devconnectionsucceeded");
        kdk::KSoundEffects::playSound(SoundType::DEVICE_ADDED);
        #else
        if (strDriveId.startsWith("/dev/sr")) {
            onRequestSendDesktopNotify(tr("Please do not pull out the CDROM when reading or writing"),
                                        QString("media-removable-symbolic"));
        } else if (strDriveId.startsWith("/dev/mmcblk")) {
            onRequestSendDesktopNotify(tr("Please do not pull out the SD Card when reading or writing"),
                                        QString("media-removable-symbolic"));
        } else {
            onRequestSendDesktopNotify(tr("Please do not pull out the USB flash disk when reading or writing"),
                                        QString("drive-removable-media-usb"));
        }
        #endif
        m_vtDeviveId.push_back(strDeviceId);
    }
    
    if (!strMountUri.isEmpty() && strMountUri.startsWith("file://") && strDriveId.startsWith("/dev/sr")) {
        qDebug() << "optical drive detected, checking content type for:" << strMountUri;
        guessContentTypeAndOpenFolder(strMountUri);
    }
}

void MainWindow::onNotifyDeviceRemoved(QString strDevId)
{
    if (strDevId.isEmpty()) {
        return;
    }
    #if IFDISTINCT_DEVICON
    QString strIcon = m_dataFlashDisk->getVolumeIcon(strDevId);
    //onRequestSendDesktopNotify(tr("Storage device removed"), strIcon, "devconnectionbreak");
    //kdk::KSoundEffects::playSound(SoundType::DEVICE_REMOVED);
    #else
    if (strDevId.startsWith("/dev/sr")) {
        onRequestSendDesktopNotify(tr("Storage device removed"),
                                    QString("media-removable-symbolic"));
    } else if (strDevId.startsWith("/dev/mmcblk")) {
        onRequestSendDesktopNotify(tr("Storage device removed"),
                                    QString("media-removable-symbolic"));
    } else {
        onRequestSendDesktopNotify(tr("Storage device removed"),
                                    QString("drive-removable-media-usb"));
    }
    #endif
}

void MainWindow::onUsbSpeedTested(const QString &devicePath, double speedMbps, bool isSlow)
{
    qInfo() << "USB speed test result:" << devicePath 
            << "Speed:" << speedMbps << "MB/s"
            << "Is slow:" << isSlow;
    
    if (!isSlow) {
        return;
    }
    
    QString message = tr("Low performance USB 2.0 device may affect transfer speed.");
    onRequestSendDesktopNotify(message, "drive-removable-media-usb", "");
}

void MainWindow::onConvertUpdateWindow(QString strDevName, unsigned uDevType)
{
    // uDevType: 0 drive , 1 volume, 2 mount
    if (uDevType == 1) {    // not drive detached
        qDebug()<<"----------onConvertUpdateWindow----------";
        insertorclick = false;
        MainWindowShow(true);
    }else if(uDevType ==2){
        if(!(m_basewidget->isHidden()))
            m_basewidget->hide();
    }
}

// the drive-connected callback function the is triggered when the usb device is inseted
void MainWindow::drive_connected_callback(GVolumeMonitor *monitor, GDrive *drive, MainWindow *p_this)
{
    // ukey
    g_autofree char* deviceName = g_drive_get_name (G_DRIVE(drive));
    qInfo() << "[drive_connected_callback] drive add" << deviceName;

    if(p_this->ifSpecicalDev(QString(deviceName))){
        qDebug() << "[drive_connected_callback] Special device detected, returning.";
        return;
    }

    if(p_this->ifsettings->get(IFAUTOLOAD).toBool()) {
        qDebug() << "[drive_connected_callback] Autoload is enabled, processing drive.";
        GList *lVolume = NULL;
        FDDriveInfo driveInfo;
        GDrive *gdrive = (GDrive *)drive;
        unsigned uSubVolumeSize = 0;
        char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            driveInfo.strId = devPath;
            char *strName = g_drive_get_name(gdrive);
            if (strName) {
                driveInfo.strName = strName;
                g_free(strName);
            }
            driveInfo.isCanEject = g_drive_can_eject(gdrive);
            driveInfo.isCanStop = g_drive_can_stop(gdrive);
            driveInfo.isCanStart = g_drive_can_start(gdrive);
            driveInfo.isRemovable = g_drive_is_removable(gdrive);
            g_free(devPath);
        }else{
            qInfo() << "[drive_connected_callback] devpath is empty ";
        }

        if (!g_drive_has_volumes(gdrive)) {
            qDebug() << "[drive_connected_callback] Drive has no volumes, scheduling drive validation.";
            QString platForm = kdk_system_get_hostVirtType();
            if (platForm == "none") {
                qDebug() << "[drive_connected_callback] Physical machine environment detected.";
                QTimer::singleShot(1000, p_this, [&,driveInfo,p_this]() { p_this->onCheckDriveValid(driveInfo); });

            } else {
                qDebug() << "[drive_connected_callback] Cloud environment detected.";
                QTimer::singleShot(9000, p_this, [&,driveInfo,p_this]() { p_this->onCheckDriveValid(driveInfo); });
            }
        }

        lVolume = g_drive_get_volumes(gdrive);
        if (lVolume) {
            uSubVolumeSize = g_list_length(lVolume);
            g_list_free(lVolume);
        }

        p_this->getDriveIconsInfo(gdrive, driveInfo);

        if (!driveInfo.strId.empty()) {
            qDebug() << "[drive_connected_callback] Adding drive info to data manager.";
            p_this->m_dataFlashDisk->addDriveInfo(driveInfo);
        }
        // perhaps uSubVolumeSize is 0 and is ok ?
        // else {
        //     qInfo()<<"wrong disk has intered";
        //     p_this->onInsertAbnormalDiskNotify(tr("There is a problem with this device"));
        // }
#if UDFAUTOMOUNT
        if (G_IS_DRIVE(gdrive) && !driveInfo.strId.empty() && g_str_has_prefix (driveInfo.strId.c_str(), "/dev/sr")) {
            qDebug() << "[drive_connected_callback] Connecting UDFAUTOMOUNT signal for CDROM.";
            g_signal_connect(G_DRIVE (gdrive), "changed", G_CALLBACK (mount_cdrom), NULL);
        }
#endif
    } else {
        qDebug() << "[drive_connected_callback] Autoload is disabled, skipping drive processing.";
    }

    if(p_this->m_dataFlashDisk->getValidInfoCount() >= 1) {
        qDebug() << "[drive_connected_callback] At least one valid device, showing systray icon.";
        p_this->m_systray->show();
    }

    p_this->triggerType = 0;
    p_this->m_dataFlashDisk->OutputInfos();
}

// the drive-disconnected callback function the is triggered when the usb device is pull out
void MainWindow::drive_disconnected_callback (GVolumeMonitor *monitor, GDrive *drive, MainWindow *p_this)
{
    qInfo() << "[drive_disconnected_callback] drive disconnect";

    FDDriveInfo driveInfo;
    if (p_this->shouldIgnoreDeviceDisconnect(drive, driveInfo)) {
        qDebug() << "[drive_disconnected_callback] Device disconnect should be ignored, returning.";
        return;
    }

 #if UDFAUTOMOUNT
    // disconnect udf cdrom
    if (!driveInfo.strId.empty() && G_IS_DRIVE(drive)) {
        g_signal_handlers_disconnect_by_func (drive, (void*) mount_cdrom, NULL);

        qDebug() << "[drive_disconnected_callback] UDFAUTOMOUNT: UDF CDROM disconnected.";
        if (gMountCdrom && gMountCdrom->contains (QString(driveInfo.strId.c_str()))) {
            gMountCdrom->remove (QString(driveInfo.strId.c_str()));
            QStringList mountPoints;
            for (auto val : gMountCdromMountPoint->values()) {
                if (QString(driveInfo.strId.c_str()) == val) {
                    mountPoints << val;
                }
            }
            for (auto m : mountPoints)
                gMountCdromMountPoint->remove (m);
        }
    }
#endif
    vector<string>::iterator itDeviceId = p_this->m_vtDeviveId.begin();
    for (; itDeviceId != p_this->m_vtDeviveId.end();) {
        if (driveInfo.strId == *itDeviceId) {
            itDeviceId = p_this->m_vtDeviveId.erase(itDeviceId);
        } else {
            itDeviceId++;
        }
    }
    if(!p_this->m_clickToEject && !g_str_has_prefix (driveInfo.strId.c_str(), "/dev/sr")){
        qDebug() << "[drive_disconnected_callback] Not manual eject and not CDROM, showing eject interface.";
        if(!p_this->m_basewidget->isHidden())
            p_this->m_basewidget->hide();
        p_this->m_eject = new ejectInterface(NULL, true);
        qDebug() << "[drive_disconnected_callback] Creating eject interface.";
        p_this->m_eject->showEjectInterface();
    }
    qDebug() << "[drive_disconnected_callback] Removing drive info from data manager.";
    p_this->m_dataFlashDisk->removeDriveInfo(driveInfo);
    if(p_this->m_dataFlashDisk->getValidInfoCount() == 0) {
        qDebug() << "[drive_disconnected_callback] No valid devices left, hiding all widgets and systray.";
        p_this->hideAllWidget();
        p_this->m_systray->hide();
    }

    p_this->hideRepairDialog(drive);
    p_this->m_dataFlashDisk->OutputInfos();
}

// when the usb device is identified, we should mount every partition
void MainWindow::volume_added_callback(GVolumeMonitor *monitor, GVolume *volume, MainWindow *p_this)
{
    qInfo() << "[volume_added_callback] volume add";
    GDrive* gdrive = g_volume_get_drive(volume);

    FILE *fp = NULL;
    int a = 0;
    char buf[128] = {0};

    fp = fopen("/proc/cmdline","r");
    if (fp) {
        while(fscanf(fp,"%127s",buf) > 0) {
            if(strcmp(buf,"live") == 0) {
                a++;
            }
        }
        fclose(fp);
    }
    p_this->ifautoload = p_this->ifsettings->get(IFAUTOLOAD).toBool();

    if(a > 0) {
        qDebug() << "[volume_added_callback] Detected 'live' mode, disabling autoload.";
        QProcess::startDetached("gsettings set org.kylin.device-daemon.autoload ifautoload false");
    } else {
        //QProcess::startDetached("gsettings set org.kylin.device-daemon.autoload ifautoload true");
    }

    bool isNewMount = false;
    char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    unsigned uVolumeSize = 0;
    GList* Volumes = g_drive_get_volumes(gdrive);
    if (Volumes) {
        uVolumeSize = g_list_length(Volumes);
        g_list_free(Volumes);
    }
    if((!gdrive)||(devPath == NULL)) {
        qDebug() << "[volume_added_callback] gdrive or devPath is null, handling as volume without drive.";
        FDVolumeInfo volumeInfo;
        bool isValidMount = true;
        char *devPath = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath) {
            if (!(g_str_has_prefix(devPath,"/dev/sr") || g_str_has_prefix(devPath,"/dev/bus") || g_str_has_prefix(devPath,"/dev/sd")
                  || g_str_has_prefix(devPath,"/dev/mmcblk"))) {
                qDebug() << "[volume_added_callback] Device path does not match supported prefixes, returning.";
                g_free(devPath);
                return;
            }
            g_free(devPath);
        }
        char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (volumeId) {
            volumeInfo.strId = volumeId;
            g_free(volumeId);
        } else {
            qDebug() << "[volume_added_callback] volumeId is null, returning.";
            return ;
        }
        char *volumeName = g_volume_get_name(volume);
        if (volumeName) {
            volumeInfo.strName = volumeName;
            g_free(volumeName);
        }
        char *strDevName = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (strDevName) {
            volumeInfo.strDevName = strDevName;
            g_free(strDevName);
        }
        p_this->getVolumeIconsInfo(volume, volumeInfo);
        volumeInfo.isCanMount = g_volume_can_mount(volume);
        volumeInfo.isCanEject = g_volume_can_eject(volume);
        volumeInfo.isShouldAutoMount = g_volume_should_automount(volume);
        GMount* mount = g_volume_get_mount(volume); //get当前卷设备的挂载信息
        if (mount) {                  //该卷设备已挂载
            qDebug() << "[volume_added_callback] Volume is already mounted.";
            volumeInfo.mountInfo.isCanEject = g_mount_can_eject(mount);
            volumeInfo.mountInfo.isCanUnmount = g_mount_can_unmount(mount);
            if (volumeInfo.mountInfo.isCanEject || volumeInfo.mountInfo.isCanUnmount) {
                char *mountId = g_mount_get_uuid(mount);
                if (mountId) {
                    volumeInfo.mountInfo.strId = mountId;
                    g_free(mountId);
                }
                char *mountName = g_mount_get_name(mount);
                if (mountName) {
                    volumeInfo.mountInfo.strName = mountName;
                    g_free(mountName);
                }
                isNewMount = !(p_this->m_dataFlashDisk->isMountInfoExist(volumeInfo.mountInfo));
                // get mount total size
                GFile *fileRoot = g_mount_get_root(mount);
                if (fileRoot) {
                    GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                    if (info) {
                        volumeInfo.mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                        g_object_unref(info);
                    }
                    g_object_unref(fileRoot);
                }
                p_this->getDataCDRomCapacity(volumeInfo.strId.c_str(), volumeInfo.mountInfo.lluTotalSize);
                // get mount uri
                GFile *root = g_mount_get_default_location(mount);
                if (root) {
                    volumeInfo.mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                    char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                    if (mountUri) {
                        volumeInfo.mountInfo.strUri = mountUri;
                        if (g_str_has_prefix(mountUri,"file:///data")) {
                            qDebug() << "[volume_added_callback] Mount URI is file:///data, marking as invalid mount.";
                            isValidMount = false;
                        } else {
                            if (volumeInfo.mountInfo.strId.empty()) {
                                volumeInfo.mountInfo.strId = volumeInfo.mountInfo.strUri;
                            }
                        }
                        g_free(mountUri);
                    }
                    char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                    if (tooltip) {
                        volumeInfo.mountInfo.strTooltip = tooltip;
                        g_free(tooltip);
                    }
                    g_object_unref(root);
                }
                p_this->getMountIconsInfo(mount, volumeInfo.mountInfo);
            }
            g_object_unref(mount);
        } else {
            if (volumeInfo.isCanMount) {
                if(p_this->ifsettings->get(IFAUTOLOAD).toBool()) {
                    qDebug() << "[volume_added_callback] Volume can be mounted and autoload is enabled, mounting now.";
                    g_volume_mount(volume,
                                G_MOUNT_MOUNT_NONE,
                                nullptr,
                                nullptr,
                                GAsyncReadyCallback(frobnitz_result_func_volume),
                                p_this);
                }
            }
        }
        if (isValidMount) {
            qDebug() << "[volume_added_callback] Adding volume info (no drive).";
            p_this->m_dataFlashDisk->addVolumeInfo(volumeInfo);
        }
    } else {
        qDebug() << "[volume_added_callback] Handling as volume with drive.";
        char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            FDDriveInfo driveInfo;
            FDVolumeInfo volumeInfo;
            driveInfo.strId = devPath;
            char *strName = g_drive_get_name(gdrive);
            if (strName) {
                driveInfo.strName = strName;
                g_free(strName);
            }
            p_this->getDriveIconsInfo(gdrive, driveInfo);
            driveInfo.isCanEject = g_drive_can_eject(gdrive);
            driveInfo.isCanStop = g_drive_can_stop(gdrive);
            driveInfo.isCanStart = g_drive_can_start(gdrive);
            driveInfo.isRemovable = g_drive_is_removable(gdrive);
            driveInfo.lluVolSum = uVolumeSize;

            if(!p_this->isSystemRootDev(driveInfo.strId.c_str()) && !p_this->ifSataDev(driveInfo.strId.c_str()) &&
                (driveInfo.isCanEject || driveInfo.isCanStop || driveInfo.isRemovable)) {
                if(g_str_has_prefix(devPath,"/dev/sr") || g_str_has_prefix(devPath,"/dev/bus") || g_str_has_prefix(devPath,"/dev/sd")
                    || g_str_has_prefix(devPath,"/dev/mmcblk")) {
                    char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                    if (volumeId) {
                        volumeInfo.strId = volumeId;
                        g_free(volumeId);
                        char *volumeName = g_volume_get_name(volume);
                        if (volumeName) {
                            volumeInfo.strName = volumeName;
                            g_free(volumeName);
                        }
                        char *strDevName = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                        if (strDevName) {
                            volumeInfo.strDevName = strDevName;
                            g_free(strDevName);
                        }
                        p_this->getVolumeIconsInfo(volume, volumeInfo);
                        volumeInfo.isCanMount = g_volume_can_mount(volume);
                        volumeInfo.isCanEject = g_volume_can_eject(volume);
                        volumeInfo.isShouldAutoMount = g_volume_should_automount(volume);
                        GMount* mount = g_volume_get_mount(volume); //get当前卷设备的挂载信息
                        if (mount) {                  //该卷设备已挂载
                            qDebug() << "[volume_added_callback] Volume is already mounted (with drive).";
                            volumeInfo.mountInfo.isCanEject = g_mount_can_eject(mount);
                            volumeInfo.mountInfo.isCanUnmount = g_mount_can_unmount(mount);
                            if (volumeInfo.mountInfo.isCanEject || volumeInfo.mountInfo.isCanUnmount) {
                                char *mountId = g_mount_get_uuid(mount);
                                if (mountId) {
                                    volumeInfo.mountInfo.strId = mountId;
                                    g_free(mountId);
                                }
                                char *mountName = g_mount_get_name(mount);
                                if (mountName) {
                                    volumeInfo.mountInfo.strName = mountName;
                                    g_free(mountName);
                                }
                                isNewMount = !(p_this->m_dataFlashDisk->isMountInfoExist(volumeInfo.mountInfo));
                                // get mount total size
                                GFile *fileRoot = g_mount_get_root(mount);
                                if (fileRoot) {
                                    GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                                    if (info) {
                                        volumeInfo.mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                                        g_object_unref(info);
                                    }
                                    g_object_unref(fileRoot);
                                }
                                p_this->getDataCDRomCapacity(volumeInfo.strId.c_str(), volumeInfo.mountInfo.lluTotalSize);
                                // get mount uri
                                GFile *root = g_mount_get_default_location(mount);
                                if (root) {
                                    volumeInfo.mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                                    char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                                    if (mountUri) {
                                        volumeInfo.mountInfo.strUri = mountUri;
                                        if (ifLocalUsrDev(QString::fromStdString(mountUri))){
                                           driveInfo.islocalUsr == true;
                                        }else{
                                           driveInfo.islocalUsr == false;
                                        }
                                        if (!g_str_has_prefix(mountUri,"file:///data")) {
                                            if (volumeInfo.mountInfo.strId.empty()) {
                                                volumeInfo.mountInfo.strId = volumeInfo.mountInfo.strUri;
                                            }
                                        }
                                        g_free(mountUri);
                                    }
                                    char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                                    if (tooltip) {
                                        volumeInfo.mountInfo.strTooltip =tooltip;
                                        g_free(tooltip);
                                    }
                                    g_object_unref(root);
                                }
                                p_this->getMountIconsInfo(mount, volumeInfo.mountInfo);
                            }
                            g_object_unref(mount);
                        } else {
                            if(p_this->ifsettings->get(IFAUTOLOAD).toBool()&&p_this->ifGpartedBusy()) {
                                if (!p_this->ifHasFsType(volumeInfo.strId.c_str())/*&& !g_str_has_prefix(volumeInfo.strId.c_str(),"/dev/sr")*/){
                                   qDebug() << "[volume_added_callback] No filesystem type found, skipping mount.";
                                   return;
                                }
                                qDebug() << "[volume_added_callback] Mounting volume (with drive) as autoload and gparted busy.";
                                g_volume_mount(volume,
                                            G_MOUNT_MOUNT_NONE,
                                            nullptr,
                                            nullptr,
                                            GAsyncReadyCallback(mount_result_func_volume_with_error),
                                            p_this);
                            }
                        }
                    }
                }
                if (isNewMount) {
                    qDebug() << "[volume_added_callback] New mount detected, will show notification.";
                    qInfo()<<"cd data disk has mounted!";
                    volumeInfo.isNewInsert = true;
                    p_this->m_dataFlashDisk->addVolumeInfoWithDrive(driveInfo, volumeInfo);
                    string strDevId = driveInfo.strId.empty()?volumeInfo.strId:driveInfo.strId;
                    if(volumeInfo.mountInfo.lluTotalSize >= 1){
                        Q_EMIT p_this->convertShowWindow(strDevId.c_str(), volumeInfo.mountInfo.strUri.c_str());
                    }
                } else {
                    qDebug() << "[volume_added_callback] Not a new mount, just updating info.";
                    p_this->m_dataFlashDisk->addVolumeInfoWithDrive(driveInfo, volumeInfo);
                }
            }
            g_free(devPath);
        }
        g_object_unref(gdrive);
    }
    if(p_this->m_dataFlashDisk->getValidInfoCount() > 0)
    {
        qDebug() << "[volume_added_callback] There is at least one valid device, showing systray icon.";
        p_this->m_systray->show();
    }
    p_this->m_dataFlashDisk->OutputInfos();
}

// when the U disk is pull out we should reduce all its partitions
void MainWindow::volume_removed_callback(GVolumeMonitor *monitor, GVolume *volume, MainWindow *p_this)
{
    qInfo() << "[volume_removed_callback] volume removed";
    FDVolumeInfo volumeInfo;
    char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if (volumeId) {
        volumeInfo.strId = volumeId;
        g_free(volumeId);
    }
    char *strName = g_volume_get_name(volume);
    if (strName) {
        volumeInfo.strName = strName;
        g_free(strName);
    }
    vector<string>::iterator itDeviceId = p_this->m_vtDeviveId.begin();
    for (; itDeviceId != p_this->m_vtDeviveId.end();) {
        if (volumeInfo.strId == *itDeviceId) {
            itDeviceId = p_this->m_vtDeviveId.erase(itDeviceId);
        } else {
            itDeviceId++;
        }
    }
    qDebug() << "[volume_removed_callback] Removing volume info from data manager.";
    p_this->m_dataFlashDisk->removeVolumeInfo(volumeInfo);
    if(p_this->m_dataFlashDisk->getValidInfoCount() == 0) {
        qDebug() << "[volume_removed_callback] No valid devices left, hiding systray.";
        p_this->m_systray->hide();
    }

    if(g_str_has_prefix(volumeInfo.strId.c_str(),"/dev/sr")){
        qDebug() << "[volume_removed_callback] CDROM device detected, handling CDROM removal.";
        bool bshowflag = false;
        if(!p_this->m_basewidget->isHidden()){
            bshowflag = true;
            p_this->m_basewidget->hide();
        }
        p_this->hideAllWidget(true);
        if(!p_this->m_clickToEject){ //手动弹出光驱显示弹出窗口
           qDebug() << "[volume_removed_callback] Manual eject not triggered, showing eject interface for CDROM.";
           p_this->m_eject = new ejectInterface(NULL);
           p_this->m_eject->showEjectInterface();
           if(bshowflag)
              QTimer::singleShot(1000, p_this, [=]() { Q_EMIT p_this->convertUpdateWindow(QString::fromStdString(volumeInfo.strName), 1); });
        }
    }

    if(p_this->m_clickToEject){
        qDebug() << "[volume_removed_callback] Manual eject triggered, showing eject interface.";
        p_this->m_eject = new ejectInterface(NULL);
        p_this->m_eject->showEjectInterface();
        //间隔一秒让弹出提示框显示出来
        QTimer::singleShot(1000, p_this, [=]() { Q_EMIT p_this->convertUpdateWindow(QString::fromStdString(volumeInfo.strName), 1); });//emit a signal to update the MainMainShow slot
    }
    p_this->m_clickToEject = false;
    p_this->m_dataFlashDisk->OutputInfos();
}

// when the volumes were mounted we add its mounts number
void MainWindow::mount_added_callback(GVolumeMonitor *monitor, GMount *mount, MainWindow *p_this)
{
    qInfo() << "[mount_added_callback] mount add";

    GDrive* gdrive = g_mount_get_drive(mount);
    GVolume* gvolume = g_mount_get_volume(mount);
    string strVolumePath = "";
    FDDriveInfo driveInfo;
    FDVolumeInfo volumeInfo;
    FDMountInfo mountInfo;
    bool isValidMount = true;

    if (gdrive) {
        char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            driveInfo.strId = devPath;
            qInfo() << "[mount_added_callback] drive id is" << driveInfo.strId.c_str();
            if (p_this->ifSataDev(driveInfo.strId.c_str())){
                qInfo() << "[mount_added_callback] Device is SATA, marking as invalid mount.";
                isValidMount = false;
            }
            char *strName = g_drive_get_name(gdrive);
            if (strName) {
                driveInfo.strName = strName;
                g_free(strName);
            }
            p_this->getDriveIconsInfo(gdrive, driveInfo);
            unsigned uVolumeSize = 0;
            GList* Volumes = g_drive_get_volumes(gdrive);
            if (Volumes) {
                uVolumeSize = g_list_length(Volumes);
                g_list_free(Volumes);
            }
            driveInfo.isCanEject = g_drive_can_eject(gdrive);
            driveInfo.isCanStop = g_drive_can_stop(gdrive);
            driveInfo.isCanStart = g_drive_can_start(gdrive);
            driveInfo.isRemovable = g_drive_is_removable(gdrive);
            driveInfo.lluVolSum = uVolumeSize;
            g_free(devPath);
        }
#if UDFAUTOMOUNT
        if (gMountCdrom && gMountCdrom->contains (QString(driveInfo.strId.c_str()))) {
            (*gMountCdrom)[QString(driveInfo.strId.c_str())] = true;
            qDebug() << "[mount_added_callback] UDFAUTOMOUNT: Marked CDROM as mounted.";
        }
#endif
        g_object_unref(gdrive);
    }
    if (gvolume) {
        char *volumeId = g_volume_get_identifier(gvolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        char *devVolumePath = g_volume_get_identifier(gvolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devVolumePath) {
            strVolumePath = devVolumePath;
            g_free(devVolumePath);
        }
        if (volumeId) {
            volumeInfo.strId = volumeId;
            g_free(volumeId);
            char *volumeName = g_volume_get_name(gvolume);
            if (volumeName) {
                volumeInfo.strName = volumeName;
                g_free(volumeName);
            }
            char *strDevName = g_volume_get_identifier(gvolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            if (strDevName) {
                volumeInfo.strDevName = strDevName;
                g_free(strDevName);
            }
            p_this->getVolumeIconsInfo(gvolume, volumeInfo);
            volumeInfo.isCanMount = g_volume_can_mount(gvolume);
            volumeInfo.isCanEject = g_volume_can_eject(gvolume);
            volumeInfo.isShouldAutoMount = g_volume_should_automount(gvolume);
        }
        g_object_unref(gvolume);
    }

    mountInfo.isCanEject = g_mount_can_eject(mount);
    char *mountId = g_mount_get_uuid(mount);
    if (mountId) {
        mountInfo.strId = mountId;
        g_free(mountId);
    }
    char *mountName = g_mount_get_name(mount);
    if (mountName) {
        mountInfo.strName = mountName;
        g_free(mountName);
    }
    mountInfo.isCanUnmount = g_mount_can_unmount(mount);
    GFile *root = g_mount_get_default_location(mount);
    // get mount total size
    GFile *fileRoot = g_mount_get_root(mount);
    if (fileRoot) {
        unsigned uRetryTime = 5;
        unsigned uDelayTime = 200000;
        while (uRetryTime > 0) {
            GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
            if (info) {
                mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                g_object_unref(info);
                break;
            } else {
                usleep(uDelayTime);
            }
            uRetryTime --;
        }
        g_object_unref(fileRoot);
    }
    p_this->getDataCDRomCapacity(driveInfo.strId.empty()?volumeInfo.strId.c_str():driveInfo.strId.c_str(),
            mountInfo.lluTotalSize);
    // get mount uri
    if (root) {
        mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
        char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
        if (mountUri) {
            qInfo() << "[mount_added_callback] mount uri is" << mountUri;
            mountInfo.strUri = mountUri;
            if (g_str_has_prefix(mountUri,"file:///data")) {
                qDebug() << "[mount_added_callback] Mount URI is file:///data, marking as invalid mount.";
                isValidMount = false;
            } else {
                if (mountInfo.strId.empty()) {
                    mountInfo.strId = mountInfo.strUri;
                }
            }

            g_free(mountUri);
        }
        char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
        if (tooltip) {
            mountInfo.strTooltip = tooltip;
            g_free(tooltip);
        }
        g_object_unref(root);
    }
    p_this->getMountIconsInfo(mount, mountInfo);

    if (driveInfo.strId.empty()) {
       qDebug() << "[mount_added_callback] No drive info, emitting telephoneMount signal.";
       Q_EMIT p_this->telephoneMount();
    }
    bool isNewMount = !(p_this->m_dataFlashDisk->isMountInfoExist(mountInfo));
    if (!driveInfo.strId.empty()) {
        if (p_this->isSystemRootDev(driveInfo.strId.c_str()) ||
            (!driveInfo.isCanEject && !driveInfo.isCanStop && !driveInfo.isRemovable)) {
            qDebug() << "[mount_added_callback] System root device or not removable/ejectable, marking as invalid mount.";
            isValidMount = false;
        }
    }
    if (volumeInfo.strId.empty()) {
        // No volume info, skip (e.g. ftp)
        qDebug() << "[mount_added_callback] No volume info, marking as invalid mount.";
        isValidMount = false;
    }

    if(isValidMount && (mountInfo.isCanUnmount || g_str_has_prefix(strVolumePath.c_str(),"/dev/bus")
            || g_str_has_prefix(strVolumePath.c_str(),"/dev/sr") || g_str_has_prefix(strVolumePath.c_str(),"/dev/mmcblk"))
            &&!g_str_has_prefix(strVolumePath.c_str(),"/dev/loop")) {
        qInfo() << "[mount_added_callback] real mount loaded";
        mountInfo.isNewInsert = true;
        if (!driveInfo.strId.empty()) {
            if (!volumeInfo.strId.empty()) {
                qDebug() << "[mount_added_callback] Adding volume info with drive.";
                volumeInfo.mountInfo = mountInfo;
                p_this->m_dataFlashDisk->addVolumeInfoWithDrive(driveInfo, volumeInfo);
            } else {
                qDebug() << "[mount_added_callback] Adding mount info (drive only).";
                p_this->m_dataFlashDisk->addMountInfo(mountInfo);
            }
        } else if (!volumeInfo.strId.empty()) {
            qDebug() << "[mount_added_callback] Adding volume info (no drive).";
            volumeInfo.mountInfo = mountInfo;
            p_this->m_dataFlashDisk->addVolumeInfo(volumeInfo);
        } else {
            qDebug() << "[mount_added_callback] Adding mount info (no drive, no volume).";
            p_this->m_dataFlashDisk->addMountInfo(mountInfo);
        }
    } else {
        qInfo() << "[mount_added_callback] Device does not meet filter conditions, skipping.";
    }

    if(p_this->m_dataFlashDisk->getValidInfoCount() >= 1) {
        if (isValidMount && isNewMount) {
            qInfo() << "[mount_added_callback] New mount detected, will show notification.";
            string strDevId = driveInfo.strId.empty()?volumeInfo.strId:driveInfo.strId;
            if(volumeInfo.mountInfo.lluTotalSize >= 1){
                Q_EMIT p_this->convertShowWindow(strDevId.c_str(), mountInfo.strUri.c_str());
            }
        }
        p_this->m_systray->show();
    }
    p_this->m_dataFlashDisk->OutputInfos();
}

// when the mountes were uninstalled we reduce mounts number
void MainWindow::mount_removed_callback(GVolumeMonitor *monitor, GMount *mount, MainWindow *p_this)
{
    qInfo() << "[mount_removed_callback] mount remove" << mount;
    FDMountInfo mountInfo;
    mountInfo.isCanEject = g_mount_can_eject(mount);
    char *mountId = g_mount_get_uuid(mount);
    if (mountId) {
        mountInfo.strId = mountId;
        g_free(mountId);
    }
    char *mountName = g_mount_get_name(mount);
    if (mountName) {
        mountInfo.strName = mountName;
        g_free(mountName);
    }

#if UDFAUTOMOUNT
    qDebug() << "[mount_removed_callback] UDFAUTOMOUNT: Processing CDROM unmount.";
    g_autoptr (GFile) mountP = g_mount_get_root (mount);
    if (G_IS_FILE(mountP)) {
        g_autofree char* path = g_file_get_path (mountP);
        if (gMountCdromMountPoint && gMountCdromMountPoint->contains (QString(path))) {
            QString devName = (*gMountCdromMountPoint)[path];
            if (gMountCdrom->contains (devName)) {
                (*gMountCdrom)[devName] = false;
                qDebug() << "[mount_removed_callback] UDFAUTOMOUNT: Marked CDROM as unmounted.";
            }
        }
    }
#endif
    mountInfo.isCanUnmount = g_mount_can_unmount(mount);
    GFile *root = g_mount_get_default_location(mount);
    if (root) {
        mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
        char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
        if (mountUri) {
            mountInfo.strUri = mountUri;
            g_free(mountUri);
            if (mountInfo.strId.empty()) {
                mountInfo.strId = mountInfo.strUri;
            }
        }
        char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
        if (tooltip) {
            mountInfo.strTooltip = tooltip;
            g_free(tooltip);
        }
        g_object_unref(root);
    }
    // check mount's volume had removed?
    FDVolumeInfo volumeInfo;
    p_this->m_dataFlashDisk->getVolumeInfoByMount(mountInfo, volumeInfo);

    std::string str = volumeInfo.strId; // 这里转换一下数据类型
    QString qstr = QString::fromStdString(str);
    QString filesystemType = "";
    if(!qstr.isEmpty()){
        filesystemType = p_this->getFileSystemType(qstr);
    }

//    QString command = "udevadm info -n " + qstr + " | grep ID_FS_TYPE| awk -F'=' '{print $2}' | tr -d '\n'";
    //这里是通过dev指定获取当前卷的文件系统类型，qstr后面的都是结果列表处理指令

//    QProcess process;
//    process.start("bash", QStringList() << "-c" << command);
//    process.waitForFinished();
//    QString result = process.readAllStandardOutput();
//    QString filesystemType = result;
    quint64 mountTickDiff = p_this->m_dataFlashDisk->getMountTickDiff(mountInfo);
    qDebug() << "[mount_removed_callback] Removing mount info from data manager.";
    p_this->m_dataFlashDisk->removeMountInfo(mountInfo);

    if(p_this->m_dataFlashDisk->getValidInfoCount() == 0) {
        qDebug() << "[mount_removed_callback] No valid devices left, hiding systray.";
        p_this->m_systray->hide();
    }
    p_this->m_dataFlashDisk->OutputInfos();
    if(!g_str_has_prefix (mountInfo.strId.c_str(), "burn:///") && qstr.isEmpty())
        Q_EMIT p_this->convertUpdateWindow(QString::fromStdString(mountInfo.strName), 2);     //emit a signal to update the MainMainShow slot
    qInfo() << "[mount_removed_callback] ID:" << mountInfo.strId.c_str() << ",volume is:" << volumeInfo.strId.c_str() << ",mount tickdiff:" << mountTickDiff;
    if (mountTickDiff > 0 && mountTickDiff < 500 && !volumeInfo.strId.empty()) {
        qDebug() << "[mount_removed_callback] Mount was recently added, scheduling remount.";
        QTimer::singleShot(1000, p_this, [&,volumeInfo,p_this]() { p_this->onRemountVolume(volumeInfo); });
    }
    //这里设置时延的目的就是等待卷信息完成同步，经测试，如此设置U盘重新挂载成功的弹出提示和文管的弹出提示时间间隔相差不到一秒
    if (filesystemType == "vfat" || filesystemType == "fat32" || filesystemType == "devtmpfs"){
        qDebug() << "[mount_removed_callback] FAT filesystem detected, sleeping 2000ms.";
        QThread::msleep(2000);
    }
    else if (filesystemType == "exfat" || filesystemType == "ntfs" || filesystemType == "fuseblk"){
        qDebug() << "[mount_removed_callback] NTFS/ExFAT filesystem detected, sleeping 1000ms.";
        QThread::msleep(1000);
    }
    else if (filesystemType == "ext4"){
        qDebug() << "[mount_removed_callback] EXT4 filesystem detected, sleeping 1000ms.";
        QThread::msleep(1000);
    }
}

// it stands that when you insert a usb device when all the  U disk partitions
void MainWindow::frobnitz_result_func_volume(GVolume *source_object,GAsyncResult *res,MainWindow *p_this)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    bool bMountSuccess = false;
    success = g_volume_mount_finish (source_object, res, &err);
    if(!err) {
        GMount* gmount = g_volume_get_mount(source_object);
        GDrive* gdrive = g_volume_get_drive(source_object);
        FDDriveInfo driveInfo;
        FDVolumeInfo volumeInfo;
        FDMountInfo mountInfo;
        if (gdrive) {
            char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
            if (devPath != NULL) {
                driveInfo.strId = devPath;
                char *strName = g_drive_get_name(gdrive);
                if (strName) {
                    driveInfo.strName = strName;
                    g_free(strName);
                }
                p_this->getDriveIconsInfo(gdrive, driveInfo);
                driveInfo.isCanEject = g_drive_can_eject(gdrive);
                driveInfo.isCanStop = g_drive_can_stop(gdrive);
                driveInfo.isCanStart = g_drive_can_start(gdrive);
                driveInfo.isRemovable = g_drive_is_removable(gdrive);
                g_free(devPath);
            }
            g_object_unref(gdrive);
        }
        char *volumeId = g_volume_get_identifier(source_object,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (volumeId) {
            volumeInfo.strId = volumeId;
            g_free(volumeId);
            char *volumeName = g_volume_get_name(source_object);
            if (volumeName) {
                volumeInfo.strName = volumeName;
                g_free(volumeName);
            }
            char *strDevName = g_volume_get_identifier(source_object,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            if (strDevName) {
                volumeInfo.strDevName = strDevName;
                g_free(strDevName);
            }
            p_this->getVolumeIconsInfo(source_object, volumeInfo);
            volumeInfo.isCanMount = g_volume_can_mount(source_object);
            volumeInfo.isCanEject = g_volume_can_eject(source_object);
            volumeInfo.isShouldAutoMount = g_volume_should_automount(source_object);
        }
        if (gmount) {
            bMountSuccess = true;
            mountInfo.isCanEject = g_mount_can_eject(gmount);
            char *mountId = g_mount_get_uuid(gmount);
            if (mountId) {
                mountInfo.strId = mountId;
                g_free(mountId);
            }
            char *mountName = g_mount_get_name(gmount);
            if (mountName) {
                mountInfo.strName = mountName;
                g_free(mountName);
            }
            mountInfo.isCanUnmount = g_mount_can_unmount(gmount);
            // get mount total size
            GFile *fileRoot = g_mount_get_root(gmount);
            if (fileRoot) {
                unsigned uRetryTime = 5;
                unsigned uDelayTime = 200000;
                while (uRetryTime > 0) {
                    GFileInfo *info = g_file_query_filesystem_info(fileRoot,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,nullptr,nullptr);
                    if (info) {
                        mountInfo.lluTotalSize = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
                        g_object_unref(info);
                        break;
                    } else {
                        usleep(uDelayTime);
                    }
                    uRetryTime --;
                }
                g_object_unref(fileRoot);
            }
            p_this->getDataCDRomCapacity(driveInfo.strId.empty()?volumeInfo.strId.c_str():driveInfo.strId.c_str(),
                mountInfo.lluTotalSize);
            // get mount uri
            GFile *root = g_mount_get_default_location(gmount);
            if (root) {
                mountInfo.isNativeDev = g_file_is_native(root);     //判断设备是本地设备or网络设备
                char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                if (mountUri) {
                    mountInfo.strUri = mountUri;
                    if (g_str_has_prefix(mountUri,"file:///data")) {
                        bMountSuccess = false;
                    }else {
                        if (mountInfo.strId.empty()) {
                            mountInfo.strId = mountInfo.strUri;
                        }
                    }
                    g_free(mountUri);
                }
                char *tooltip = g_file_get_parse_name(root);      //提示，即文件的解释
                if (tooltip) {
                    mountInfo.strTooltip = tooltip;
                    g_free(tooltip);
                }
                g_object_unref(root);
            }
            p_this->getMountIconsInfo(gmount, mountInfo);
            g_object_unref(gmount);
        }
        if (bMountSuccess) {
            mountInfo.isNewInsert = true;
            bool isNewMount = !(p_this->m_dataFlashDisk->isMountInfoExist(mountInfo));
            if (!driveInfo.strId.empty()) {
                if (!volumeInfo.strId.empty()) {
                    volumeInfo.mountInfo = mountInfo;
                    p_this->m_dataFlashDisk->addVolumeInfoWithDrive(driveInfo, volumeInfo);
                } else {
                    p_this->m_dataFlashDisk->addMountInfo(mountInfo);
                }
            } else if (!volumeInfo.strId.empty()) {
                volumeInfo.mountInfo = mountInfo;
                p_this->m_dataFlashDisk->addVolumeInfo(volumeInfo);
            } else {
                p_this->m_dataFlashDisk->addMountInfo(mountInfo);
            }
            if (isNewMount) {
                qInfo()<<"sig has emitted";
                string strDevId = driveInfo.strId.empty()?volumeInfo.strId:driveInfo.strId;
                if(volumeInfo.mountInfo.lluTotalSize >= 1){
                    Q_EMIT p_this->convertShowWindow(strDevId.c_str(), mountInfo.strUri.c_str());     //emit a signal to trigger the MainMainShow slot
                }
            }
        }
    } else if (G_IO_ERROR_ALREADY_MOUNTED == err->code) {
    } else if (G_IO_ERROR_UNKNOWN == err->code) {
    } else {
        qInfo()<<"sorry mount failed"<<err->code<<","<<err->message;
        GDrive* gdrive = g_volume_get_drive(source_object);
        Q_EMIT p_this->deviceError(gdrive);
        if (nullptr != gdrive) {
            g_object_unref(gdrive);
        }
    }
}

// here we begin painting the main interface
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    triggerType = 1;  //It represents how we open the interface


    if (!insertorclick) {
        hideAllWidget();
    }

    if(m_basewidget && !m_basewidget->isHidden() && !insertorclick){
       m_basewidget->hide();
       return ;
    }

    insertorclick = false;

    qDebug()<<"--------icon activated--MainWindowShow----------";
    MainWindowShow();
}

void MainWindow::hideEvent(QHideEvent event)
{
    // delete open_widget;
}

/*
 * newarea use all the information of the U disk to paint the main interface and add line
*/
void MainWindow::newarea(int No,
                         GDrive *Drive,
                         GVolume *Volume,
                         QString Drivename,
                         QString nameDis1,
                         QString nameDis2,
                         QString nameDis3,
                         QString nameDis4,
                         qlonglong capacityDis1,
                         qlonglong capacityDis2,
                         qlonglong capacityDis3,
                         qlonglong capacityDis4,
                         QString pathDis1,
                         QString pathDis2,
                         QString pathDis3,
                         QString pathDis4,
                         int linestatus)
{
    open_widget = new QClickWidget(ui->centralWidget,No,Drive,Volume,Drivename,nameDis1,nameDis2,nameDis3,nameDis4,
                                   capacityDis1,capacityDis2,capacityDis3,capacityDis4,
                                   pathDis1,pathDis2,pathDis3,pathDis4);
    connect(open_widget,&QClickWidget::clickedConvert,this,[=]()
    {
        ui->centralWidget->hide();
    });
    connect(open_widget,&QClickWidget::noDeviceSig,this,[=]()
    {
        m_systray->hide();
    });


    line = new QWidget;
    line->setFixedHeight(1);
    line->setObjectName("lineWidget");
    if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black" || currentThemeMode == "ukui-default"
        || currentThemeMode == "ukui")
    {
        line->setStyleSheet("background-color:rgba(255,255,255,0.2);");
    }
    else
    {
        line->setStyleSheet("background-color:rgba(0,0,0,0.2);");
    }

    //when the drive is only or the drive is the first one,we make linestatus become  1
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    line->setFixedSize(276,1);
    if (linestatus == 2)
    {
        this->vboxlayout->addWidget(line);
    }

    vboxlayout->setContentsMargins(2,4,2,4);
    vboxlayout->setSpacing(0);
    vboxlayout->setMargin(0);
    this->vboxlayout->addWidget(open_widget);

    if (linestatus == 0)
    {
        this->vboxlayout->addWidget(line);
    }
}

//showFrist：调用这个
void MainWindow::newarea(unsigned uDiskNo,
                 QString strDriveId,
                 QString strVolumeId,
                 QString strMountId,
                 QString driveName,
                 QString volumeName,
                 quint64 capacityDis,
                 QString strMountUri,
                 int uVolumeSum)
{
    qDebug()<<"---newarea-"<<"-strDriveId"<<strDriveId<<"-strVolumeId"<<strVolumeId<<"-strMountId"<<strMountId;

    m_fdClickWidget = new FDClickWidget(ui->centralWidget,uDiskNo,strDriveId,strVolumeId,strMountId,
                                                driveName,volumeName,capacityDis,strMountUri,uVolumeSum);
    connect(m_fdClickWidget, &FDClickWidget::clickedEject,this,&MainWindow::onClickedEject);

    setVolLabelLayout(uDiskNo, ui->centralWidget);

}

void MainWindow::showFristWindow(quint64 uVolumeSum,
                     int uVolumeNum,
                     QString strDriveId,
                     QString strVolumeId,
                     QString strMountId,
                     QString driveName,
                     QString volumeName,
                     quint64 capacityDis,
                     QString strMountUri)
{
    qDebug()<<"显示第一个设备，卷总数为"<<uVolumeSum;
    newarea(uVolumeNum, strDriveId,strVolumeId,strMountId,driveName,
            volumeName, capacityDis,strMountUri,uVolumeSum);
    hign = 112;
    if(uVolumeSum >=3 ){
        hign = hign+(uVolumeSum-2)*FLASHDISKITEM_TITLE_HEIGHT;
    }

    ui->centralWidget->setEjectOnCentra(hign);
    ui->centralWidget->setFixedSize(388, hign);
    ui->centralWidget->m_animLabel->setFixedSize(320, hign);
    ui->centralWidget->m_mainWindowHigh = hign;

    moveBottomNoBase();
    if(!m_basewidget->isHidden()){
        m_basewidget->hide();
    }

    if((uVolumeNum == uVolumeSum)&&(ui->centralWidget->isHidden()==true)){
        m_basewidget->mainWindowVboxLayout->addWidget(ui->centralWidget);
        m_basewidget->setFixedSize(388, hign);
        interfaceHideTime->start(3000);
        ui->centralWidget->initOpacityGSettings();
        ui->centralWidget->show();
        qInfo()<<"在basewindow中显示第一个设备"<<uVolumeSum<<ui->centralWidget->isHidden();
        m_basewidget->showBaseWindow();
        moveBaseWidgetGeo();      
    }

}

void MainWindow::showMainWindow(int uVolumeNum,
                                int uDiskCount,
                                quint64 uVolumeSum,
                                QVBoxLayout *vboxLayout1,
                                QString strDriveId,
                                QString strVolumeId,
                                QString strMountId,
                                QString driveName,
                                QString volumeName,
                                quint64 capacityDis,
                                QString strMountUri)
{
    qDebug() << "[showMainWindow] Processing volume" << uVolumeNum << "of device" << uDiskCount << ", total volumes:"
             << uVolumeSum << "| DriveId:" << strDriveId << ", VolumeId:" << strVolumeId << ", MountId:" << strMountId << "| Names:"
             << driveName << "/" << volumeName << ", Capacity:" << capacityDis;
    
    if(!centralWidgetList.contains(strDriveId)){
        qDebug() << "[showMainWindow] Creating new FDFrame for drive:" << strDriveId;
        FDFrame *fdframe = new FDFrame(this);// 创建新的设备框
        centralWidgetList.insert(strDriveId, fdframe);
    } else {
        qDebug() << "[showMainWindow] Using existing FDFrame for drive:" << strDriveId;
    }

    if (uDiskCount > 0) {
        hign = 112;
        if(uVolumeSum >=3 ){
            hign = hign+(uVolumeSum-2)*FLASHDISKITEM_TITLE_HEIGHT;
            qDebug() << "[showMainWindow] Multiple volumes detected, adjusted height to:" << hign;
            centralWidgetList[strDriveId]->setEjectOnCentra(hign);
        }
    } else {
        hign = 0;
        qDebug() << "[showMainWindow] No disk count, setting height to 0";
        ui->centralWidget->setFixedSize(0, 0);
    }
    
    qDebug() << "[showMainWindow] Setting frame size to 388x" << hign;
    centralWidgetList[strDriveId]->setFixedSize(388, hign);
    centralWidgetList[strDriveId]->m_animLabel->setFixedSize(320, hign);

    qDebug() << "[showMainWindow] Creating FDClickWidget for volume";
    m_fdClickWidget = new FDClickWidget(centralWidgetList[strDriveId],uVolumeNum, strDriveId,strVolumeId,strMountId,driveName,
                                      volumeName, capacityDis,strMountUri, uVolumeSum);
    connect(m_fdClickWidget, &FDClickWidget::clickedEject,this,&MainWindow::onClickedEject);

    setVolLabelLayout(uVolumeNum, centralWidgetList[strDriveId]);
    //centralWidgetList[strDriveId]->setLayout(vboxLayout1);
    if(uVolumeNum == uVolumeSum){
        qDebug() << "[showMainWindow] Last volume reached, setting main window height to" << hign;
        centralWidgetList[strDriveId]->m_mainWindowHigh = hign;
    }

    int widgethigh = 0;
    if ((insertorclick)&&(uVolumeSum == uVolumeNum))
    {
        qDebug() << "[showMainWindow] Insertorclick mode and last volume, adding widgets to base window";
        QMap<QString, FDFrame *>::iterator iter;
        int i = 0;
        for (iter = centralWidgetList.begin(); iter != centralWidgetList.end(); iter++) {
            if(i <= FLASHDISKITEM_SHOWDISK_MAX){
                 m_basewidget->mainWindowVboxLayout->addWidget(iter.value());
                 //计算总高度，用于调整弹窗尺寸
                 widgethigh += iter.value()->height()+(i*FLASHDISKITEM_SPACE_HEIGHT);
                 qDebug() << "[showMainWindow] Widget" << i << "height:" << iter.value()->height() << ", total height so far:" << widgethigh;
            } else {
                qDebug() << "[showMainWindow] Skipping widget" << i << "due to max limit";
            }
            i++;
        }
        // 最终，调整父容器尺寸并显示
        qDebug() << "[showMainWindow] Setting base widget size to 388x" << widgethigh;
        m_basewidget->setFixedSize(388, widgethigh);
        m_basewidget->showBaseWindow();
        interfaceHideTime->start(3000);
        moveBaseWidgetGeo();
    }

}

void MainWindow::moveBottomRight()
{
    //MARGIN 为到任务栏或屏幕边缘的间隔
    #define MARGIN 8
    QDBusInterface iface("org.ukui.panel",
                         "/panel/position",
                         "org.ukui.panel",
                         QDBusConnection::sessionBus());
    QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");
    //reply获取的参数共5个，分别是 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
    //    reply.value();
    qInfo()<<reply.value().at(4).toInt();
    QVariantList position_list=reply.value();
    QRect rect;

    switch(reply.value().at(4).toInt()){
    case 1:
        //任务栏位于上方
        rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                     position_list.at(1).toInt()+MARGIN,
                     this->width(),this->height());
        break;
        //任务栏位于左边
    case 2:
        rect = QRect(position_list.at(0).toInt()+MARGIN,
                     position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                     this->width(),this->height());
        break;
        //任务栏位于右边
    case 3:
        rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                     position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                     this->width(),this->height());
        break;
        //任务栏位于下方
    default:
        rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                     position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                     this->width(),this->height());
        break;
    }
    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
}

/*
    use dbus to get the location of the panel
*/
int MainWindow::getPanelPosition(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );
    QDBusReply<int> reply = interface.call("GetPanelPosition", str);

    return reply;
}

/*
    use the dbus to get the height of the panel
*/
int MainWindow::getPanelHeight(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );
    QDBusReply<int> reply = interface.call("GetPanelSize", str);
    return reply;
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
}

void MainWindow::moveBottomDirect(GDrive *drive)
{
}

bool MainWindow::ifNewInsertVol()
{
    map<string, FDDriveInfo>& listDriveInfo = m_dataFlashDisk->getDevInfoWithDrive();
    map<string, FDDriveInfo>::iterator itDriveInfo = listDriveInfo.begin();
    for ( ;itDriveInfo != listDriveInfo.end(); itDriveInfo++) {
        map<string, FDVolumeInfo>::iterator itVolumeInfo = itDriveInfo->second.listVolumes.begin();
        for ( ;itVolumeInfo != itDriveInfo->second.listVolumes.end(); itVolumeInfo++) {
            if ((itVolumeInfo->second.strId.empty() && itVolumeInfo->second.mountInfo.strId.empty())
                || (!itVolumeInfo->second.isNewInsert && !itVolumeInfo->second.mountInfo.isNewInsert)) {
                // is not new insert device
                qInfo()<<"---------循环便利退出-----------";
                return false;
                continue;
            }
        }
    }
    return true;
}

bool MainWindow::ifSpecicalDev(QString strDev)
{
    QString filename = CONFIG_FILE_PATH;
    QSettings settings(filename, QSettings::IniFormat);
    bool ret = false;
    QString devicename;
    settings.setIniCodec("UTF-8");
    settings.beginGroup("devfliter");
    devicename = settings.value("devicename").toString();
    QStringList list = devicename.split(",");
    if(list.contains(strDev)){
        ret = true;
    }

    return ret;
}

void MainWindow::hideAllWidget(bool ifHidden)
{
    // 清理 centralWidgetList内部所有FDFrame
    if(ifHidden == false){
        QMap<QString, FDFrame *>::iterator iter;
        for (iter = centralWidgetList.begin(); iter != centralWidgetList.end(); iter++) {
            if(!iter.value()->isHidden())
                iter.value()->hide();
                delete iter.value();
        }
    }else{
        QMap<QString, FDFrame *>::iterator iter;
        for (iter = centralWidgetList.begin(); iter != centralWidgetList.end(); iter++) {
                iter.value()->hide();
                delete iter.value();
        }
    }
    centralWidgetList.clear();
}

void MainWindow::setVolLabelLayout(int volnum, FDFrame* frame)
{
    if(volnum<=2){
        if(volnum == 1){
            this->m_iconClickWidget = m_fdClickWidget;
            m_fdClickWidget->m_fristClickWidget = m_fdClickWidget;
            if (layoutDirection() == Qt::LeftToRight)
                m_fdClickWidget->move(5,volnum);
            else
                m_fdClickWidget->move(frame->width() - 5 - m_fdClickWidget->width(),volnum);
        }else{
            if (layoutDirection() == Qt::LeftToRight)
                m_fdClickWidget->move(5,70);
            else
                m_fdClickWidget->move(frame->width() - 5 - m_fdClickWidget->width(),70);
        }
        m_fdClickWidget->m_fristClickWidget = this->m_iconClickWidget;
        m_fdClickWidget->setParent(frame);
        m_fdClickWidget->show();
    }else{
         m_fdClickWidget->m_fristClickWidget = this->m_iconClickWidget;
         m_fdClickWidget->setParent(frame);
         if (layoutDirection() == Qt::LeftToRight)
             m_fdClickWidget->move(5,40+volnum*15);
         else
             m_fdClickWidget->move(frame->width() - 5 - m_fdClickWidget->width(),40+volnum*15);
         m_fdClickWidget->show();
    }
}

void MainWindow::hideRepairDialog(GDrive *drive)
{
    if(drive == NULL){
        return;
    }
    g_autofree char* device = g_drive_get_identifier (drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);

    if(device != NULL){
        QString devicename = QString::fromStdString(device);
        qDebug() << "[hideRepairDialog] Hiding dialog for device:" << devicename;
        
        GList* volumes = g_drive_get_volumes(drive);
        if (!volumes && mRepairDialog.contains(devicename)) {
            RepairDialogBox* dialog = mRepairDialog.value(devicename);
            if (dialog) {
                mRepairDialog.remove(devicename);
                disconnect(dialog, nullptr, this, nullptr);
                dialog->reject();
            }
        } else {
            for (auto v = volumes; nullptr != v; v = v->next) {
                GVolume* vv = G_VOLUME(v->data);
                g_autofree char* volumeName = g_volume_get_identifier (vv, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
                g_autoptr(GMount) m = g_volume_get_mount(vv);
                if (!m) {
                    if (volumeName && mRepairDialog.contains(QString(volumeName))) {
                        RepairDialogBox* dialog = mRepairDialog.value(QString(volumeName));
                        if (dialog) {
                            mRepairDialog.remove(QString(volumeName));
                            disconnect(dialog, nullptr, this, nullptr);
                            dialog->reject();
                        }
                    }
                }
            }
            g_list_free_full(volumes, g_object_unref);
        }
    }
}

#if 0
//play notify sound when the device insert or remove
void MainWindow::palyNotifySound(QString strSoundName)
{
    //获取通知声效开关状态
    bool b_soundSwitchStatus = true;
    const QByteArray soundSwitch(CONTROL_PATH_SOUNDS_SCHEMA);
    if (QGSettings::isSchemaInstalled(soundSwitch)) {
        QGSettings *soundSwitchSetting = new QGSettings(soundSwitch);
        b_soundSwitchStatus = soundSwitchSetting->get(CONTROL_EVENT_SOUNDS_KEY).toBool();
        delete soundSwitchSetting;
        soundSwitchSetting = NULL;
    }

    //未打开消息音效,直接返回
    if (!b_soundSwitchStatus) {
        return;
    }
    ca_context_create(&m_pCaContext);

    gint retval;
    const gchar *desc = "alert sound";

    //播放通知声音
    const QByteArray text = strSoundName.toLocal8Bit();
    const gchar *id = text.data();
    const gchar *eventId = id;
    retval = ca_context_play(m_pCaContext, 0, CA_PROP_EVENT_ID, eventId, CA_PROP_EVENT_DESCRIPTION, desc, NULL);
    if (retval != 0) {
        qWarning()<<"声音播放错误";
    }
    qInfo() << desc << id << retval;

}
#endif


// 入口
void MainWindow::MainWindowShow(bool isUpdate)
{
    hideAllWidget(true); // 每次进来先强制清理一次 Map

    qInfo() << "[MainWindowShow] Starting main window display, isUpdate:" << isUpdate << ", insertorclick:" << insertorclick;
    this->getTransparentData();
    m_dataFlashDisk->OutputInfos();
    QString strTrans;
    strTrans =  QString::number(m_transparency, 10, 2);
    qDebug() << "[MainWindowShow] Transparency value:" << strTrans;
#if (QT_VERSION < QT_VERSION_CHECK(5,7,0))
    QString convertStyle = "#centralWidget{background:rgba(19,19,20,0.95);}";
#else
//    QString convertStyle = "#centralWidget{background:rgba(19,19,20," + strTrans + ");}";
#endif
    if(ui->centralWidget != NULL)
    {
        //cancel the connection between the timeout signal and main interface hiding
        if (insertorclick == false) {
            if (interfaceHideTime->isActive()) {
                qDebug() << "[MainWindowShow] Stopping interface hide timer (not insertorclick).";
                interfaceHideTime->stop();
            }
        } else if (!m_bIsMouseInTraIcon && !m_bIsMouseInCentral) {
            if (isUpdate) {
                if (m_basewidget->isHidden()) {
                    qDebug() << "[MainWindowShow] Base widget is hidden during update, returning.";
                    return ;
                } else {
                    if (m_dataFlashDisk->getValidInfoCount() < 1) {
                        m_basewidget->hide();
                        return ;
                    }
                }
            }
            interfaceHideTime->start(5000);
        }
    }
    num = 0;
    unsigned uDiskCount = 0;
    unsigned uVolumeCount = 0;

    // 这是清理ui->centralWidget内部的 FDClickWidget，
    // FDFrame没有清除，只清除了内部的 FDClickWidget，
    // 通过 hideAllWidget()来清除所需内容
    // 所有的设备（即使只有 1 个）都由 showMainWindow 创建并存入 centralWidgetList
    // 而 hideAllWidget()会清除centralWidgetList<strDriveId, FDFrame *>
    // QList<FDClickWidget *> listMainWindow = ui->centralWidget->findChildren<FDClickWidget *>();
    // for(FDClickWidget *listItem:listMainWindow)
    // {
    //     listItem->hide();
    //     listItem->deleteLater();
    // }

    // QList<QWidget *> listLine = ui->centralWidget->findChildren<QWidget *>();
    // for(QWidget *listItem:listLine)
    // {
    //     if(listItem->objectName() == "lineWidget") {
    //         listItem->hide();
    //         listItem->deleteLater();
    //     }
    // }


    // only show new insert device
    if (insertorclick) {
        qDebug() << "[MainWindowShow] Insertorclick is true, showing new insert devices.";
        //Convenient interface layout for all drives
        map<string, FDDriveInfo>& listDriveInfo = m_dataFlashDisk->getDevInfoWithDrive();
        if (!listDriveInfo.empty()) {
            if(!ui->centralWidget->isHidden()){
                ui->centralWidget->hide();
            }
            hideAllWidget();
            map<string, FDDriveInfo>::iterator itDriveInfo = listDriveInfo.begin();
            for ( ;itDriveInfo != listDriveInfo.end(); itDriveInfo++) {
                unsigned uVolumeNum = 0;
                unsigned uVolumeSum = 0;
                bool isCanShow = (itDriveInfo->second.isCanEject || itDriveInfo->second.isCanStop || itDriveInfo->second.isRemovable);
                QString strDriveName = QString::fromStdString(itDriveInfo->second.strName);
                if (itDriveInfo->second.listVolumes.size() > 0) {
                    uDiskCount++;
                    uVolumeSum = itDriveInfo->second.listVolumes.size();
                }
                QVBoxLayout *vboxLayout1 = new QVBoxLayout();
                qDebug() << "[MainWindowShow] Iterating through devices - DriveName:" << strDriveName << ", DiskCount:" << uDiskCount << ", Total volumes:" << uVolumeSum;
                map<string, FDVolumeInfo>::iterator itVolumeInfo = itDriveInfo->second.listVolumes.begin();
                for ( ;itVolumeInfo != itDriveInfo->second.listVolumes.end(); itVolumeInfo++) {
                    QString strApiName;
                    quint64 lluTotalSize = itVolumeInfo->second.mountInfo.lluTotalSize;
                    QString strMountUri = QString::fromStdString(itVolumeInfo->second.mountInfo.strUri);
                    QString strDriveId = QString::fromStdString(itDriveInfo->second.strId);
                    QString strVolumeId = QString::fromStdString(itVolumeInfo->second.strId);
                    QString strMountId = QString::fromStdString(itVolumeInfo->second.mountInfo.strId);
                    unsigned uVolumeType = 0;  // 0:normal file volume, 1: cddata, 2:tele dev

                    qDebug() << "[MainWindowShow] Iterating through volumes - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                    if ((itVolumeInfo->second.strId.empty() && itVolumeInfo->second.mountInfo.strId.empty())
                        || (!itVolumeInfo->second.isNewInsert && !itVolumeInfo->second.mountInfo.isNewInsert)) {
                        // is not new insert device
                        continue;
                    }
                    if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                        string strMountUri = itVolumeInfo->second.mountInfo.strUri;
                        if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                            uVolumeType = 0;
                        } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                            uVolumeType = 2;
                        } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                            uVolumeType = 1;
                        }
                    }
                    if (uVolumeType == 1 || uVolumeType == 2 || isCanShow) {  // cd module or drive can show
                        if (!itVolumeInfo->second.strId.empty()) {
                            uVolumeNum++;
                            uVolumeCount++;
                            strApiName = QString::fromStdString(itVolumeInfo->second.strName);
                        } else if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                            uVolumeNum++;
                            uVolumeCount++;
                            strApiName = QString::fromStdString(itVolumeInfo->second.mountInfo.strName);
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                    if (uDiskCount > 0) {
                        if (uVolumeType == 1) {   // deal with cd info
                            qDebug() << "[MainWindowShow] Processing CDROM volume - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount, uVolumeSum,vboxLayout1, strDriveId, strVolumeId, strMountId, strDriveName,
                                           strApiName, 1, "burn:///");
                        } else if (uVolumeType == 2) {
                            QString telephoneName = tr("telephone device");
                            QByteArray strTelePhone = telephoneName.toLocal8Bit();
                            char *realTele = strTelePhone.data();
                            qDebug() << "[MainWindowShow] Processing telephone device - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount, uVolumeSum,vboxLayout1, strDriveId, strVolumeId, strMountId, realTele,
                                         strApiName, lluTotalSize, strMountUri);
                        } else {
                            qDebug() << "[MainWindowShow] Processing USB device - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount, uVolumeSum,vboxLayout1, strDriveId, strVolumeId, strMountId, strDriveName,
                                           strApiName, lluTotalSize, strMountUri);
                        }
                    }
                }
                if (itDriveInfo->second.listVolumes.size() > 0 && uVolumeNum == 0 && uDiskCount > 0) {
                    uDiskCount --;
                }
            }
        }
        // show volume info without drive
        map<string, FDVolumeInfo>& listVolumeInfo = m_dataFlashDisk->getDevInfoWithVolume();
        if (!listVolumeInfo.empty()) {
            qDebug() << "[MainWindowShow] Showing volume info without drive.";
            map<string, FDVolumeInfo>::iterator itVolumeInfo = listVolumeInfo.begin();
            QVBoxLayout *vboxLayout1 = new QVBoxLayout();
            for (; itVolumeInfo != listVolumeInfo.end(); itVolumeInfo++) {
                QString strApiName;
                QString strMainName;
                bool isCanShow = itVolumeInfo->second.isCanEject;
                quint64 lluTotalSize = itVolumeInfo->second.mountInfo.lluTotalSize;
                QString strMountUri = QString::fromStdString(itVolumeInfo->second.mountInfo.strUri);
                QString strVolumeId = QString::fromStdString(itVolumeInfo->second.strId);
                QString strMountId = QString::fromStdString(itVolumeInfo->second.mountInfo.strId);
                unsigned uVolumeType = 0;  // 0:normal file volume, 1: cddata, 2:tele dev
                unsigned uVolumeSum = listVolumeInfo.size();
                if ((itVolumeInfo->second.strId.empty() && itVolumeInfo->second.mountInfo.strId.empty())
                    || (!itVolumeInfo->second.isNewInsert && !itVolumeInfo->second.mountInfo.isNewInsert)) {
                    // is not new insert device
                    continue;
                }
                if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                    string strMountUri = itVolumeInfo->second.mountInfo.strUri;
                    if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                        uVolumeType = 0;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                        uVolumeType = 2;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                        uVolumeType = 1;
                    }
                }
                if (uVolumeType == 1 || uVolumeType == 2 || isCanShow) {  // cd module or drive can show
                    if (!itVolumeInfo->second.strId.empty()) {
                        uDiskCount++;
                        uVolumeCount++;
                        strMainName = strApiName = QString::fromStdString(itVolumeInfo->second.strName);
                    } else if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                        uDiskCount++;
                        uVolumeCount++;
                        strMainName = strApiName = QString::fromStdString(itVolumeInfo->second.mountInfo.strName);
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
                if (uDiskCount > 0) {
                    if (uVolumeType == 1) {   // deal with cd info
                        // if(uDiskCount == 1) {
                        //     showFristWindow(uVolumeSum, 1, "",strVolumeId,strMountId,strMainName,
                        //                     strApiName, 1, "burn:///");
                        // } else {
                        //     showMainWindow(1, uDiskCount, uVolumeSum,vboxLayout1, "", strVolumeId, strMountId, strMainName,
                        //                    strApiName, 1, "burn:///");
                        // }
                        // 不区分是否是首个设备，全部走统一路径创建FDFrame
                        showMainWindow(1, uDiskCount, uVolumeSum, vboxLayout1, "", strVolumeId, strMountId, strMainName,
                                            strApiName, 1, "burn:///");
                    } else if (uVolumeType == 2) {
                        QString telephoneName = tr("telephone device");
                        QByteArray strTelePhone = telephoneName.toLocal8Bit();
                        char *realTele = strTelePhone.data();
                        // if(uDiskCount == 1) {
                        //     showFristWindow(uVolumeSum,1, "",strVolumeId,strMountId,realTele,
                        //                     strApiName, lluTotalSize, strMountUri);
                        // } else {
                        //     showMainWindow(1, uDiskCount, uVolumeSum,vboxLayout1, "", strVolumeId, strMountId, realTele,
                        //                    strApiName, lluTotalSize, strMountUri);
                        // }
                        showMainWindow(1, uDiskCount, uVolumeSum,vboxLayout1, "", strVolumeId, strMountId, realTele,
                                           strApiName, lluTotalSize, strMountUri);
                    } else {
                        // if(uDiskCount == 1) {
                        //     showFristWindow(uVolumeSum, 1, "", strVolumeId, strMountId, strMainName,
                        //                     strApiName, lluTotalSize, strMountUri);
                        // } else {
                        //     showMainWindow(1, uDiskCount, uVolumeSum,vboxLayout1, "", strVolumeId, strMountId, strMainName,
                        //                   strApiName, lluTotalSize, strMountUri);
                        // }
                        showMainWindow(1, uDiskCount, uVolumeSum,vboxLayout1, "", strVolumeId, strMountId, strMainName,
                                          strApiName, lluTotalSize, strMountUri);
                    }
                }
            }
        }
        // show mount info without drive & volume
        map<string, FDMountInfo>& listMountInfo = m_dataFlashDisk->getDevInfoWithMount();
        if (!listMountInfo.empty()) {
            qDebug() << "[MainWindowShow] Showing mount info without drive & volume.";
            map<string, FDMountInfo>::iterator itMountInfo = listMountInfo.begin();
            for (; itMountInfo != listMountInfo.end(); itMountInfo++) {
                QString strApiName;
                QString strMainName;
                bool isCanShow = (itMountInfo->second.isCanUnmount || itMountInfo->second.isCanEject);
                quint64 lluTotalSize = itMountInfo->second.lluTotalSize;
                QString strMountUri = QString::fromStdString(itMountInfo->second.strUri);
                QString strMountId = QString::fromStdString(itMountInfo->second.strId);
                unsigned uMountType = 0;  // 0:normal file, 1: cddata, 2:tele dev
                if (itMountInfo->second.strId.empty() || !itMountInfo->second.isNewInsert) {
                    // is not new insert device
                    continue;
                }
                if (!itMountInfo->second.strId.empty()) {
                    string strMountUri = itMountInfo->second.strUri;
                    if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                        uMountType = 0;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                        uMountType = 2;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                        uMountType = 1;
                    }
                } else {
                    continue;
                }
                if (uMountType == 1 || uMountType == 2 || isCanShow) {  // cd module or drive can show
                    uDiskCount++;
                    uVolumeCount++;
                    strMainName = strApiName = QString::fromStdString(itMountInfo->second.strName);
                } else {
                    continue;
                }
                if (uDiskCount > 0) {
                    if (uMountType == 1) {   // deal with cd info
                        if(uDiskCount == 1) {
                            newarea(1, "","",strMountId,strMainName,
                                    strApiName,1, "burn:///", 1);
                        } else {
                            newarea(1, "","",strMountId,strMainName,
                                    strApiName,1, "burn:///", 2);
                        }
                    } else if (uMountType == 2) {
                        QString telephoneName = tr("telephone device");
                        QByteArray strTelePhone = telephoneName.toLocal8Bit();
                        char *realTele = strTelePhone.data();
                        if(uDiskCount == 1) {
                            newarea(1, "","",strMountId,realTele,
                                    strApiName,lluTotalSize,strMountUri,1);
                        } else {
                            newarea(1, "","",strMountId,realTele,
                                    strApiName,lluTotalSize,strMountUri,2);
                        }
                    } else {
                        if(uDiskCount == 1) {
                            newarea(1, "","",strMountId,strMainName,
                                    strApiName,lluTotalSize,strMountUri,1);
                        } else {
                            newarea(1, "","",strMountId,strMainName,
                                    strApiName,lluTotalSize,strMountUri,2);
                        }
                    }
                }
            }
        }
    } else {  // show all device
        qDebug() << "[MainWindowShow] Showing all devices.";
        //Convenient interface layout for all drives
        if(!m_basewidget->isHidden()){
            m_basewidget->hide();
        }
        map<string, FDDriveInfo>& listDriveInfo = m_dataFlashDisk->getDevInfoWithDrive();
        if (!listDriveInfo.empty()) {
            map<string, FDDriveInfo>::iterator itDriveInfo = listDriveInfo.begin();
            for ( ;itDriveInfo != listDriveInfo.end(); itDriveInfo++) {
                unsigned uVolumeNum = 0;
                unsigned uVolumeSum = 0;
                bool isCanShow = (itDriveInfo->second.isCanEject || itDriveInfo->second.isCanStop || itDriveInfo->second.isRemovable);
                QString strDriveName = QString::fromStdString(itDriveInfo->second.strName);
                if (itDriveInfo->second.listVolumes.size() > 0) {
                    uDiskCount++;
                    uVolumeSum = itDriveInfo->second.listVolumes.size();
                    qDebug() << "[MainWindowShow] Showing device - DiskCount:" << uDiskCount << ", VolumeSum:" << uVolumeSum;
                }
                QVBoxLayout *vboxLayout2 = new QVBoxLayout();
                map<string, FDVolumeInfo>::iterator itVolumeInfo = itDriveInfo->second.listVolumes.begin();
                for ( ;itVolumeInfo != itDriveInfo->second.listVolumes.end(); itVolumeInfo++) {
                    QString strApiName;
                    quint64 lluTotalSize = itVolumeInfo->second.mountInfo.lluTotalSize;
                    QString strMountUri = QString::fromStdString(itVolumeInfo->second.mountInfo.strUri);
                    QString strDriveId = QString::fromStdString(itDriveInfo->second.strId);
                    QString strVolumeId = QString::fromStdString(itVolumeInfo->second.strId);
                    QString strMountId = QString::fromStdString(itVolumeInfo->second.mountInfo.strId);
                    unsigned uVolumeType = 0;  // 0:normal file volume, 1: cddata, 2:tele dev
                    if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                        string strMountUri = itVolumeInfo->second.mountInfo.strUri;
                        qInfo()<<"uri is "<<QString::fromStdString(strMountUri);
                        if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                            uVolumeType = 0;
                        } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                            uVolumeType = 2;
                        } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                            uVolumeType = 1;
                        }
                    }else{
                        qInfo()<<"no mount "<<strDriveId<<strVolumeId<<strMountId;
                        continue;
                    }
                    if (uVolumeType == 1 || uVolumeType == 2 || isCanShow) {  // cd module or drive can show
                        if (!itVolumeInfo->second.strId.empty()) {
                            uVolumeNum++;
                            uVolumeCount++;
                            strApiName = QString::fromStdString(itVolumeInfo->second.strName);
                        } else if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                            uVolumeNum++;
                            uVolumeCount++;
                            strApiName = QString::fromStdString(itVolumeInfo->second.mountInfo.strName);
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                    //如果设备数大于0
                    if (uDiskCount > 0) {
                        if (uVolumeType == 1) {   // deal with cd info
                            qDebug() << "[MainWindowShow] Processing CDROM volume - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount, uVolumeSum,vboxLayout2, strDriveId, strVolumeId, strMountId, strDriveName,
                                           strApiName, 1, "burn:///");
                        } else if (uVolumeType == 2) {
                            QString telephoneName = tr("telephone device");
                            QByteArray strTelePhone = telephoneName.toLocal8Bit();
                            char *realTele = strTelePhone.data();
                            qDebug() << "[MainWindowShow] Processing telephone device - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount, uVolumeSum,vboxLayout2, strDriveId, strVolumeId, strMountId, realTele,
                                             strApiName, lluTotalSize, strMountUri);
                        } else {
                            if(isUpdate && itDriveInfo->second.lluVolSum != uVolumeSum){
                                qDebug() << "[MainWindowShow] DriveId:" << QString::fromStdString(itDriveInfo->second.strId) << "DriveVolumeSum:" << itDriveInfo->second.lluVolSum << "volume sum is " << uVolumeSum;
                                continue;
                            }
                            qDebug() << "[MainWindowShow] Processing USB device - DriveId:" << strDriveId << ", VolumeId:" << strVolumeId;
                            showMainWindow(uVolumeNum, uDiskCount,uVolumeSum ,vboxLayout2, strDriveId, strVolumeId, strMountId, strDriveName,
                                          strApiName, lluTotalSize, strMountUri);
                        }
                    }
                }
            }

        }
        // show volume info without drive
        map<string, FDVolumeInfo>& listVolumeInfo = m_dataFlashDisk->getDevInfoWithVolume();
        if (!listVolumeInfo.empty()) {
            map<string, FDVolumeInfo>::iterator itVolumeInfo = listVolumeInfo.begin();
            QVBoxLayout *vboxLayout2 = new QVBoxLayout();
            for (; itVolumeInfo != listVolumeInfo.end(); itVolumeInfo++) {
                QString strApiName;
                QString strMainName;
                bool isCanShow = itVolumeInfo->second.isCanEject;
                quint64 lluTotalSize = itVolumeInfo->second.mountInfo.lluTotalSize;
                QString strMountUri = QString::fromStdString(itVolumeInfo->second.mountInfo.strUri);
                QString strVolumeId = QString::fromStdString(itVolumeInfo->second.strId);
                QString strMountId = QString::fromStdString(itVolumeInfo->second.mountInfo.strId);
                unsigned uVolumeType = 0;  // 0:normal file volume, 1: cddata, 2:tele dev
                if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                    string strMountUri = itVolumeInfo->second.mountInfo.strUri;
                    if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                        uVolumeType = 0;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                        uVolumeType = 2;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                        uVolumeType = 1;
                    }
                }
                if (uVolumeType == 1 || uVolumeType == 2 || isCanShow) {  // cd module or drive can show
                    if (!itVolumeInfo->second.strId.empty()) {
                        uDiskCount++;
                        uVolumeCount++;
                        strMainName = strApiName = QString::fromStdString(itVolumeInfo->second.strName);
                    } else if (!itVolumeInfo->second.mountInfo.strId.empty()) {
                        uDiskCount++;
                        uVolumeCount++;
                        strMainName = strApiName = QString::fromStdString(itVolumeInfo->second.mountInfo.strName);
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
                if (uDiskCount > 0) {
                    if (uVolumeType == 1) {   // deal with cd info
                        showMainWindow(1, uDiskCount, 1, vboxLayout2, "", strVolumeId, strMountId, strMainName,
                                       strApiName, 1, "burn:///");
                    } else if (uVolumeType == 2) {
                        QString telephoneName = tr("telephone device");
                        QByteArray strTelePhone = telephoneName.toLocal8Bit();
                        char *realTele = strTelePhone.data();
                        showMainWindow(1, uDiskCount, 1,vboxLayout2, "", strVolumeId, strMountId, realTele,
                                       strApiName, lluTotalSize, strMountUri);
                    } else {
                        showMainWindow(1, uDiskCount, 1,vboxLayout2, "", strVolumeId, strMountId, strMainName,
                                      strApiName, lluTotalSize, strMountUri);
                    }
                }
            }
        }
        // show mount info without drive & volume
        map<string, FDMountInfo>& listMountInfo = m_dataFlashDisk->getDevInfoWithMount();
        if (!listMountInfo.empty()) {
            QVBoxLayout *vboxLayout2 = new QVBoxLayout();
            map<string, FDMountInfo>::iterator itMountInfo = listMountInfo.begin();
            for (; itMountInfo != listMountInfo.end(); itMountInfo++) {
                QString strApiName;
                QString strMainName;
                bool isCanShow = (itMountInfo->second.isCanUnmount || itMountInfo->second.isCanEject);
                quint64 lluTotalSize = itMountInfo->second.lluTotalSize;
                QString strMountUri = QString::fromStdString(itMountInfo->second.strUri);
                QString strMountId = QString::fromStdString(itMountInfo->second.strId);
                unsigned uMountType = 0;  // 0:normal file, 1: cddata, 2:tele dev
                if (!itMountInfo->second.strId.empty()) {
                    string strMountUri = itMountInfo->second.strUri;
                    if(g_str_has_prefix(strMountUri.c_str(),"file:///")) {
                        uMountType = 0;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"mtp://") || g_str_has_prefix(strMountUri.c_str(),"gphoto2://")){
                        uMountType = 2;
                    } else if (g_str_has_prefix(strMountUri.c_str(),"burn:///")/* || g_str_has_prefix(strMountUri.c_str(),"cdda://")*/) {
                        uMountType = 1;
                    }
                } else {
                    continue;
                }
                if (uMountType == 1 || uMountType == 2 || isCanShow) {  // cd module or drive can show
                    uDiskCount++;
                    uVolumeCount++;
                    strMainName = strApiName = QString::fromStdString(itMountInfo->second.strName);
                } else {
                    continue;
                }
                if (uDiskCount > 0) {
                    if (uMountType == 1) {   // deal with cd info
                        showMainWindow(1, uDiskCount, 1, vboxLayout2, "", "", strMountId, strMainName,
                                       strApiName, 1, "burn:///");
                    } else if (uMountType == 2) {
                        QString telephoneName = tr("telephone device");
                        QByteArray strTelePhone = telephoneName.toLocal8Bit();
                        char *realTele = strTelePhone.data();
                        showMainWindow(1, uDiskCount, 1,vboxLayout2, "", "", strMountId, realTele,
                                       strApiName, lluTotalSize, strMountUri);

                    } else {
                        showMainWindow(1, uDiskCount, 1,vboxLayout2, "", "", strMountId, strMainName,
                                      strApiName, lluTotalSize, strMountUri);
                    }
                }
            }
        }
        if (!insertorclick){
            QMap<QString, FDFrame *>::iterator iter;
            int i = 0;
            for (iter = centralWidgetList.begin(); iter != centralWidgetList.end(); iter++) {
                if(i <= FLASHDISKITEM_SHOWDISK_MAX){
                    m_basewidget->mainWindowVboxLayout->addWidget(iter.value());
                }
                i++;
            }
            m_basewidget->setFixedWidth(388);   //height自动适配高度
            m_basewidget->showBaseWindow();
            moveBaseWidgetGeo();
        }
    }

}

void MainWindow::ifgetPinitMount()
{
    int pointMountNum = 0;
    QFile file(tmpPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString content = file.readLine().trimmed();
        while (!file.atEnd())
        {
            if (content.contains(".mount")) {
                pointMountNum += 1;
            }

            content = file.readLine().trimmed();
            if(pointMountNum >= 1) {
                findPointMount = true;
            } else {
                findPointMount = false;
            }
        }
    }
    file.close();
}

void MainWindow::onMaininterfacehide()
{
    m_basewidget->hide();
    ui->centralWidget->hide();
    hideAllWidget();
    this->driveVolumeNum = 0;
    interfaceHideTime->stop();
    m_dataFlashDisk->resetAllNewState();
}

void MainWindow::moveBottomNoBase()
{
#if 0
    int position=0;
    int panelSize=0;
    if(QGSettings::isSchemaInstalled(QString("org.ukui.panel.settings").toLocal8Bit()))
    {
        QGSettings* gsetting=new QGSettings(QString("org.ukui.panel.settings").toLocal8Bit());
        if(gsetting->keys().contains(QString("panelposition")))
            position=gsetting->get("panelposition").toInt();
        else
            position=0;
        if(gsetting->keys().contains(QString("panelsize")))
            panelSize=gsetting->get("panelsize").toInt();
        else
            panelSize=SmallPanelSize;
    }
    else
    {
        position=0;
        panelSize=SmallPanelSize;
    }

    int x=QApplication::primaryScreen()->geometry().x();
    int y=QApplication::primaryScreen()->geometry().y();

    if(position==0)
        ui->centralWidget->setGeometry(QRect(x + QApplication::primaryScreen()->geometry().width()-ui->centralWidget->width() - DISTANCEMEND - DISTANCEPADDING,y+ QApplication::primaryScreen()->geometry().height()-panelSize-ui->centralWidget->height() - DISTANCEPADDING,ui->centralWidget->width(),ui->centralWidget->height()));
    else if(position==1)
        ui->centralWidget->setGeometry(QRect(x + QApplication::primaryScreen()->geometry().width()-ui->centralWidget->width() - DISTANCEMEND - DISTANCEPADDING,y+ panelSize + DISTANCEPADDING,ui->centralWidget->width(),ui->centralWidget->height()));  // Style::minw,Style::minh the width and the height of the interface  which you want to show
    else if(position==2)
        ui->centralWidget->setGeometry(QRect(x+panelSize + DISTANCEPADDING,y + QApplication::primaryScreen()->geometry().height() - ui->centralWidget->height() - DISTANCEPADDING,ui->centralWidget->width(),ui->centralWidget->height()));
    else
        ui->centralWidget->setGeometry(QRect(x+QApplication::primaryScreen()->geometry().width()-panelSize-ui->centralWidget->width() - DISTANCEPADDING,y + QApplication::primaryScreen()->geometry().height() - ui->centralWidget->height() - DISTANCEPADDING,ui->centralWidget->width(),ui->centralWidget->height()));
    ui->centralWidget->repaint();

#endif
    // MARGIN 为到任务栏或屏幕边缘的间隔
#define MARGIN 8
    QDBusInterface iface("org.ukui.panel",
                         "/panel/position",
                         "org.ukui.panel",
                         QDBusConnection::sessionBus());
    QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");

    if (!iface.isValid() || !iface.isValid() || reply.value().size()<5) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        ui->centralWidget->setGeometry(0,0,ui->centralWidget->width(),ui->centralWidget->height());
    } else {
        // reply获取的参数共5个，分别是 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
        QVariantList position_list=reply.value();
        QRect rect;
        qInfo()<<"get panel geo"<<position_list;
        switch(reply.value().at(4).toInt()){
        case 1:
            // 任务栏位于上方
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-ui->centralWidget->width()-MARGIN,
                         position_list.at(1).toInt()+MARGIN,
                         ui->centralWidget->width(),ui->centralWidget->height());
            break;
            // 任务栏位于左边
        case 2:
            rect = QRect(position_list.at(0).toInt()+MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-ui->centralWidget->height()-MARGIN,
                         ui->centralWidget->width(),this->height());
            break;
            // 任务栏位于右边
        case 3:
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-ui->centralWidget->width()-MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-ui->centralWidget->height()-MARGIN,
                         ui->centralWidget->width(),ui->centralWidget->height());
            break;
            // 任务栏位于下方
        default:
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-ui->centralWidget->width()-MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-ui->centralWidget->height()-MARGIN,
                         ui->centralWidget->width(),ui->centralWidget->height());
            break;
        }
        qInfo()<<"frist window geo"<<rect;
        kdk::WindowManager::setGeometry(ui->centralWidget->windowHandle(), rect);
    }
}

void MainWindow::moveBaseWidgetGeo()
{
// MARGIN 为到任务栏或屏幕边缘的间隔
#define MARGIN 8
#if 0
    QDBusInterface iface("org.ukui.panel",
                         "/panel/position",
                         "org.ukui.panel",
                         QDBusConnection::sessionBus());
    QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");

    if (!iface.isValid() || !iface.isValid() || reply.value().size()<5) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        m_basewidget->setGeometry(0,0,m_basewidget->width(),m_basewidget->height());
    } else {
        // reply获取的参数共5个，分别是 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
        QVariantList position_list=reply.value();
        QRect rect;
        qInfo()<<"get position:"<<position_list;
        switch(reply.value().at(4).toInt()){
        case PanelPosition::Top:
            // 任务栏位于上方
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-m_basewidget->width()-MARGIN,
                         position_list.at(1).toInt()+MARGIN,
                         m_basewidget->width(),m_basewidget->height());
            break;
            // 任务栏位于左边
        case PanelPosition::Left:
            rect = QRect(position_list.at(0).toInt()+MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-m_basewidget->height()-MARGIN,
                         m_basewidget->width(),this->height());
            break;
            // 任务栏位于右边
        case PanelPosition::Right:
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-m_basewidget->width()-MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-m_basewidget->height()-MARGIN,
                         m_basewidget->width(),m_basewidget->height());
            break;
            // 任务栏位于下方
        default:
            rect = QRect(position_list.at(0).toInt()+position_list.at(2).toInt()-m_basewidget->width()-MARGIN,
                         position_list.at(1).toInt()+reply.value().at(3).toInt()-m_basewidget->height()-MARGIN,
                         m_basewidget->width(),m_basewidget->height());
            break;
        }
        qInfo()<<"windows position:"<<rect;
        kdk::WindowManager::setGeometry(m_basewidget->windowHandle(), rect);
    }
#endif
    auto tmpScreen = getCurrentScreen();
    if (!tmpScreen) {
        tmpScreen = QGuiApplication::primaryScreen();
    }
    QRect availableGeo = tmpScreen->geometry();
    int x, y;

    setMaxHeightLimit();

    switch (m_panelPosition)
    {
    case PanelPosition::Top:
        if (layoutDirection() == Qt::LeftToRight) {
            x = availableGeo.x() + availableGeo.width() - m_basewidget->width() - MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        } else {
            x =  MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        }
        break;
    case PanelPosition::Bottom:
        if (layoutDirection() == Qt::LeftToRight) {
            x = availableGeo.x() + availableGeo.width() - m_basewidget->width() - MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - m_basewidget->height() - MARGIN;
        } else {
            x =  MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - m_basewidget->height() - MARGIN;
        }
        break;
    case PanelPosition::Left:
        x = availableGeo.x() + m_panelSize + MARGIN;
        y = availableGeo.y() + availableGeo.height() - m_basewidget->height() - MARGIN;
        break;
    case PanelPosition::Right:
        x = availableGeo.x() + availableGeo.width() - m_panelSize - m_basewidget->width() - MARGIN;
        y = availableGeo.y() + availableGeo.height() - m_basewidget->height() - MARGIN;
        break;
    }
    qDebug() << availableGeo << x << y;
    kdk::WindowManager::setGeometry(m_basewidget->windowHandle(), QRect(x, y, m_basewidget->width(),m_basewidget->height()));
}

/*
 * determine how to open the maininterface,if trigger is 0,the main interface show when we inset USB
 * device directly,if trigger is 1,we show main interface by clicking the systray icon.
*/
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    #if 0
    if(triggerType == 0)
    {
        if(event->type() == QEvent::Enter)
        {
            disconnect(interfaceHideTime, SIGNAL(timeout()), this, SLOT(onMaininterfacehide()));
            this->show();
        }

        if(event->type() == QEvent::Leave)
        {
            connect(interfaceHideTime, SIGNAL(timeout()), this, SLOT(onMaininterfacehide()));
            interfaceHideTime->start(2000);
        }
    }

    if(triggerType == 1){}

    if (obj == this)
    {
        if (event->type() == QEvent::WindowDeactivate && !(this->isHidden()))
        {
            this->hide();
            return true;
        }
    }
    if (!isActiveWindow())
    {
        activateWindow();
    }
    #endif
    return false;
}

// new a gsettings object to get the information of the opacity of the window
void MainWindow::initTransparentState()
{
    const QByteArray idtrans(THEME_QT_TRANS);

    if(QGSettings::isSchemaInstalled(idtrans))
    {
        m_transparency_gsettings = new QGSettings(idtrans);
    }
}

// use gsettings to get the opacity
void MainWindow::getTransparentData()
{
    if (!m_transparency_gsettings) {
       m_transparency = 0.95;
       return;
    }

    QStringList keys = m_transparency_gsettings->keys();
    if (keys.contains("transparency")) {
        m_transparency = m_transparency_gsettings->get("transparency").toDouble();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
//    QPainterPath path;
//    auto rect = this->rect();
//    rect.adjust(1, 1, -1, -1);
//    path.addRoundedRect(rect, 4, 4);
//    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    QRect rectReal = this->rect();
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(m_transparency);
//    p.setPen(Qt::NoPen);
//    p.drawRoundedRect(rectReal, 4, 4);
//    QWidget::paintEvent(event);
}

void MainWindow::onEjectVolumeForce(GVolume *v)
{
    g_volume_eject_with_operation(v, G_MOUNT_UNMOUNT_FORCE, nullptr, nullptr, GAsyncReadyCallback(&MainWindow::frobnitz_force_result_func), this);
}

void MainWindow::AsyncUnmount(QString strMountRoot,MainWindow *p_this)
{
    qInfo() << "dataPath:" << strMountRoot;
    QProcess p;
    p.setProgram("pkexec");
    p.setArguments(QStringList() << "eject" << strMountRoot);
    p.start();

    bool bSuccess = p.waitForFinished();

    if (p_this) {
        p_this->ifSucess = bSuccess;
    }
}

void MainWindow::frobnitz_force_result_func(GDrive *source_object,GAsyncResult *res, MainWindow *p_this)
{
    auto env = qgetenv("QT_QPA_PLATFORMTHEME");
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_drive_eject_with_operation_finish (source_object, res, &err);
    qInfo()<<"frobnitz_force_result_func";

    if (!err) {
        FDDriveInfo driveInfo;
        char *devPath = g_drive_get_identifier(source_object,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            driveInfo.strId = devPath;
            g_free(devPath);
        }

        char *strName = g_drive_get_name(source_object);
        if (strName) {
            driveInfo.strName = strName;
            g_free(strName);
        }

//        p_this->m_eject = new ejectInterface(p_this->ui->centralWidget);
//        p_this->m_eject->showEjectInterface();
        FlashDiskData::getInstance()->removeDriveInfo(driveInfo);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0) {
            p_this->m_systray->hide();
        }
    } else {
        GList* listVolume = g_drive_get_volumes(source_object);
        if (listVolume) {
            GList* lVolume = NULL;
            for (lVolume = listVolume; lVolume != NULL; lVolume= lVolume->next) {
                GVolume* gvolume = (GVolume *)lVolume->data;
                GMount* gmount = g_volume_get_mount(gvolume);
                if (gmount) {
                    GFile* root = g_mount_get_root(gmount);
                    if (root) {
                        char* strMountRoot = g_file_get_path(root);
                        if (strMountRoot) {
                            QString strMountPath = strMountRoot;
                            QtConcurrent::run(&MainWindow::AsyncUnmount, strMountPath, p_this);
                            g_free(strMountRoot);
                        }
                        g_object_unref(root);
                    }
                    g_object_unref(gmount);
                }
            }
            g_list_free(listVolume);
            listVolume = NULL;
        }
    }
}

void MainWindow::frobnitz_result_func(GDrive *source_object,GAsyncResult *res,MainWindow *p_this)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_drive_eject_with_operation_finish (source_object, res, &err);
    qInfo()<<"frobnitz_result_func";

    if (!err) {
        FDDriveInfo driveInfo;
        char *devPath = g_drive_get_identifier(source_object,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            driveInfo.strId = devPath;
            g_free(devPath);
        }

        char *strName = g_drive_get_name(source_object);
        if (strName) {
            driveInfo.strName = strName;
            g_free(strName);
        }

//        p_this->m_eject = new ejectInterface(p_this->ui->centralWidget);
//        p_this->m_eject->showEjectInterface();

        FlashDiskData::getInstance()->removeDriveInfo(driveInfo);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0) {
            p_this->m_systray->hide();
        }
    } else /*if(g_drive_can_stop(source_object) == true)*/ {
        FDDriveInfo driveInfo;
        char *devPath = g_drive_get_identifier(source_object,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            driveInfo.strId = devPath;
            g_free(devPath);
        }

        char *strName = g_drive_get_name(source_object);
        if (strName) {
            driveInfo.strName = strName;
            g_free(strName);
        }

        if (p_this->chooseDialog == nullptr) {
            p_this->chooseDialog = new interactiveDialog(QString::fromStdString(driveInfo.strId), p_this->ui->centralWidget);
        }else{
            p_this->chooseDialog->updateContentLable(QString::fromStdString(driveInfo.strId));
        }
        p_this->chooseDialog->showInteractivDialog();
        p_this->connect(p_this->chooseDialog,&interactiveDialog::FORCESIG,p_this,[=]() {
            g_drive_eject_with_operation(source_object,
                                         G_MOUNT_UNMOUNT_FORCE,
                                         NULL,
                                         NULL,
                                         GAsyncReadyCallback(&MainWindow::frobnitz_force_result_func),
                                         p_this
                                         );
            p_this->chooseDialog->close();
        });
    }
}

void MainWindow::frobnitz_normal_result_volume_eject(GVolume *source_object,GAsyncResult *res,MainWindow *p_this)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_volume_eject_with_operation_finish(source_object, res, &err);
    if(!err) {
        FDVolumeInfo volumeInfo;
        char *volumeId = g_volume_get_identifier(source_object,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (volumeId) {
            volumeInfo.strId = volumeId;
            g_free(volumeId);
        }

        FlashDiskData::getInstance()->removeVolumeInfo(volumeInfo);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0) {
            p_this->m_systray->hide();
        }
    }
}

void MainWindow::frobnitz_force_result_unmount(GMount *source_object,GAsyncResult *res,MainWindow *p_this)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_mount_unmount_with_operation_finish(source_object,res, &err);
    if(!err) {
        FDMountInfo mountInfo;
        mountInfo.isCanEject = g_mount_can_eject(source_object);
        char *mountId = g_mount_get_uuid(source_object);
        if (mountId) {
            mountInfo.strId = mountId;
            g_free(mountId);
        }

        GFile *root = g_mount_get_default_location(source_object);
        if (root) {
            char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
            if (mountUri) {
                mountInfo.strUri = mountUri;
                g_free(mountUri);
                if (mountInfo.strId.empty()) {
                    mountInfo.strId = mountInfo.strUri;
                }
            }
            g_object_unref(root);
        }

        FlashDiskData::getInstance()->removeMountInfo(mountInfo);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0)
        {
            p_this->m_systray->hide();
        }
    }
}

void MainWindow::onClickedEject(EjectDeviceInfo eDeviceInfo)
{
    m_curEjectDeviceInfo = eDeviceInfo;
    m_curEjectDeviceInfo.pVoid = this;
    m_curEjectDeviceInfo.uFlag = G_MOUNT_UNMOUNT_NONE;
    qDebug()<<"-------- eject !----";
    hideAllWidget(true);
    m_basewidget->hide();
    m_clickToEject = true;;
    doRealEject(&m_curEjectDeviceInfo, G_MOUNT_UNMOUNT_NONE);
}

bool MainWindow::onDeviceErrored(GDrive* drive)
{
    g_return_val_if_fail(G_IS_DRIVE(drive), false);

    g_autofree char* device = g_drive_get_identifier (drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);

    g_autofree char* name = g_drive_get_name (drive);

    qDebug() << "[onDeviceErrored] device:" << (device ? device : "NULL") << ", name:" << (name ? name : "NULL");

    g_return_val_if_fail(device && strstr(device, "/dev/sd"), false);
    g_return_val_if_fail (DeviceOperation::getDriveSize(drive) > 0, false);
    // FT ePass3000GM UKey not repair
    GList* volumes = g_drive_get_volumes(drive);
    int volumeCount = 0;
    for (GList* v = volumes; v != nullptr; v = v->next) ++volumeCount;
    qDebug() << "[onDeviceErrored] volumes count:" << volumeCount;
    kdk::KSoundEffects::playSound(SoundType::DIALOG_ERROR);  //播放系统音效
    if (!volumes && !mRepairDialog.contains(QString(device)) && !g_strrstr(name, "FT ePass3000GM")) {
        qDebug() << "[onDeviceErrored] No volumes, show repair dialog for drive.";
        RepairDialogBox *b = new RepairDialogBox(drive);
        b->setAttribute(Qt::WA_DeleteOnClose);
        b->connect(b, &RepairDialogBox::repairOK, this, [=] (RepairDialogBox* d) {
            if (mRepairDialog.contains(d->getDeviceName())) {
                b->mshowflag = false;
                mRepairDialog.remove(d->getDeviceName());
            }
        });
        mRepairDialog[QString(device)] = b;
        b->mshowflag = true;
        b->show();
    } else {
        for (auto v = volumes; nullptr != v; v = v->next) {
            GVolume* vv = G_VOLUME(v->data);
            g_autofree char* name = g_volume_get_name (vv);
            g_autofree char* volumeName = g_volume_get_identifier (vv, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
            g_autoptr(GMount) m = g_volume_get_mount(vv);
            if (!m  && !g_strrstr(name, "FT ePass3000GM")) {
                if (volumeName && !mRepairDialog.contains(QString(volumeName))) {
                    qDebug() << "[onDeviceErrored] Show repair dialog for volume:" << (volumeName ? volumeName : "NULL");
                    RepairDialogBox* b = new RepairDialogBox(vv);
                    b->setAttribute(Qt::WA_DeleteOnClose);
                    b->connect(b, &RepairDialogBox::remountDevice, this, &MainWindow::remountVolume);
                    b->connect(b, &RepairDialogBox::repairOK, this, [=] (RepairDialogBox* d) {
                        if (mRepairDialog.contains(d->getDeviceName())) {
                            b->mshowflag = false;
                            mRepairDialog.remove(d->getDeviceName());
                        }
                    });
                    mRepairDialog[QString(volumeName)] = b;
                    b->mshowflag = true;
                    b->show();
                }
            }
        }
        g_list_free_full(volumes, g_object_unref);
    }

    return true;
}

void MainWindow::onMountVolume(GVolume* v)
{
    g_volume_mount(v, G_MOUNT_MOUNT_NONE, nullptr, nullptr, GAsyncReadyCallback(frobnitz_result_func_volume), this);
}

bool MainWindow::doRealEject(EjectDeviceInfo* peDeviceInfo, GMountUnmountFlags flag)
{
    // find the device's drive & volume & mount
    if (!peDeviceInfo || !peDeviceInfo->pVoid) {
        return false;
    }

    GList *lDrive = NULL;
    GList *lVolume = NULL;
    GList *lMount = NULL;
    GList *current_drive_list = NULL;
    GList *current_volume_list = NULL;
    GList *current_mount_list = NULL;
    GDrive* devDrive = NULL;
    GVolume* devVolume = NULL;
    GMount* devMount = NULL;
    unsigned uVolumeSize = 0;
    bool isDone = false;
    bool isUmountable = false;

    //about drive
    GVolumeMonitor *g_volume_monitor = g_volume_monitor_get();
    if (!peDeviceInfo->strDriveId.isEmpty()) {
        current_drive_list = g_volume_monitor_get_connected_drives(g_volume_monitor);
        QString devName;
        for (lDrive = current_drive_list; lDrive != NULL; lDrive = lDrive->next) {
            GDrive *gdrive = (GDrive *)lDrive->data;
            char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
            if (devPath != NULL) {
                if(peDeviceInfo->strDriveId == devPath) {
                    devDrive = gdrive;
                    current_volume_list = g_drive_get_volumes(gdrive);
                    if (current_volume_list) {
                        uVolumeSize = g_list_length(current_volume_list);
                        for(lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next){ //遍历驱动器上的所有卷设备
                            GVolume* volume = (GVolume *)lVolume->data;
                            devMount = g_volume_get_mount(volume);
                            if(devMount)
                               isUmountable = g_mount_can_unmount(devMount);
                            char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                            if (volumeId) {
                                if (peDeviceInfo->strVolumeId != volumeId) {
                                    g_free(volumeId);
                                    continue;
                                }
                                g_free(volumeId);
                            } else {
                                continue ;
                            }
                            devVolume = volume;
                            break;
                        }
                    }
                    devName = devPath;
                }
                g_free(devPath);
            }
        }
        if((devName.startsWith ("/dev/sr"))&&ifBurnerBusy(devName)){
            qInfo()<<"the devname is"<<devName;
            if (this->chooseDialog == nullptr) {
                this->chooseDialog = new interactiveDialog(devName, this->ui->centralWidget);
            }else{
                this->chooseDialog->updateContentLable(devName);
            }
            this->chooseDialog->showInteractivDialog();
            return true;
        }

        if (devDrive != NULL) {
            auto mount_op = g_mount_operation_new();
            if (peDeviceInfo->strMountUri.isEmpty()) {
                g_drive_eject_with_operation(devDrive,
                        flag,
                        NULL,
                        NULL,
                        GAsyncReadyCallback(&MainWindow::frobnitz_result_func),
                        peDeviceInfo->pVoid);
                isDone = true;
            } else {                
                if (!(g_drive_can_stop(devDrive) || g_drive_can_eject(devDrive))&&isUmountable){
                    qInfo()<<"-------------umount-------------------";
                    if (current_volume_list) {
                        for(lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next){ //遍历驱动器上的所有卷设备
                            GVolume* volume = (GVolume *)lVolume->data;
                            devMount = g_volume_get_mount(volume);
                            g_mount_unmount_with_operation(devMount,
                                    flag,
                                    NULL,
                                    NULL,
                                    GAsyncReadyCallback(&MainWindow::frobnitz_force_result_unmount),
                                    peDeviceInfo->pVoid);
                         }
                    }
                    isDone = true;
                }
                if (!isDone && !devName.startsWith ("/dev/sr") && (g_drive_can_stop(devDrive) || g_drive_is_removable(devDrive))) {//for mobile harddisk.
                    g_drive_stop(devDrive, flag, mount_op, NULL,
                                 GAsyncReadyCallback(driveStopCb),
                                 peDeviceInfo);
                    isDone = true;
                }

                if (!isDone && g_drive_can_eject(devDrive)){//for udisk or DVD.
                    g_drive_eject_with_operation(devDrive,
                            flag,
                            NULL,
                            NULL,
                            GAsyncReadyCallback(&MainWindow::frobnitz_result_func),
                            peDeviceInfo->pVoid);
                    isDone = true;
                    #if 0
                    GFile *file = g_file_new_for_uri(peDeviceInfo->strMountUri.toUtf8().constData());
                    if (file) {
                        g_file_eject_mountable_with_operation(file,
                                                        flag,
                                                        nullptr,
                                                        nullptr,
                                                        GAsyncReadyCallback(fileEjectMountableCB),
                                                        peDeviceInfo);
                        g_object_unref(file);
                        isDone = true;
                    }
                    #endif
                }
            }
        }
        if (current_volume_list) {
            g_list_free(current_volume_list);
            current_volume_list = NULL;
        }
        if (current_drive_list) {
            g_list_free(current_drive_list);
            current_drive_list = NULL;
        }
    }

    //about mount not associated with a volume
    if (!isDone && !peDeviceInfo->strMountId.isEmpty()) {
        current_mount_list = g_volume_monitor_get_mounts(g_volume_monitor);
        if (current_mount_list) {
            for (lMount = current_mount_list; lMount != NULL; lMount = lMount->next) {
                GMount *gmount = (GMount *)lMount->data;
                QString strId = "";
                char *mountId = g_mount_get_uuid(gmount);
                if (mountId) {
                    strId = mountId;
                    g_free(mountId);
                }
                // get mount uri
                GFile *root = g_mount_get_default_location(gmount);
                if (root) {
                    char *mountUri = g_file_get_uri(root);           //get挂载点的uri路径
                    if (mountUri) {
                        if (strId.isEmpty()) {
                            strId = mountUri;
                        }
                        g_free(mountUri);
                    }
                    g_object_unref(root);
                }
                if (peDeviceInfo->strMountId == strId) {
                    devMount = gmount;
                    devDrive = g_mount_get_drive(gmount);       //bug146099在各别云桌面上可能存在无法通过volume找到gdrive的问题，增加通过Mount查找Gdrive移除设备
                    break;
                }
            }
            if (devMount != NULL) {
                if(devDrive!= NULL){
                    QString devNamefromDev = g_drive_get_identifier(devDrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
                    if(!devNamefromDev.startsWith("/dev/sr")&& (g_drive_can_stop(devDrive) || g_drive_is_removable(devDrive))){
                        g_drive_stop(devDrive, flag, NULL, NULL, GAsyncReadyCallback(driveStopCb), peDeviceInfo);
                        isDone = true;                      
                    }else{
                        qInfo()<<"can_stop:"<<g_drive_can_stop(devDrive)<<"is remove:"<<g_drive_is_removable(devDrive);
                    }
                }else{
                    g_mount_unmount_with_operation(devMount,
                            flag,
                            NULL,
                            NULL,
                            GAsyncReadyCallback(&MainWindow::frobnitz_force_result_unmount),
                            peDeviceInfo->pVoid);
                    isDone = true;
                }
            }
            g_list_free(current_mount_list);
            current_mount_list = NULL;
        }
    }

    //about volume not associated with a drive
    if (!isDone && !peDeviceInfo->strVolumeId.isEmpty()) {
        current_volume_list = g_volume_monitor_get_volumes(g_volume_monitor);
        if (current_volume_list) {
            for (lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next) {
                GVolume *volume = (GVolume *)lVolume->data;
                char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                if (volumeId) {
                    if (peDeviceInfo->strVolumeId != volumeId) {
                        g_free(volumeId);
                        continue;
                    }
                    g_free(volumeId);
                } else {
                    continue ;
                }
                devVolume = volume;
                break;
            }
        }
        if (devVolume != NULL) {
            GMount* pCurMount = g_volume_get_mount(devVolume);
            if (pCurMount) {
                g_mount_unmount_with_operation(pCurMount,
                        flag,
                        NULL,
                        NULL,
                        GAsyncReadyCallback(&MainWindow::frobnitz_force_result_unmount),
                        peDeviceInfo->pVoid);
                g_object_unref(pCurMount);
                isDone = true;
            }
        }
        if (current_volume_list) {
            g_list_free(current_volume_list);
            current_volume_list = NULL;
        }
    }

    if (!isDone) {
        FDMountInfo mountInfo;
        FDVolumeInfo volumeInfo;
        FDDriveInfo driveInfo;
        mountInfo.strId = peDeviceInfo->strMountId.toStdString();
        m_dataFlashDisk->removeMountInfo(mountInfo);
        volumeInfo.strId = peDeviceInfo->strVolumeId.toStdString();
        m_dataFlashDisk->removeVolumeInfo(volumeInfo);
        driveInfo.strId = peDeviceInfo->strDriveId.toStdString();
        m_dataFlashDisk->removeDriveInfo(driveInfo);
        if(m_dataFlashDisk->getValidInfoCount() == 0) {
            m_systray->hide();
        }
    }
    return true;
}

GAsyncReadyCallback MainWindow::fileEjectMountableCB(GFile *file, GAsyncResult *res, EjectDeviceInfo *peDeviceInfo)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_file_eject_mountable_with_operation_finish(file, res, &err);
    qInfo()<<"fileEjectMountableCB";
    if (!err) {
        FDDriveInfo driveInfo;
        driveInfo.strId = peDeviceInfo->strDriveId.toStdString();
        driveInfo.strName = peDeviceInfo->strDriveName.toStdString();
        MainWindow* pThis = (MainWindow*)(peDeviceInfo->pVoid);
//        pThis->m_eject = new ejectInterface(pThis->ui->centralWidget);
//        pThis->m_eject->showEjectInterface();
        FlashDiskData::getInstance()->removeDriveInfo(driveInfo);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0) {
            pThis->m_systray->hide();
        }
    } else /*if(g_drive_can_stop(source_object) == true)*/ {
        if (peDeviceInfo->uFlag != G_MOUNT_UNMOUNT_FORCE) {
            FDDriveInfo driveInfo;
            driveInfo.strId = peDeviceInfo->strDriveId.toStdString();
            driveInfo.strName = peDeviceInfo->strDriveName.toStdString();
            MainWindow* pThis = (MainWindow*)(peDeviceInfo->pVoid);
            if (pThis->chooseDialog == nullptr) {
                pThis->chooseDialog = new interactiveDialog(QString::fromStdString(driveInfo.strId), pThis->ui->centralWidget);
            }
            pThis->chooseDialog->showInteractivDialog();
            pThis->connect(pThis->chooseDialog,&interactiveDialog::FORCESIG,pThis,[=]() {
                pThis->chooseDialog->close();
                peDeviceInfo->uFlag = G_MOUNT_UNMOUNT_FORCE;
                pThis->doRealEject(peDeviceInfo, G_MOUNT_UNMOUNT_FORCE);
            });
        }
    }
    return nullptr;
}

void MainWindow::driveStopCb(GObject* object, GAsyncResult* res, EjectDeviceInfo *peDeviceInfo)
{
    gboolean success =  FALSE;
    GError *err = nullptr;
    success = g_drive_stop_finish(G_DRIVE(object), res, &err);
    qInfo() << "driveStopCb:" << success;

    if (success || !err || (G_IO_ERROR_FAILED_HANDLED == err->code)) {
        FDDriveInfo driveInfo;
        driveInfo.strId = peDeviceInfo->strDriveId.toStdString();
        driveInfo.strName = peDeviceInfo->strDriveName.toStdString();
        MainWindow* pThis = (MainWindow*)(peDeviceInfo->pVoid);
//        pThis->m_eject = new ejectInterface(NULL);
//        pThis->m_eject->showEjectInterface();
        FlashDiskData::getInstance()->removeDriveInfo(driveInfo);
        qInfo() << "driveStopCb:" <<peDeviceInfo->strMountUri<<peDeviceInfo->strVolumeId<<peDeviceInfo->strDriveId;
        pThis->m_dataFlashDisk->emitRemoveNotify(peDeviceInfo->strMountUri,peDeviceInfo->strVolumeId);
        if(FlashDiskData::getInstance()->getValidInfoCount() == 0) {
            pThis->m_systray->hide();
        }
    } else {
        if (!strcmp(err->message,"Not authorized to perform operation")){/* gmountOperation会弹出授权框，防止二次弹框 */
            return;
        }
        if (peDeviceInfo->uFlag != G_MOUNT_UNMOUNT_FORCE) {
            FDDriveInfo driveInfo;
            driveInfo.strId = peDeviceInfo->strDriveId.toStdString();
            driveInfo.strName = peDeviceInfo->strDriveName.toStdString();
            MainWindow* pThis = (MainWindow*)(peDeviceInfo->pVoid);
            if (pThis->chooseDialog == nullptr) {
                pThis->chooseDialog = new interactiveDialog(QString::fromStdString(driveInfo.strId), pThis->ui->centralWidget);
            }
            pThis->chooseDialog->showInteractivDialog();
            pThis->connect(pThis->chooseDialog,&interactiveDialog::FORCESIG,pThis,[=]() {
                pThis->chooseDialog->close();
                peDeviceInfo->uFlag = G_MOUNT_UNMOUNT_FORCE;
                pThis->doRealEject(peDeviceInfo, G_MOUNT_UNMOUNT_FORCE);
            });
        }
    }
}

void MainWindow::onRemountVolume(FDVolumeInfo volumeInfo)
{
    if (volumeInfo.strId.empty()) {
        return;
    }

    if (!(g_str_has_prefix(volumeInfo.strId.c_str(),"/dev/bus") || g_str_has_prefix(volumeInfo.strId.c_str(),"/dev/sd"))) {
        return;
    }

    qInfo() << "volumeInfo.strId:" << volumeInfo.strId.c_str();
    GList *lDrive = NULL;
    GList *lVolume = NULL;
    GList *current_drive_list = NULL;
    GList *current_volume_list = NULL;
    GDrive* devDrive = NULL;
    GVolume* devVolume = NULL;
    bool isDone = false;

    //about drive
    GVolumeMonitor *g_volume_monitor = g_volume_monitor_get();
    current_drive_list = g_volume_monitor_get_connected_drives(g_volume_monitor);
    for (lDrive = current_drive_list; lDrive != NULL; lDrive = lDrive->next) {
        GDrive *gdrive = (GDrive *)lDrive->data;
        char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        if (devPath != NULL) {
            devDrive = gdrive;
            current_volume_list = g_drive_get_volumes(gdrive);
            if (current_volume_list) {
                for(lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next){ //遍历驱动器上的所有卷设备
                    GVolume* volume = (GVolume *)lVolume->data;
                    char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                    if (volumeId) {
                        if (volumeInfo.strId != volumeId) {
                            g_free(volumeId);
                            continue;
                        }
                        g_free(volumeId);
                    } else {
                        continue ;
                    }
                    devVolume = volume;
                    break;
                }
            }
            g_free(devPath);
        }
    }
    if (devVolume != NULL) {
        isDone = true;
        GMount* pCurMount = g_volume_get_mount(devVolume);
        if (pCurMount) {
            g_object_unref(pCurMount);
        } else {
            volumeInfo.isCanMount = g_volume_can_mount(devVolume);
            qInfo()<<"Volume canmount:"<<volumeInfo.isCanMount<<"|"<<ifsettings->get(IFAUTOLOAD).toBool();
            if(ifsettings->get(IFAUTOLOAD).toBool()) {
                g_volume_mount(devVolume,
                            G_MOUNT_MOUNT_NONE,
                            nullptr,
                            nullptr,
                            GAsyncReadyCallback(frobnitz_result_func_volume),
                            this);
            }
        }
    }
    if (current_volume_list) {
        g_list_free(current_volume_list);
        current_volume_list = NULL;
    }
    if (current_drive_list) {
        g_list_free(current_drive_list);
        current_drive_list = NULL;
    }

    //about volume not associated with a drive
    if (!isDone) {
        current_volume_list = g_volume_monitor_get_volumes(g_volume_monitor);
        if (current_volume_list) {
            for (lVolume = current_volume_list; lVolume != NULL; lVolume = lVolume->next) {
                GVolume *volume = (GVolume *)lVolume->data;
                GDrive *gdrive = g_volume_get_drive(volume);
                if (!gdrive) {
                    char *volumeId = g_volume_get_identifier(volume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
                    if (volumeId) {
                        if (volumeInfo.strId != volumeId) {
                            g_free(volumeId);
                            continue;
                        }
                        g_free(volumeId);
                    } else {
                        continue ;
                    }
                    devVolume = volume;
                    break;
                } else {
                    g_object_unref(gdrive);
                }
            }
        }
        if (devVolume != NULL) {
            isDone = true;
            GMount* pCurMount = g_volume_get_mount(devVolume);
            if (pCurMount) {
                g_object_unref(pCurMount);
            } else {
                volumeInfo.isCanMount = g_volume_can_mount(devVolume);
                qInfo()<<"Volume canmount:"<<volumeInfo.isCanMount<<"|"<<ifsettings->get(IFAUTOLOAD).toBool();
                if(ifsettings->get(IFAUTOLOAD).toBool()) {
                    g_volume_mount(devVolume,
                                G_MOUNT_MOUNT_NONE,
                                nullptr,
                                nullptr,
                                GAsyncReadyCallback(frobnitz_result_func_volume),
                                this);
                }
            }
        }
        if (current_volume_list) {
            g_list_free(current_volume_list);
            current_volume_list = NULL;
        }
    }
}

void MainWindow::onCheckDriveValid(FDDriveInfo driveInfo)
{
    if (driveInfo.strId.empty()) {
        return;
    }

    qInfo() << "driveInfo.strId:" << driveInfo.strId.c_str();
    if(!this->isSystemRootDev(driveInfo.strId.c_str()) &&
        (driveInfo.isCanEject || driveInfo.isCanStop || driveInfo.isRemovable)) {
        if(g_str_has_prefix(driveInfo.strId.c_str(),"/dev/sr") || g_str_has_prefix(driveInfo.strId.c_str(),"/dev/bus")
            || g_str_has_prefix(driveInfo.strId.c_str(),"/dev/sd") || g_str_has_prefix(driveInfo.strId.c_str(),"/dev/mmcblk")) {

            GList *lDrive = NULL;
            GList *current_drive_list = NULL;
            GDrive* devDrive = NULL;

            //about drive
            GVolumeMonitor *g_volume_monitor = g_volume_monitor_get();
            current_drive_list = g_volume_monitor_get_connected_drives(g_volume_monitor);
            for (lDrive = current_drive_list; lDrive != NULL; lDrive = lDrive->next) {
                GDrive *gdrive = (GDrive *)lDrive->data;
                char *devPath = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
                if (devPath != NULL) {
                    string strId = devPath;
                    if (strId == driveInfo.strId) {
                        devDrive = gdrive;
                        g_free(devPath);
                        break;
                    }
                    g_free(devPath);
                }
            }
            if (devDrive != NULL) {
                if (!g_drive_has_volumes(devDrive)) {
                    Q_EMIT this->deviceError(devDrive);
                }
            }
            if (current_drive_list) {
                g_list_free(current_drive_list);
                current_drive_list = NULL;
            }
        }
    }
}

bool MainWindow::getDataCDRomCapacity(QString strDevId, quint64 &totalCapacity)
{
    if (!strDevId.startsWith("/dev/sr")) {
        return false;
    }

    quint64 uTotalCapacity = 0;
    DataCDROM *cdrom = new DataCDROM(strDevId);
    if (cdrom) {
        cdrom->getCDROMInfo();
        uTotalCapacity = cdrom->getCDROMCapacity();
        delete cdrom;
        cdrom = nullptr;
    }
    if (uTotalCapacity > 0) {
        totalCapacity = uTotalCapacity;
        return true;
    } else {
        qInfo()<<"the CD/DVD's capacicy is 0";
        return false;
    }
}

void MainWindow::getDriveIconsInfo(GDrive* drive, FDDriveInfo& driveInfo)
{
    GIcon *g_icon = g_drive_get_icon(drive);
    if (g_icon) {
        if (G_IS_THEMED_ICON(g_icon)) {
            const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
            if (icon_names) {
                driveInfo.strIconPath = *icon_names;
            }
        }
        g_object_unref(g_icon);
    }
}

void MainWindow::getVolumeIconsInfo(GVolume* volume, FDVolumeInfo& volumeInfo)
{
    GIcon *g_icon = g_volume_get_icon(volume);
    if (g_icon) {
        if (G_IS_THEMED_ICON(g_icon)) {
            const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
            if (icon_names) {
                volumeInfo.strIconPath = *icon_names;
            }
        }
        g_object_unref(g_icon);
    }
}

void MainWindow::getMountIconsInfo(GMount* mount, FDMountInfo& mountInfo)
{
    GIcon *g_icon = g_mount_get_icon(mount);
    if (g_icon) {
        if (G_IS_ICON(g_icon)) {
            if (G_IS_THEMED_ICON(g_icon)) {
                const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
                if (icon_names) {
                    mountInfo.strIconPath = *icon_names;
                }
            } else {
                // if it's a bootable-media,maybe we can get the icon from the mount directory.
                char *bootableIcon = g_icon_to_string(g_icon);
                if(bootableIcon) {
                    mountInfo.strIconPath = bootableIcon;
                    g_free(bootableIcon);
                }
            }
        }
        g_object_unref(g_icon);
    }
}

//check if burner is busy
bool MainWindow::ifBurnerBusy(QString strDev){

    bool ret = false;
    QDBusInterface interface( "com.kylin.burner.manager",
                              "/com/kylin/burner/manager",
                              "com.kylin.burner.manager",
                              QDBusConnection::sessionBus() );
    if(interface.isValid()){
        QDBusReply<bool> reply = interface.call("isBusyDevice", strDev);
        ret = reply;
    }
    return ret;
}

//check if gparted is busy
bool MainWindow::ifGpartedBusy(){

    QString filename = CONFIG_FILE_PATH;
    QSettings m_settings(filename, QSettings::IniFormat);
    bool ret = true;
    bool ifmounted = true;
    m_settings.setIniCodec("UTF-8");
    m_settings.beginGroup("Gparted");
    ifmounted = m_settings.value("automounted").toBool();

    //判断分区编辑器进程是否存在
    FILE* fp;
    char szbuffer[16] = {0};
    fp = popen("ps -ef|grep gpartedbin | grep -v grep |wc -l", "r");
    //1：存在 0：不存在
    size_t sizeread = fread(szbuffer, 1, 1, fp);    //第一个字节就是fp读取出来的结果 0或1，一个字节就可以，sizeof(szbuffer)会有空格补齐，strcmp对比时有误
    if (sizeread > 0){                              //不对size做判断会有告警
        if(strcmp(szbuffer, "1") == 0){
            qInfo()<<"gparted is exist.";
             if(!ifmounted)
                 ret = false;
        }
    }else{
        qInfo()<<"read false";
    }

    pclose(fp);

    return ret;
}

#if UDFAUTOMOUNT
static void eject_cdrom (GDrive* drive, gpointer u)
{
    qDebug() << "DJ-  eject!!!";

    g_return_if_fail (G_IS_DRIVE (drive));

    g_autofree char* devName = g_drive_get_identifier (drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);

    if (devName) {
        if (gMountCdrom->contains (QString(devName))) {
            gMountCdrom->remove (QString(devName));
            QStringList mountPoints;
            for (auto val : gMountCdromMountPoint->values()) {
                if (QString(devName) == val) {
                    mountPoints << val;
                }
            }
            for (auto m : mountPoints)
                gMountCdromMountPoint->remove (m);
        }
    }
}

static void mount_cdrom (GDrive* drive, gpointer u)
{
    g_return_if_fail(G_IS_DRIVE (drive));

    if (!gMountCdrom) {
        gMountCdrom = new QMap<QString, bool>();
    }

    if (!gMountCdromMountPoint) {
        gMountCdromMountPoint = new QMap<QString, QString>();
    }

    bool canMount = true;
    if (g_drive_has_media (drive)) {
        g_autofree char* devName = g_drive_get_identifier (drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        const char* name = g_get_user_name ();
        if (devName && name) {

            QString mountPoint = QString("/media/%1/%2").arg(name)
                    .arg(QString(QCryptographicHash::hash(devName, QCryptographicHash::Md5).toBase64()
                    .replace ("/", "").replace ("+","").replace ("=", "")));

            if (gMountCdrom->contains (devName) && !(*gMountCdrom)[QString(devName)]) {
                canMount = false;
            }

            if (canMount) {
                int ret = QProcess::execute ("pkexec", QStringList()
                        << "/usr/bin/kylin-device-daemon-pkexec.sh"
                        << "-d"
                        << devName
                        << "-p"
                        << mountPoint);

                qDebug() << "DJ- execute 'mount_cdrom' return: " << ret;

                if (!gMountCdrom->contains (devName)) {
                    gMountCdrom->insert (QString(devName), true);
                    gMountCdromMountPoint->insert (mountPoint, QString(devName));
                }
            }
        }
    } else {
        eject_cdrom (drive, u);
    }
}
#endif

QString MainWindow::convertToDBusPath(const QString &strVolId)
{
    QString deviceId = strVolId;

    if (deviceId.startsWith("/dev/")) {
        deviceId = deviceId.mid(5);
    }

    if (deviceId.startsWith("dm-")) {
        deviceId.replace("-", "_2d");
    }

    return QString("/org/freedesktop/UDisks2/block_devices/%1").arg(deviceId);
}

void MainWindow::setMaxHeightLimit()
{
    auto tmpScreen = getCurrentScreen();
    if (!tmpScreen) {
        tmpScreen = QGuiApplication::primaryScreen();
    }
    QRect availableGeo = tmpScreen->geometry();
    int maxAvailableHeight = 0;
    switch (m_panelPosition) {
    case PanelPosition::Top:
    case PanelPosition::Bottom:
        maxAvailableHeight = availableGeo.height() - m_panelSize - 2 * MARGIN;
        break;
    case PanelPosition::Left:
    case PanelPosition::Right:
        maxAvailableHeight = availableGeo.height() - 2 * MARGIN;
        break;
    }
    m_basewidget->setMaxHeight(maxAvailableHeight);
}

bool MainWindow::shouldIgnoreDeviceDisconnect(GDrive *drive, FDDriveInfo &driveInfo)
{
    // 获取设备基本信息
    g_autofree char* deviceName = g_drive_get_name(drive);
    char *devPath = g_drive_get_identifier(drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    
    qInfo() << "Device name:" << (deviceName ? deviceName : "NULL");
    qInfo() << "Device path:" << (devPath ? devPath : "NULL");
    
    if (ifSpecicalDev(QString(deviceName))) {
        qInfo() << "Skipping special device:" << deviceName;
        if (devPath) g_free(devPath);
        return true;
    }
    
    if (devPath != NULL) {
        driveInfo.strId = devPath;
        g_free(devPath);
    } else {
        qInfo() << "No device path available";
        return true;
    }
    
    gboolean canEject = g_drive_can_eject(drive);
    gboolean canStop = g_drive_can_stop(drive);
    gboolean canStart = g_drive_can_start(drive);
    gboolean isRemovable = g_drive_is_removable(drive);
    gboolean hasMedia = g_drive_has_media(drive);
    gboolean hasVolumes = g_drive_has_volumes(drive);
    
    qInfo() << "Device capabilities - canEject:" << canEject 
            << "canStop:" << canStop 
            << "canStart:" << canStart 
            << "isRemovable:" << isRemovable 
            << "hasMedia:" << hasMedia 
            << "hasVolumes:" << hasVolumes;
    
    if (isSystemRootDev(driveInfo.strId.c_str())) {
        qInfo() << "Skipping system root device:" << driveInfo.strId.c_str();
        return true;
    }
    
#if 0
    if (ifSataDev(driveInfo.strId.c_str())) {
        qInfo() << "Skipping SATA device:" << driveInfo.strId.c_str();
        return true;
    }
#endif
    
    if (!isRemovable && !canEject && !canStop) {
        qInfo() << "Skipping non-removable device:" << driveInfo.strId.c_str();
        return true;
    }
    
    if (!hasVolumes && QString(deviceName).contains("SD/MMC", Qt::CaseInsensitive)) {
        qInfo() << "Skipping empty SD/MMC reader:" << driveInfo.strId.c_str();
        return true;
    }

    QString deviceNameStr = QString(deviceName ? deviceName : "");
    if (isEmptyCardReaderByConfig(deviceNameStr, hasMedia, hasVolumes)) {
        qInfo() << "Skipping empty card reader (by config):" << driveInfo.strId.c_str();
        return true;
    }
    
    return false;
}

bool MainWindow::isEmptyCardReaderByConfig(const QString &deviceName, gboolean hasMedia, gboolean hasVolumes)
{
    if (hasMedia || hasVolumes) {
        return false;
    }

    QString filename = CONFIG_FILE_PATH;
    QSettings settings(filename, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    // Check for card reader filter
    settings.beginGroup("cardreaderfilter");
    QString cardReaderNames = settings.value("devicename", "").toString();
    settings.endGroup();

    if (cardReaderNames.isEmpty()) {
        return false;
    }

    QStringList cardReaderList = cardReaderNames.split(",");
    for (const QString &cardReader : cardReaderList) {
        QString trimmedCardReader = cardReader.trimmed();
        if (!trimmedCardReader.isEmpty() &&
            deviceName.contains(trimmedCardReader, Qt::CaseInsensitive)) {
            qInfo() << "Detected empty card reader by config:" << deviceName << "matches" << trimmedCardReader;
            return true;
        }
    }

    return false;
}

void MainWindow::mount_result_func_volume_with_error(GVolume *source_object, GAsyncResult *res, MainWindow *p_this)
{
    GError *err = nullptr;
    gboolean success = g_volume_mount_finish(source_object, res, &err);
    // Debug log for mount attempt
    char *volumeId = g_volume_get_identifier(source_object, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    char *volumeName = g_volume_get_name(source_object);
    qDebug() << "[mount_result_func_volume_with_error] volumeId:" << (volumeId ? volumeId : "NULL")
             << ", volumeName:" << (volumeName ? volumeName : "NULL")
             << ", success:" << success;
    if (!success && err) {
        QString errMsg = QString::fromUtf8(err->message ? err->message : "");
        qDebug() << "[mount_result_func_volume_with_error] Mount failed, error code:" << err->code << ", error message:" << errMsg;
        // Typical filesystem corruption keywords
        if (errMsg.contains("bad superblock", Qt::CaseInsensitive) ||
            errMsg.contains("wrong fs type", Qt::CaseInsensitive) ||
            errMsg.contains("unknown filesystem", Qt::CaseInsensitive) ||
            errMsg.contains("missing codepage", Qt::CaseInsensitive) ||
            errMsg.contains("corrupted", Qt::CaseInsensitive)) {
            GDrive* gdrive = g_volume_get_drive(source_object);
            char *driveId = gdrive ? g_drive_get_identifier(gdrive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE) : nullptr;
            char *driveName = gdrive ? g_drive_get_name(gdrive) : nullptr;
            qDebug() << "[mount_result_func_volume_with_error] Filesystem corruption detected, driveId:" << (driveId ? driveId : "NULL")
                     << ", driveName:" << (driveName ? driveName : "NULL");
            Q_EMIT p_this->deviceError(gdrive);
            if (driveId) g_free(driveId);
            if (driveName) g_free(driveName);
            if (gdrive) {
                g_object_unref(gdrive);
            }
        } else {
            qDebug() << "[mount_result_func_volume_with_error] Mount failed but not due to filesystem corruption, no repair dialog triggered.";
        }
    } else if (!success) {
        qDebug() << "[mount_result_func_volume_with_error] Mount failed, but no GError info.";
    } else {
        qDebug() << "[mount_result_func_volume_with_error] Mount succeeded.";
    }
    if (volumeId) g_free(volumeId);
    if (volumeName) g_free(volumeName);
    if (err) {
        g_error_free(err);
    }
}

void MainWindow::autoMountLocalDrivers()
{
    qDebug() << "kylin-device-daemon: auto mount local volumes start";
    
    GVolumeMonitor* vm = g_volume_monitor_get();
    if (!vm) {
        qWarning() << "kylin-device-daemon: failed to get volume monitor";
        return;
    }
    
    GList* drives = g_volume_monitor_get_connected_drives(vm);
    if (drives) {
        for (GList* i = drives; nullptr != i; i = i->next) {
            GDrive* d = static_cast<GDrive*>(i->data);
            if (G_IS_DRIVE(d)) {
                GList* volumes = g_drive_get_volumes(d);
                if (volumes) {
                    for (GList* j = volumes; nullptr != j; j = j->next) {
                        GVolume* v = static_cast<GVolume*>(j->data);
                        if (G_IS_VOLUME(v)) {
                            g_autofree char* uuid = g_volume_get_uuid(v);
                            if (0 != g_strcmp0("2691-6AB8", uuid)) {
                                GMount* mount = g_volume_get_mount(v);
                                if (!mount) {
                                    qDebug() << "kylin-device-daemon: auto mounting volume with uuid:" << uuid;
                                    g_volume_mount(v, G_MOUNT_MOUNT_NONE, nullptr, nullptr, nullptr, this);
                                } else {
                                    g_object_unref(mount);
                                }
                            }
                            g_object_unref(v);
                        }
                    }
                    g_list_free(volumes);
                }
                g_object_unref(d);
            }
        }
        g_list_free(drives);
    }
    g_object_unref(vm);
    
    qDebug() << "auto mount local volumes finished";
}

static void guessContentTypeCallback(GObject* object, GAsyncResult *res, gpointer data)
{
    qInfo() << "guessContentTypeCallback start";
    char **guessType;
    GError *error = nullptr;
    QString openFolderCmd;
    GFile* root;
    char *mountUri;
    bool openFolder;
    static QString lastMountUri;

    openFolder = true;
    root = g_mount_get_default_location(G_MOUNT(object));

    // fix #205338
    g_autoptr(GFileInfo) access_info = g_file_query_info(root, 
        G_FILE_ATTRIBUTE_ACCESS_CAN_READ "," G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, 
        G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    openFolder = g_file_info_get_attribute_boolean(access_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ) && 
                 g_file_info_get_attribute_boolean(access_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);

    mountUri = g_file_get_uri(root);
    openFolderCmd = "/usr/bin/peony " + QString(mountUri);

    guessType = g_mount_guess_content_type_finish(G_MOUNT(object), res, &error);

    if (error) {
        qInfo() << "error in guess content type:" << error->message;
        g_error_free(error);
    } else {
        GDrive *drive = g_mount_get_drive(G_MOUNT(object));
        g_autofree char *unixDevice = nullptr;
        if (drive) {
            unixDevice = g_drive_get_identifier(drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
            g_object_unref(drive);
        }

        if (guessType && g_strv_length(guessType) > 0) {
            int n;
            for (n = 0; guessType[n]; ++n) {
                qDebug() << "processing guess type:" << guessType[n];
                
                if (g_content_type_is_a(guessType[n], "x-content/image-dcf")) {
                    openFolder = false;
                }
                if (g_content_type_is_a(guessType[n], "x-content/win32-software")) {
                    openFolder = false;
                }
                if (unixDevice && !strcmp(guessType[n], "x-content/bootable-media") && !strstr(unixDevice, "/dev/sr")) {
                    openFolder = false;
                }
                if (!strcmp(guessType[n], "x-content/blank-dvd") || !strcmp(guessType[n], "x-content/blank-cd")) {
                    openFolder = false;
                }
                if (strstr(guessType[n], "x-content/audio-")) {
                    if (!lastMountUri.compare(mountUri)) {
                        lastMountUri.clear();
                        break;
                    }
                    lastMountUri = mountUri;
                }
                
                // Fix the issue of automatically opening a new window in the file manager when mounting ISO files
                QString currentType = QString(guessType[n]);
                if (openFolder && !unixDevice && currentType == "x-content/bootable-media")
                    openFolder = false;

                if (!openFolder)
                    break;

                QString uri = mountUri;
                if (uri.startsWith("gphoto") || uri.startsWith("mtp")) {
                    break;
                }

                if (g_content_type_is_a(guessType[n], "x-content/ostree-repository")) {
                    openFolder = false;
                    break;
                }

                if (openFolder) {
                    qInfo() << "opening folder:" << openFolderCmd;
                    QProcess::startDetached(openFolderCmd);
                }

                openFolder = true;
            }
            g_strfreev(guessType);
        } else {
            // Only DvD devices are allowed to open folder automatically.
            if (unixDevice && strstr(unixDevice, "/dev/sr") && QGSettings::isSchemaInstalled("org.gnome.desktop.media-handling")) {
                QGSettings autoMountSettings("org.gnome.desktop.media-handling");
                if (autoMountSettings.get("automount-open").toBool()) {
                    qInfo() << "DVD automount, opening folder:" << openFolderCmd;
                    QProcess::startDetached(openFolderCmd);
                }
            }
        }
    }

    g_free(mountUri);
    g_object_unref(root);
    g_object_unref(object);
    qInfo() << "guessContentTypeCallback finished";
}

void MainWindow::guessContentTypeAndOpenFolder(const QString &mountUri)
{
    qDebug() << "guess content type for:" << mountUri;
    
    GFile *file = g_file_new_for_uri(mountUri.toUtf8().constData());
    if (!file) {
        qWarning() << "failed to create GFile";
        return;
    }
    
    GMount *mount = g_file_find_enclosing_mount(file, nullptr, nullptr);
    g_object_unref(file);
    
    if (!mount) {
        qWarning() << "failed to find mount";
        return;
    }
    
    g_autofree char *uuid = g_mount_get_uuid(mount);
    if (uuid && 0 == g_strcmp0("2691-6AB8", uuid)) {
        qDebug() << "special volume, skip auto-open";
        g_object_unref(mount);
        return;
    }
    
    g_mount_guess_content_type(mount, FALSE, nullptr, guessContentTypeCallback, nullptr);
}

QScreen* MainWindow::getCurrentScreen()
{
#ifdef KY_SDK_WAYLANDHELPER
    if (m_basewidget && m_basewidget->windowHandle()) {
        UkuiWindowHelper windowHelper(m_basewidget->windowHandle());
        windowHelper.setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
        QScreen* screen = windowHelper.getCurrentScreenBlocking();
        if (screen) {
            qDebug() << "[getCurrentScreen] Got screen from UkuiWindowHelper:" << screen->name();
            return screen;
        }
    }
    qDebug() << "[getCurrentScreen] UkuiWindowHelper unavailable, fallback to cursor position";
#endif

    auto tmpScreen = QGuiApplication::screenAt(QCursor::pos());
    if (tmpScreen) {
        qDebug() << "[getCurrentScreen] Got screen from cursor position:" << tmpScreen->name();
    } else {
        qDebug() << "[getCurrentScreen] Cursor position failed, using primary screen";
    }
    return tmpScreen;
}
