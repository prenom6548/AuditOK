#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QDebug>
#include <QSettings>
#include <gsettingmonitor.h>
#include <QDesktopWidget>
#include <QScreen>
#include <QIcon>
#include <QPixmap>
#include "tipwidget.h"
#include "generatetools.h"
#include "config.h"
#include "windowmanage.hpp"

#define KYLIN_STATUS_MANAGER_PATH "/"
#define KYLIN_STATUS_MANAGER_NAME "com.kylin.statusmanager.interface"
#define KYLIN_STATUS_MANAGER_INTERFACE "com.kylin.statusmanager.interface"

#define KYLIN_CONNECTIVITY_MANAGER_PATH "/"
#define KYLIN_CONNECTIVITY_MANAGER_SERVICE "com.kylin.connectivity.manager"
#define KYLIN_CONNECTIVITY_MANAGER_INTERFACE "com.kylin.connectivity.manager"

#define GNOME_SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define GNOME_SESSION_MANAGER_NAME "org.gnome.SessionManager"
#define GNOME_SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

const QString DISC_SERVER_PATH = "connectivity_softbus_server";
const QString DISC_CLIENT_PATH = "connectivity_softbus_client";
const QString FILE_DOWN_PATH = getenv("HOME") + QString("/.connectivitycache/cache/");
const QString TEMP_DOWN_PATH = "/temp/";
const QString FILE_PATH_PCSEARCH = QString("/.connectivitycache/pcsearchInfo.db");

const int USBPORT = 27183;
const int WIFIPORT = 27186;
const int USB_FTP_PORT = 27184;
const int WIFI_FTP_PORT = 27187;
const int FTP_SERVER_PORT = 27189;
const QString USB_FTPURL = "127.0.0.1";
const QString LOCAL_ROOT = getenv("HOME") + QString("/");
const int FTP_MAX_THREAD = 5;
const int SIDE_MARGIN = 70;
const int MSGICON_H = 24;
const int MSGICON_W = 24;
const int SEARCH_LOAD_MAX_NUM = 5;

quint32 MainWindow::getWinId() const
{
    return m_winId;
}

QString MainWindow::getDeviceName()
{
    return m_connectInfo.deviceName;
}

void MainWindow::list(QString path)
{
    if (!m_isDbusOperation) {
        // 手机存储埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantMobileStorage);
        qInfo() << "External request file list information.";
        m_isDbusOperation = true;
        m_ftpClient->list(path);
    }
}

void MainWindow::downFile(const QList<FileInfo> &fileList, QString downloadPath)
{
    if (!m_isDbusOperation) {
        // 下载文件埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantMobileStorage);
        qInfo() << "External requests to download the file.";
        m_isDbusOperation = true;
        m_ftpClient->downAllFiles(fileList, downloadPath);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->hide();
    if (m_isConnect) {
        slotDisconnect();
    }
    if (mSingleInstanceFd) {
        lockf(mSingleInstanceFd, F_ULOCK, 0);
    }
    QWidget::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        m_titlebar->initHelp();
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = this->windowState();
        qInfo() << "Window state change: " << state;
        if (state == Qt::WindowNoState) {
            // 窗口模式
            m_titlebar->setMaxBtnMode(false);
        } else if (state == Qt::WindowMaximized) {
            // 最大化模式
            m_titlebar->setMaxBtnMode(true);
        }
    }
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    kabase::WindowManage::getWindowId(&m_winId);
    qRegisterMetaType<QMap<QString, FileInfo>>("QMap<QString,FileInfo>");
    initUI();
    initGsetting();
    initDbus();
    initService();
}

MainWindow::~MainWindow()
{
    deleterUI();
    deleterService();
}

void MainWindow::initService()
{
    m_connectionService = new ConnectionService(this);
    QStringList list;
    GenerateTools::getUserPassword(list);
    m_connectionService->setUserPassword(list.value(GenerateTools::KEY_USERPASS::KEY_USER),
                                         list.value(GenerateTools::KEY_USERPASS::KEY_PASSWORD));
    connect(m_connectionService, &ConnectionService::sigConnectRequest, this, &MainWindow::slotConnectRequest);
    connect(m_connectionService, &ConnectionService::sigConnectDenied, this, &MainWindow::slotConnectDenied);
    connect(m_connectionService, &ConnectionService::sigConnectInfo, this, &MainWindow::slotConnect);
    connect(m_connectionService, &ConnectionService::sigDisconnect, this, &MainWindow::slotDisconnect);
    connect(m_connectionService, &ConnectionService::sigNotFountApk, this, &MainWindow::slotNotFountApk);
    connect(m_connectionService, &ConnectionService::sigNoUsbDevice, this, &MainWindow::slotNoUsbDevice);

    // 监听USB插拔信号
    m_usbManage = new UsbManage;
    connect(m_usbManage, &UsbManage::sigVolumeConnected, this, &MainWindow::slotVolumeConnected);
    connect(m_usbManage, &UsbManage::sigVolumeDisconnected, this, &MainWindow::slotVolumeDisconnected);

    // 监听wifi
    m_wifiManager = new WifiManager;
    connect(m_wifiManager, &WifiManager::sigWifiConnected, this, &MainWindow::slotWifiConnected);
    connect(m_wifiManager, &WifiManager::sigWifiDisconnected, this, &MainWindow::slotWifiDisConnected);

    m_wifiManagerThread = new QThread;
    m_wifiManager->moveToThread(m_wifiManagerThread);
    connect(m_wifiManagerThread, &QThread::started, m_wifiManager, &WifiManager::startWifiInfo);
    m_wifiManagerThread->start();

    m_dirManager = new DirManager;
    connect(m_dirManager, &DirManager::finish, m_dirManager, [=]() {
        if (m_dirManager != nullptr) {
            m_dirManager->deleteLater();
            m_dirManager = nullptr;
        }
    });
    m_dirManager->start();

    // 启动ftp服务端
    m_ftpServer = new FtpServer(FTP_SERVER_PORT);
    m_ftpServer->addUser(list.value(GenerateTools::KEY_USERPASS::KEY_USER).toStdString(),
                         list.value(GenerateTools::KEY_USERPASS::KEY_PASSWORD).toStdString(), LOCAL_ROOT.toStdString(),
                         fineftp::Permission::All);
    m_ftpServer->start(FTP_MAX_THREAD);

    m_pcScreen = new PcScreenManage;
    connect(m_pcScreen, &PcScreenManage::sigRequestReceived, this, &MainWindow::slotRequestReceived);

    m_discovery = new KDiscovery::Discovery();
    connect(m_discovery, &KDiscovery::Discovery::sigAllDeviceInfo, this, &MainWindow::slotAllDeviceInfo);

    discInit();
}

void MainWindow::initGsetting()
{
    /* 主题 */
    changeTheme();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
            &MainWindow::changeTheme);
    changeFontSize();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this,
            &MainWindow::changeFontSize);
    if (QGSettings::isSchemaInstalled(UKUI_STYLE_GSETTING_PATH)) {
        m_fontData = new QGSettings(UKUI_STYLE_GSETTING_PATH);
        connect(m_fontData, &QGSettings::changed, this, &MainWindow::changeFont);
    }
}

void MainWindow::initDbus()
{
    // 右键发送
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(KYLIN_CONNECTIVITY_SEND_SERVICE)) {
        qWarning() << "init send file dbus fail!";
    }
    sessionBus.registerObject(KYLIN_CONNECTIVITY_SEND_PATH, this, QDBusConnection::ExportAllSlots);

    // 管理服务，挂载ftp，启动软总线。清理端口占用
    m_dbusInterface = new QDBusInterface(KYLIN_CONNECTIVITY_MANAGER_SERVICE, KYLIN_CONNECTIVITY_MANAGER_PATH,
                                         KYLIN_CONNECTIVITY_MANAGER_INTERFACE, QDBusConnection::systemBus());
    QObject::connect(m_dbusInterface, SIGNAL(sigMountError(bool, QString)), this, SLOT(slotMountError(bool, QString)));
    QObject::connect(m_dbusInterface, SIGNAL(sigInitDiscServer(bool)), this, SLOT(slotInitDiscServerResult(bool)));
    // 临时方案
    if (m_dbusInterface->isValid()) {
        m_dbusInterface->call("killServerPortProcess");
    }

    // 平板模式切换
    m_tableModeListener = new TableModeListener(this);
    connect(m_tableModeListener, &TableModeListener::sigModeChange, this, &MainWindow::slotModeChanged);
    slotModeChanged(m_tableModeListener->getMode());

    // 防止熄屏
    m_activityDbusInterface = new QDBusInterface(GNOME_SESSION_MANAGER_NAME, GNOME_SESSION_MANAGER_PATH,
                                                 GNOME_SESSION_MANAGER_INTERFACE, QDBusConnection::sessionBus());
}

void MainWindow::disconnectScreen()
{
    // 断开连接
    if (m_deviceManage != nullptr) {
        // 清空设备名
        m_deviceManage->deleteLater();
        m_deviceManage = nullptr;
    }
    if (m_pcScreen != nullptr) {
        m_pcScreen->disconnected();
    }
    if (m_suspendTabBar != nullptr) {
        m_suspendTabBar->deleteLater();
        m_suspendTabBar = nullptr;
    }
}

void MainWindow::disconnectFtp()
{
    if (m_ftpClient != nullptr) {
        if (m_isMountFtp) {
            umountFtp(MOUNT_DIR);
            m_isMountFtp = false;
        }
        m_ftpClient->deleteLater();
        m_ftpClient = nullptr;
    }
}

void MainWindow::stopAllDown()
{
    if (m_fileSyncManage != nullptr) {
        m_fileSyncManage->abortDown();
        m_fileSyncManage->deleteLater();
        m_fileSyncManage = nullptr;
    }
}

void MainWindow::disconnectService()
{
    stopAllDown();
    disconnectScreen();
    disconnectFtp();
    if (m_searchServer != nullptr) {
        delete m_searchServer;
        m_searchServer = nullptr;
    }
    if (m_searchThread != nullptr) {
        m_searchThread->quit();
        m_searchThread->wait();
        m_searchThread->deleteLater();
        m_searchThread = nullptr;
    }
    if (m_activityDbusInterface->isValid()) {
        m_activityDbusInterface->call("Uninhibit", m_inhibitValue);
    }
    if (m_connectionService != nullptr) {
        m_connectionService->abortService();
    }
}

void MainWindow::deleterService()
{
    if (m_usbManage != nullptr) {
        qInfo() << "close usb manage!";
        m_usbManage->deleteLater();
        m_usbManage = nullptr;
    }
    if (m_wifiManager != nullptr) {
        qInfo() << "close wifi manage!";
        m_wifiManager->deleteLater();
        m_wifiManager = nullptr;
    }
    if (m_ftpServer != nullptr) {
        qInfo() << "close ftpserver!";
        m_ftpServer->stop();
        delete m_ftpServer;
        m_ftpServer = nullptr;
    }
    if (m_dirManager != nullptr) {
        qInfo() << "close dir manage!";
        m_dirManager->stopRun();
        m_dirManager->requestInterruption();
        m_dirManager->quit();
        m_dirManager->wait();
        m_dirManager->deleteLater();
        m_dirManager = nullptr;
    }
    if (m_pcScreen != nullptr) {
        qInfo() << "close pcscreen manage!";
        m_pcScreen->deleteLater();
        m_pcScreen = nullptr;
    }
    if (m_activityDbusInterface != nullptr) {
        qInfo() << "close activity dbus!";
        m_activityDbusInterface->deleteLater();
        m_activityDbusInterface = nullptr;
    }
    if (m_dbusInterface != nullptr) {
        deInitDisc();
        qInfo() << "kill all server!";
        if (m_dbusInterface->isValid()) {
            m_dbusInterface->asyncCall("killServerPortProcess");
            m_dbusInterface->asyncCall("killConnectivity");
        }
        qInfo() << "close sever dbus!";
        m_dbusInterface->deleteLater();
        m_dbusInterface = nullptr;
    }
    if (m_wifiManagerThread != nullptr) {
        qInfo() << "close wifi manage thread!";
        m_wifiManagerThread->requestInterruption();
        m_wifiManagerThread->quit();
        m_wifiManagerThread->wait();
        m_wifiManagerThread->deleteLater();
        m_wifiManagerThread = nullptr;
    }
}

bool MainWindow::mountFtp(QString url, QString userName, QString pwd, QString path)
{
    if (m_dbusInterface->isValid()) {
        qInfo() << "Start mounting FTP...";
        m_dbusInterface->asyncCall("mountFtp", url, userName, pwd, path);
        return true;
    }
    return false;
}

bool MainWindow::umountFtp(QString path)
{
    if (m_dbusInterface->isValid()) {
        qInfo() << "Start umounting FTP...";
        m_dbusInterface->asyncCall("umountFtp", path);
        return true;
    }
    return false;
}

void MainWindow::clearConnectionInfo()
{
    m_connectInfo.uuid.clear();
    m_connectInfo.deviceName.clear();
    m_connectInfo.address.clear();
    m_connectInfo.connectType = ConnectionService::ConnectType::NOTCONNECT;
    m_connectInfo.usernamePwd.username.clear();
    m_connectInfo.usernamePwd.pwd.clear();
}

void MainWindow::initUI()
{
    setWindowTitle(tr("kylin-connectivity"));

    m_titlebar = new Titlebar(this);
    connect(m_titlebar, &Titlebar::sigAboutWinShow, this, &MainWindow::slotAboutWinShow);

    m_homepage = new HomePage;
    m_homepage->setDeviceCode(m_wifiIp);
    connect(m_homepage, &HomePage::sigConnectBtnClicked, this, &MainWindow::slotConnectBtnClicked);

    m_connectInterfaceWin = new ConnectInterface;
    connect(m_connectInterfaceWin, &ConnectInterface::sigUSBconnectBtnClicked, this,
            &MainWindow::slotUSBConnectOnClicked);
    connect(m_connectInterfaceWin, &ConnectInterface::sigBackBtnClicked, this, &MainWindow::slotReturnHomePage);
    connect(m_connectInterfaceWin, &ConnectInterface::sigConnectAddress, this, &MainWindow::slotConnectServiceUI);
    connect(m_connectInterfaceWin, &ConnectInterface::sigContinueSearch, this, &MainWindow::slotContinueSearch);

    m_connectInterfaceWin->setInterFaceCodeInfo(m_wifiIp);

    m_mainStackedWin = new QStackedWidget(this);
    m_mainStackedWin->addWidget(m_homepage);
    m_mainStackedWin->setCurrentIndex(WinIndex::WinHomePage);
    m_mainStackedWin->addWidget(m_connectInterfaceWin);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->setSpacing(0);
    hLayout2->setMargin(0);
    hLayout2->addWidget(m_mainStackedWin);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(m_titlebar);
    vLayout->addLayout(hLayout2);

    this->setLayout(vLayout);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

void MainWindow::connectUI()
{
    deleterDialog();
    if (!m_isConnect) {
        m_isConnect = true;
        m_fileManageWin = new FileManageWin(this);
        connect(m_fileManageWin, &FileManageWin::sigCrumbIndexChange, this, &MainWindow::slotCrumbIndexChange);
        connect(m_fileManageWin, &FileManageWin::sigBtnCliked, this, &MainWindow::slotFileManageBtnClicked);
        connect(m_fileManageWin, &FileManageWin::sigSearchTextChanged, this, &MainWindow::slotSearchTextChanged);
        m_fileManageWin->load();
        m_fileManageWin->changeFontSize(m_fontSize);
        m_connectedWin = new ConnectedWin();
        connect(m_connectedWin, &ConnectedWin::sigBtnClicked, this, &MainWindow::slotConnectedWinBtnClicked);
        m_connectedWin->setTheme(m_theme);
        m_connectedWin->addWidget(m_fileManageWin);
        m_connectedWin->setDeviceName(m_connectInfo.deviceName);
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::PC) {
            m_connectedWin->setDeviceType(PublicAttributes::DeviceType::Pc);
            if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::INITIATOR) {
                m_connectedWin->changeScreenButton(false);
            } else if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::RECIPIENT) {
                m_connectedWin->changeScreenButton(true);
            }

            m_fileManageWin->pushCrumb(m_connectInfo.deviceName);
            m_isFileViewPage = true;
        } else if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
            bool isInitiator = false;
            if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::INITIATOR) {
                isInitiator = true;
            } else if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::RECIPIENT) {
                isInitiator = false;
            }
            m_connectedWin->setDeviceType(PublicAttributes::DeviceType::Android, isInitiator);

            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList));
            m_isFileViewPage = false;
        }
        m_connectedWin->changeFontSize(m_fontSize);
        m_mainStackedWin->addWidget(m_connectedWin);
        m_mainStackedWin->setCurrentIndex(WinIndex::WinConnected);

        m_connectInterfaceWin->setSearchBtnState(false);
        m_titlebar->setBackground(false);

        // 暂时取消usb中文输入法提示
        // if (m_connectInfo.connectType == ConnectionService::ConnectType::USB) {
        //     QString str1 = QString(tr("USB connection device succeeded!"));
        //     QString str2 = QString(
        //         tr("If you need to input Chinese on the mobile phone screen on the computer, please download the "
        //            "extension tool in the personal information input method on the mobile phone."));
        //     m_messageBox = new MessageDialog(this);
        //     m_messageBox->setText(str1, str2);
        //     m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(MSGICON_W, MSGICON_H));
        //     m_messageBox->addButton(tr("OK"));
        //     moveMessageBox();
        // }
        m_discovery->stopDiscovery();

        QString str1 = "";
        if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::INITIATOR) {
            str1 = QString(tr("Agreed to connect"));
        } else {
            str1 = QString(tr("Peer has agreed"));
        }
        QString str2 = QString(tr("Establishing connection, please wait..."));
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(str1, str2);
        m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success"));
        m_messageBox->addButton(tr("CANCEL"));
        connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *) { slotDisconnect(); });
        moveMessageBox();
    }
}

void MainWindow::updateResult()
{
    clearView();
    if (m_downFileType == FileSyncManage::FileType::FileNum) {
        QMap<int, int> fileNumMap;
        switch (m_fileKey) {
            case FileSyncManage::FileKey::Default: {
                m_androidHomePage = new AndroidHomePage(this);
                connect(m_androidHomePage, &AndroidHomePage::sigBtnClicked, this, &MainWindow::slotAndroidBtnClicked);
                m_androidHomePage->setTheme(m_theme);
                m_fileSyncManage->getItemCount(FileParSer::KeyType::All, fileNumMap);
                QMap<int, int>::iterator it = fileNumMap.begin();
                while (it != fileNumMap.end()) {
                    m_androidHomePage->setItemCount(AndroidItem::Type(it.key()), it.value());
                    it++;
                }
                m_androidHomePage->changeFontSize(m_fontSize);
                m_fileManageWin->setWidget(m_androidHomePage, FileManageWin::TabType::AndroidHomePage);
            } break;
            case FileSyncManage::FileKey::WeChat: {
                m_fileSyncManage->getItemCount(FileParSer::KeyType::WeChat, fileNumMap);
                initMultiMediaWin(fileNumMap);
            } break;
            case FileSyncManage::FileKey::QQ: {
                m_fileSyncManage->getItemCount(FileParSer::KeyType::QQ, fileNumMap);
                initMultiMediaWin(fileNumMap);
            } break;
        }
    } else {
        QList<FileInfo> list;
        m_fileSyncManage->getClassifiedFileList(m_fileKey, m_downFileType, list);
        m_fileView = new FileView(this);
        connect(m_fileView, &FileView::sigOpenFile, this, &MainWindow::slotOpenFile);
        connect(m_fileView, &FileView::sigDownFileInfo, this, &MainWindow::slotDownFile);
        connect(m_fileView, &FileView::sigItemSelectAll, this, &MainWindow::slotFileItemSelecAll);
        m_fileView->setViewInfo(list);
        m_fileView->setViewMode(m_viewMode);
        m_fileView->setModelFlag(m_model);
        m_fileView->setTheme(m_theme);
        m_fileView->setItemMode(FileView::ItemSelectMode::Default, true);
        m_fileView->setAllowDrag(false);
        m_fileView->changeFontSize(m_fontSize);
        m_fileManageWin->setWidget(m_fileView, FileManageWin::TabType::Default);
        if (m_downFileType == FileSyncManage::FileType::Picture) {
            m_fileView->setThumbnailType(FileView::ThumbnailType::Image);
            m_isDownloadingThumbanil = true;
            m_fileSyncManage->downThumb(m_fileKey, m_downFileType);
        } else if (m_downFileType == FileSyncManage::FileType::Video) {
            m_fileView->setThumbnailType(FileView::ThumbnailType::Video);
            m_isDownloadingThumbanil = true;
            m_fileSyncManage->downThumb(m_fileKey, m_downFileType);
        }
    }
}

void MainWindow::initMultiMediaWin(QMap<int, int> &map)
{
    m_multiMediaWin = new MultiMediaWin(this);
    connect(m_multiMediaWin, &MultiMediaWin::sigBtnClicked, this, &MainWindow::slotAndroidBtnClicked);
    m_multiMediaWin->setTheme(m_theme);
    QMap<int, int>::iterator it = map.begin();
    while (it != map.end()) {
        m_multiMediaWin->setItemCount(AndroidItem::Type(it.key()), it.value());
        it++;
    }
    m_multiMediaWin->changeFontSize(m_fontSize);
    m_fileManageWin->setWidget(m_multiMediaWin, FileManageWin::TabType::AppHomePage);
}

void MainWindow::initTransmissionDialog()
{
    m_transmissionDialog = new TransmissionDialog(this);
    connect(m_transmissionDialog, &TransmissionDialog::sigCancelAbort, this, &MainWindow::slotAbortTransmission);
    m_transmissionDialog->setProgressBarRange(0, 100);
    QRect availableGeometry = geometry();
    m_transmissionDialog->move(availableGeometry.center() - m_transmissionDialog->rect().center());
    m_transmissionDialog->show();
}

void MainWindow::clearView()
{
    deleterDialog();
    if (m_isDownloadingThumbanil && m_fileSyncManage != nullptr) {
        m_isDownloadingThumbanil = false;
        m_fileSyncManage->abortDownThumb();
    }
    if (m_androidHomePage != nullptr) {
        m_androidHomePage->deleteLater();
        m_androidHomePage = nullptr;
    }
    if (m_multiMediaWin != nullptr) {
        m_multiMediaWin->deleteLater();
        m_multiMediaWin = nullptr;
    }
    if (m_fileView != nullptr) {
        m_fileView->deleteLater();
        m_fileView = nullptr;
    }
    if (m_tipDisplayWin != nullptr) {
        m_tipDisplayWin->deleteLater();
        m_tipDisplayWin = nullptr;
    }
    if (m_tipDisplayLay != nullptr) {
        m_tipDisplayLay->deleteLater();
        m_tipDisplayLay = nullptr;
    }
}

void MainWindow::setToolTipWin(QString msg, kdk::TipType type)
{
    if (m_tipWin != nullptr) {
        delete m_tipWin;
        m_tipWin = nullptr;
    }
    m_tipWin = new kdk::KBallonTip(this);
    m_tipWin->setWindowFlag(Qt::FramelessWindowHint);
    m_tipWin->setAttribute(Qt::WA_TranslucentBackground);
    m_tipWin->setTipType(type);
    m_tipWin->setText(msg);
    m_tipWin->setTipTime(2000);
    m_tipWin->setContentsMargins(4, 0, 8, 0);
    m_tipWin->setFixedHeight(40 + m_fontSize);
    m_tipWin->move((this->geometry().x() + ((this->width() - m_tipWin->width()) >> 1)),
                   (this->geometry().y() + ((this->height() - m_tipWin->height()) >> 1) + 250));
    m_tipWin->activateWindow();
    m_tipWin->showInfo();
}

void MainWindow::deleterUI()
{
    if (m_titlebar != nullptr) {
        m_titlebar->deleteLater();
        m_titlebar = nullptr;
    }
    if (m_connectInterfaceWin != nullptr) {
        m_connectInterfaceWin->deleteLater();
        m_connectInterfaceWin = nullptr;
    }
    if (m_homepage != nullptr) {
        m_homepage->deleteLater();
        m_homepage = nullptr;
    }
    if (m_tipWin != nullptr) {
        m_tipWin->deleteLater();
        m_tipWin = nullptr;
    }
}

void MainWindow::startTimer()
{
    deleterDialog();
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::slotConnectFailed, Qt::DirectConnection);
    m_timer->start(10000);
}

void MainWindow::initSearchServer(QString searchPath)
{
    qInfo() << "searchInfo install...";
    m_searchServer = new SearchServer(searchPath);
    connect(m_searchServer, &SearchServer::sigLoadFail, this, &MainWindow::slotSearchLoadFail);
    connect(m_searchServer, &SearchServer::sigSearchResult, this, &MainWindow::slotSearchResult);

    m_searchThread = new QThread;
    m_searchServer->moveToThread(m_searchThread);
    m_searchThread->start();
    Q_EMIT m_searchServer->sigInitSearch();
}

void MainWindow::startLoadUI()
{
    if (!m_isOpeningFile) {
        startTimer();
    }
    m_loadDialog = new LoadDialog(this);
    m_loadDialog->move(this->geometry().x() + ((this->width() - m_loadDialog->width()) >> 1),
                       this->geometry().y() + (((this->height() - m_loadDialog->height()) >> 1)));
    m_loadDialog->show();
}

void MainWindow::deleterDialog()
{
    if (m_timer != nullptr) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (m_loadDialog != nullptr) {
        m_loadDialog->deleteLater();
        m_loadDialog = nullptr;
    }
    if (m_messageBox != nullptr) {
        m_messageBox->hide();
        m_messageBox->deleteLater();
        m_messageBox = nullptr;
    }
    if (m_transmissionDialog != nullptr) {
        m_transmissionDialog->deleteLater();
        m_transmissionDialog = nullptr;
        setToolTipWin(tr("file download failed"), kdk::TipType::Error);
    }
}

void MainWindow::initPcSearchInfo()
{
    m_isPcSearchInfo = true;
    m_ftpClient->setTempPath(FILE_DOWN_PATH + m_connectInfo.uuid + "/");
    m_ftpClient->downFile(FILE_PATH_PCSEARCH);
    // ftp服务暂时不支持qt5的短连接请求，待修复
    // m_fileSyncManage = new FileSyncManage(m_connectInfo.uuid, m_url, this);
    // m_fileSyncManage->setSearchPath(FileSyncManage::SearchFileType::Pc, m_connectInfo.deviceName);
    // connect(m_fileSyncManage, SIGNAL(sigDownFileFail(QString, QString)), this,
    //         SLOT(slotDownFileFail(QString, QString)));
    // connect(m_fileSyncManage, &FileSyncManage::sigTempFileDownFinish, this, &MainWindow::slotDownTempFileFinish);
    // m_fileSyncManage->downloadSearch();
}

void MainWindow::deleteTransmissionDialog()
{
    if (m_transmissionDialog != nullptr) {
        m_transmissionDialog->deleteLater();
        m_transmissionDialog = nullptr;
    }
}

void MainWindow::deleteStackedWidget()
{
    if (m_fileManageWin != nullptr) {
        m_fileManageWin->deleteLater();
        m_fileManageWin = nullptr;
    }
    if (m_connectedWin != nullptr) {
        m_mainStackedWin->removeWidget(m_connectedWin);
        m_connectedWin->deleteLater();
        m_connectedWin = nullptr;
    }
}

void MainWindow::initMobileFileSync()
{
    m_fileSyncManage = new FileSyncManage(m_connectInfo.uuid, m_url, this);
    connect(m_fileSyncManage, &FileSyncManage::sigDownFileFinish, this, &MainWindow::slotDownFileFinish);
    connect(m_fileSyncManage, &FileSyncManage::sigDownAllFileFinish, this, &MainWindow::slotDownAllFileFinish);
    connect(m_fileSyncManage, SIGNAL(sigDownFileFail(QString, QString)), this,
            SLOT(slotDownFileFail(QString, QString)));
    connect(m_fileSyncManage, &FileSyncManage::sigSearchInfoFinsh, this, &MainWindow::slotSearchInfoFinish);
    connect(m_fileSyncManage, &FileSyncManage::sigTempFileDownFinish, this, &MainWindow::slotDownTempFileFinish);
    m_fileSyncManage->setSearchPath(FileSyncManage::SearchFileType::Android);

    checkAndroidHomePage();
}

int MainWindow::getScreenIndex()
{
    QDesktopWidget *desktopWidget = QApplication::desktop();
    for (int i = 0; i < desktopWidget->screenCount(); i++) {
        if (desktopWidget->screenGeometry(i).contains(this->geometry())) {
            return i;
        }
    }
    return 0;
}

void MainWindow::discInit()
{
    if (m_dbusInterface->isValid()) {
        qInfo() << "softbus init";
        m_dbusInterface->asyncCall("startDiscServer", DISC_SERVER_PATH);
    }
}

void MainWindow::deInitDisc()
{
    qInfo() << "deInitDisc!";
    if (m_discovery != nullptr) {
        m_discovery->stopDiscovery();
        m_discovery->deleteLater();
        m_discovery = nullptr;
    }
    if (m_dbusInterface->isValid()) {
        qInfo() << "softbus deinit";
        m_dbusInterface->asyncCall("stopDiscServer");
    }
    if (m_discClient) {
        if (m_discClient->state() == QProcess::Running) {
            m_discClient->close();
        }
        m_discClient->deleteLater();
        m_discClient = nullptr;
    }
}

void MainWindow::addBackList(QStringList &list)
{
    if (m_isFileViewPage) {
        m_forwardList.clear();
        m_fileManageWin->setGoForwardEnabled(false);
    }
    m_backList.append(list);
    m_fileManageWin->setGoBackEnabled(true);
}

void MainWindow::checkAndroidHomePage()
{
    m_backList.clear();
    m_forwardList.clear();
    m_fileManageWin->setGoBackEnabled(false);
    m_fileManageWin->setGoForwardEnabled(false);
    m_downFileType = FileSyncManage::FileType::FileNum;
    m_fileKey = FileSyncManage::FileKey::Default;
    m_isFileViewPage = false;
    m_fileSyncManage->updateFileInfo(m_fileKey, m_downFileType);
}

void MainWindow::checkFileView(QStringList &pathList)
{
    pathList.removeAll(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Storage));
    pathList.removeAll(m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList));
    pathList.removeAll(m_connectInfo.deviceName);
    QString path = "/";
    path.append(pathList.join("/"));
    slotCdDirectory(path);
}

void MainWindow::crumbTagToAndroidItemType(QString tag)
{
    AndroidItem::Type type;
    if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::Picture)) {
        type = AndroidItem::Type::Picture;
    } else if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::Video)) {
        type = AndroidItem::Type::Video;
    } else if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::Music)) {
        type = AndroidItem::Type::Music;
    } else if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::Doc)) {
        type = AndroidItem::Type::Doc;
    } else if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::QQ)) {
        type = AndroidItem::Type::QQ;
    } else if (tag == m_fileManageWin->crumbText(FileManageWin::CrumbTag::WeChat)) {
        type = AndroidItem::Type::WeChat;
    }
    slotAndroidBtnClicked(type);
}

void MainWindow::checkPage(QStringList crumbPathList)
{
    m_fileManageWin->clearCrumb();
    if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
        m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList));
        if (crumbPathList.size() == 1) {
            checkAndroidHomePage();
        } else {
            if (m_isFileViewPage) {
                for (int i = 1; i < crumbPathList.size(); i++) {
                    m_fileManageWin->pushCrumb(crumbPathList.value(i));
                }
                checkFileView(crumbPathList);
            } else {
                switch (crumbPathList.size()) {
                    case 2: {
                        crumbTagToAndroidItemType(crumbPathList.value(1));
                    } break;
                    case 3: {
                        if (crumbPathList.value(1) == m_fileManageWin->crumbText(FileManageWin::CrumbTag::QQ)) {
                            m_fileKey = FileSyncManage::FileKey::QQ;
                            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::QQ));
                        } else {
                            m_fileKey = FileSyncManage::FileKey::WeChat;
                            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::WeChat));
                        }
                        crumbTagToAndroidItemType(crumbPathList.value(2));
                    } break;
                }
            }
        }
    } else {
        m_fileManageWin->pushCrumb(m_connectInfo.deviceName);
        for (int i = 1; i < crumbPathList.size(); i++) {
            m_fileManageWin->pushCrumb(crumbPathList.value(i));
        }
        checkFileView(crumbPathList);
    }
}

void MainWindow::moveMessageBox()
{
    QRect availableGeometry = geometry();
    m_messageBox->move(availableGeometry.center() - m_messageBox->rect().center());
    m_messageBox->show();
}

void MainWindow::switchUploadPath()
{
    if (!m_uploadPath.isEmpty()) {
        m_fileManageWin->clearCrumb();
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList));
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Storage));
        } else {
            m_fileManageWin->pushCrumb(m_connectInfo.deviceName);
        }
        QStringList pathList = m_uploadPath.split("/");
        pathList.removeAll("");
        for (QString path : pathList) {
            m_fileManageWin->pushCrumb(path);
        }
        m_ftpClient->cdToDir(m_uploadPath);
        m_uploadPath.clear();
    }
}

void MainWindow::setTipDisplayWin(QString text)
{
    m_tipDisplayWin = new QLabel(this);
    QPalette palettes = m_tipDisplayWin->palette();
    if (m_theme == PublicAttributes::Theme::Light) {
        palettes.setColor(QPalette::Text, QColor("#8C8C8C"));
    } else {
        palettes.setColor(QPalette::Text, QColor("#8F9399"));
    }
    m_tipDisplayWin->setPalette(palettes);
    QFont font;
    font.setPointSizeF(m_fontSize + 10);
    m_tipDisplayWin->setFont(font);
    m_tipDisplayWin->setText(text);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_tipDisplayWin);
    layout->addStretch();

    m_tipDisplayLay = new TipDisplayWin(this);
    connect(m_tipDisplayLay, &TipDisplayWin::sigUploadFileInfo, this, &MainWindow::slotUploadFile);

    m_tipDisplayLay->setLayout(layout);
}

void MainWindow::setLocalPathList(const QStringList &list)
{
    qInfo() << "get local peony menu send path list" << list;
    if (!m_isConnect) {
        deleterDialog();
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(tr("Not currently connected, please connect"));
        m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
        m_messageBox->addButton(tr("OK"));
        moveMessageBox();
        qInfo() << "peony menu send: not current connect";
        return;
    }
    m_isDusUpload = true;
    if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
        slotUploadFile("/Download", list);
    } else if (m_connectInfo.deviceType == ConnectionService::DeviceType::PC) {
        slotUploadFile("/下载", list);
    }
}

void MainWindow::changeTheme()
{
    qInfo() << "changeLightTheme";
    QString themeStr = kdk::GsettingMonitor::getSystemTheme().toString();
    if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
        m_theme = PublicAttributes::Theme::Dark;
    } else {
        m_theme = PublicAttributes::Theme::Light;
    }
    if (m_titlebar != nullptr) {
        m_titlebar->setTheme(m_theme);
    }
    if (m_homepage != nullptr) {
        m_homepage->setTheme(m_theme);
    }
    if (m_connectInterfaceWin != nullptr) {
        m_connectInterfaceWin->setTheme(m_theme);
    }
    if (m_connectedWin != nullptr) {
        m_connectedWin->setTheme(m_theme);
    }
    if (m_pcScreen != nullptr) {
        m_pcScreen->setTheme(m_theme);
    }
    if (m_suspendTabBar != nullptr) {
        m_suspendTabBar->setTheme(m_theme);
    }
    if (m_androidHomePage != nullptr) {
        m_androidHomePage->setTheme(m_theme);
    }
    if (m_multiMediaWin != nullptr) {
        m_multiMediaWin->setTheme(m_theme);
    }
    if (m_fileView != nullptr) {
        m_fileView->setTheme(m_theme);
    }
    if (m_tipDisplayWin != nullptr) {
        QPalette palettes = m_tipDisplayWin->palette();
        if (m_theme == PublicAttributes::Theme::Light) {
            palettes.setColor(QPalette::Text, QColor("#8C8C8C"));
        } else {
            palettes.setColor(QPalette::Text, QColor("#8F9399"));
        }
        m_tipDisplayWin->setPalette(palettes);
    }
}

void MainWindow::changeFontSize()
{
    m_fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    if (m_homepage != nullptr) {
        m_homepage->changeFontSize(m_fontSize);
    }
    if (m_titlebar != nullptr) {
        m_titlebar->changeFontSize(m_fontSize);
    }
    if (m_connectInterfaceWin != nullptr) {
        m_connectInterfaceWin->changeFontSize(m_fontSize);
    }
    if (m_connectedWin != nullptr) {
        m_connectedWin->changeFontSize(m_fontSize);
    }
    if (m_androidHomePage != nullptr) {
        m_androidHomePage->changeFontSize(m_fontSize);
    }
    if (m_multiMediaWin != nullptr) {
        m_multiMediaWin->changeFontSize(m_fontSize);
    }
    if (m_fileManageWin != nullptr) {
        m_fileManageWin->changeFontSize(m_fontSize);
    }
    if (m_fileView != nullptr) {
        m_fileView->changeFontSize(m_fontSize);
    }
    if (m_tipDisplayWin != nullptr) {
        QFont font;
        font.setPointSizeF(m_fontSize + 10);
        font.setBold(true);
        m_tipDisplayWin->setFont(font);
    }
}

void MainWindow::changeFont(const QString &key)
{
    if (key == "systemFont") {
        qInfo() << "The font changes...";
        changeFontSize();
    }
}

void MainWindow::slotVolumeConnected(QString volumeName)
{
    // todo
}

void MainWindow::slotVolumeDisconnected(QString volumeName)
{
    if (m_connectInfo.connectType == ConnectionService::ConnectType::USB && m_connectInfo.deviceName.isEmpty()) {
        qInfo() << m_connectInfo.deviceName << "disconnected";
        slotDisconnect();
    }
}

void MainWindow::slotWifiConnected(QString wifiIP)
{
    m_wifiIp = wifiIP;
    if (m_connectInterfaceWin != nullptr) {
        m_connectInterfaceWin->setInterFaceCodeInfo(wifiIP);
        slotContinueSearch();
    }
    if (m_homepage != nullptr) {
        m_homepage->setDeviceCode(GenerateTools::getIptoCode(m_wifiIp));
    }
}

void MainWindow::slotWifiDisConnected()
{
    m_wifiIp.clear();
    if (m_connectInterfaceWin != nullptr) {
        m_connectInterfaceWin->setInterFaceCodeInfo(m_wifiIp);
    }
    if (m_homepage != nullptr) {
        m_homepage->setDeviceCode(GenerateTools::getIptoCode(m_wifiIp));
    }
    if (m_connectInfo.connectType == ConnectionService::ConnectType::WIFI) {
        slotDisconnect();
    }
}

void MainWindow::slotConnectRequest(QString deviceName)
{
    deleterDialog();
    QString str1 = QString(tr("Connection request received from\"")) + deviceName + QString(tr("\""));
    QString str2 = QString(tr("After consent, the other party can view and download all the files on the device, "
                              "and can share the other party's desktop to this screen."));
    m_messageBox = new MessageDialog(this);
    m_messageBox->setText(str1, str2);
    m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
    QPushButton *yesBtn = m_messageBox->addButton(QString(tr("YES")));
    QPushButton *noBtn = m_messageBox->addButton(QString(tr("NO")));
    connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *btn) {
        if (yesBtn == btn) {
            m_connectionService->setConnectionRespond(true);
        } else {
            m_connectionService->setConnectionRespond(false);
        }
    });
    moveMessageBox();
}

void MainWindow::slotConnectDenied()
{
    deleterDialog();
    QString str1 = QString(tr("The other party has rejected your connection request!"));
    QString str2 = QString(tr("Connection failed. Please contact the other party and try again."));
    m_messageBox = new MessageDialog(this);
    m_messageBox->setText(str1, str2);
    m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
    QPushButton *reconnectBtn = m_messageBox->addButton(QString(tr("RECONNECT")));
    QPushButton *closeBtn = m_messageBox->addButton(QString(tr("CLOSE")));
    connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *btn) {
        if (reconnectBtn == btn) {
            m_connectionService->startClient(m_reconnectAddress);
            startTimer();
        }
    });
    moveMessageBox();
}

void MainWindow::slotConnect(const ConnectionService::ConnectionInfo &connectInfo)
{
    m_connectInfo = connectInfo;
    m_url.setUserName(m_connectInfo.usernamePwd.username);
    m_url.setPassword(m_connectInfo.usernamePwd.pwd);
    m_connectInfo.deviceName.remove(" ");
    connectUI();
    m_ftpClient = new FtpManager(this);
    connect(m_ftpClient, &FtpManager::sigFtpServerConnected, this, &MainWindow::slotFtpConnected);
    connect(m_ftpClient, &FtpManager::sigFtpConnectFailed, this, &MainWindow::slotConnectFailed);
    connect(m_ftpClient, &FtpManager::sigUpdateTransferProgress, this, &MainWindow::slotTransferProgress);
    connect(m_ftpClient, &FtpManager::sigCurrentDirectoryList, this, &MainWindow::slotCurrentDirectoryList);
    connect(m_ftpClient, SIGNAL(sigDownFileFail(QString)), this, SLOT(slotDownFileFail(QString)));
    connect(m_ftpClient, &FtpManager::sigFtpReconnected, this, [=]() { deleterDialog(); });
    connect(m_ftpClient, &FtpManager::sigDownFileFinish, this, &MainWindow::slotDownTempFileFinish);
    connect(m_ftpClient, &FtpManager::sigRejectTran, this, &MainWindow::slotRejectTran);
    m_ftpClient->setTransferMode(QFtp::Passive);

    if (m_connectInfo.deviceType == ConnectionService::DeviceType::PC) {
        // PC连接埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantPcConn);
        m_url.setUrl("ftp://" + m_connectInfo.address + ":" + QString::number(FTP_SERVER_PORT));
    } else {
        // 安卓连接埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantAndroidConn);

        if (m_connectInfo.connectType == ConnectionService::ConnectType::WIFI) {
            // WFIF连接埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantWifiConn);
            m_url.setUrl("ftp://" + m_connectInfo.address + ":" + QString::number(WIFI_FTP_PORT));
        } else {
            // USB连接埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantUsbConn);
            m_url.setUrl("ftp://" + m_connectInfo.address + ":" + QString::number(USB_FTP_PORT));
        }
    }
    m_ftpClient->setUrl(m_url);
    if (m_activityDbusInterface->isValid()) {  // 投屏时pc不锁屏
        QDBusMessage reply = m_activityDbusInterface->call("Inhibit", "kylin-connectivity", (quint32)0,
                                                           "kylin-connectivity media is playing", (quint32)8);
        m_inhibitValue = reply.arguments().at(0).value<uint>();
    }
    m_ftpClient->connectFtpServer();
}

void MainWindow::slotDisconnect()
{
    // 断开连接埋点
    GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantDisconnect);

    if (isHidden()) {
        show();
    }
    Q_EMIT sigDisconnect();
    m_titlebar->setBackground(true);
    m_mainStackedWin->setCurrentIndex(WinIndex::WinHomePage);
    clearView();
    deleteStackedWidget();
    deleteTransmissionDialog();
    deleterDialog();
    slotContinueSearch();
    m_isConnect = false;
    m_downFileType = FileSyncManage::FileType::FileNum;
    m_fileKey = FileSyncManage::FileKey::Default;
    m_url.clear();
    clearConnectionInfo();
    m_isDbusOperation = false;
    m_isPcSearchInfo = false;
    m_searchLoadNum = 0;
    m_reconnectAddress.clear();
    m_backList.clear();
    m_forwardList.clear();
    m_isFileViewPage = true;
    m_lastPathList.clear();
    m_viewMode = QListView::IconMode;
    m_tipMesseage.clear();
    m_isDownloadingThumbanil = false;
    m_uploadPath = "";
    m_isFileViewPage = false;
    m_isOpeningFile = false;
    disconnectService();
}

void MainWindow::slotNotFountApk()
{
    deleterDialog();
    qCritical() << "kylin-assistant not installed!";
    m_messageBox = new MessageDialog(this);
    m_messageBox->setText(tr("Please install kylin-assistant on the Android terminal!"));
    m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
    m_messageBox->addButton(tr("OK"));
    moveMessageBox();
}

void MainWindow::slotNoUsbDevice()
{
    deleterDialog();
    m_messageBox = new MessageDialog(this);
    m_messageBox->setText(tr("Please use the USB to connect your phone device!"));
    m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
    m_messageBox->addButton(tr("OK"));
    moveMessageBox();
}

void MainWindow::slotConnectFailed()
{
    qInfo() << "Failed to connect device!";
    slotDisconnect();
    if (m_isConnect) {
        setToolTipWin(tr("Connection error"), kdk::TipType::Error);
    } else {
        setToolTipWin(tr("Connection timed out"), kdk::TipType::Error);
    }
}

void MainWindow::slotFtpConnected()
{
    bool flag = mountFtp(m_url.toString(), m_url.userName(), m_url.password(), MOUNT_DIR);
    if (!flag) {
        slotMountError(true, "d-bus service fail!");
    }
    m_isMountFtp = true;
    Q_EMIT sigConnected(m_connectInfo.deviceName);
    if (m_connectInfo.deviceType == ConnectionService::DeviceType::PC) {
        initPcSearchInfo();
        slotCdDirectory("/");
    } else if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
        initMobileFileSync();
    }
}

void MainWindow::slotCurrentDirectoryList(const QMap<QString, FileInfo> &map)
{
    if (m_isDbusOperation) {
        // dbus操作
        Q_EMIT sigFileInfoList(map);
        m_isDbusOperation = false;
    } else {
        clearView();
        if (map.isEmpty()) {
            setTipDisplayWin(tr("No Content"));
            m_fileManageWin->setWidget(m_tipDisplayLay, FileManageWin::TabType::Default);
            return;
        }
        m_fileView = new FileView(this);
        connect(m_fileView, &FileView::sigOpenFile, this, &MainWindow::slotOpenFile);
        connect(m_fileView, &FileView::sigDownFileInfo, this, &MainWindow::slotDownFile);
        connect(m_fileView, &FileView::sigUploadFileInfo, this, &MainWindow::slotUploadFile);
        connect(m_fileView, &FileView::sigItemSelectAll, this, &MainWindow::slotFileItemSelecAll);
        m_fileView->setViewInfo(map);
        m_fileView->setViewMode(m_viewMode);
        m_fileView->setModelFlag(m_model);
        m_fileView->setTheme(m_theme);
        m_fileView->setItemMode(FileView::ItemSelectMode::Default, true);
        m_fileView->changeFontSize(m_fontSize);
        m_fileManageWin->setWidget(m_fileView, FileManageWin::TabType::Default);
    }
}

void MainWindow::slotTransferProgress(qint64 readBytes, qint64 totalBytes)
{
    if (!m_isDbusOperation) {
        if (m_transmissionDialog != nullptr) {
            if (totalBytes != 0) {
                int value = (double(readBytes) / double(totalBytes)) * 100;
                m_transmissionDialog->setProgressBarValue(value);
            }
            if (readBytes == totalBytes) {
                deleteTransmissionDialog();
                deleterDialog();
                m_messageBox = new MessageDialog(this);
                m_messageBox->setText(m_tipMesseage);
                m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success"));
                m_messageBox->addButton(tr("OK"));
                moveMessageBox();
                switchUploadPath();
            }
        }
    } else {
        if (readBytes == totalBytes) {
            m_isDbusOperation = false;
        }
    }
}

void MainWindow::slotDownFileFinish(QString flag, QString filepath)
{
    if (!flag.isEmpty()) {
        // flag不为空，一个缩略图下载完成
        if (m_fileView != nullptr) {
            m_fileView->setItemThumbnail(filepath);
        }
    }
}

void MainWindow::slotDownAllFileFinish(QString flag)
{
    if (flag.isEmpty()) {
        // flag为空，信息文件下载完成
        qInfo() << "File information update completed!";
        updateResult();
    } else {
        // 缩略图全部下载完成
        m_isDownloadingThumbanil = false;
        qInfo() << "All thumbnails downloaded.";
    }
}

void MainWindow::slotDownTempFileFinish(QString filePath)
{
    deleterDialog();
    if (m_isPcSearchInfo) {
        m_isPcSearchInfo = false;
        initSearchServer(filePath);
    } else if (!filePath.isEmpty()) {
        FileInfo::fileOpen(filePath);
    } else {
        qInfo() << "Open file error, file path is null!";
    }
}

void MainWindow::slotDownFileFail(QString fileName, QString flag)
{
    Q_UNUSED(fileName);
    deleterDialog();
    if (m_isOpeningFile) {
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(tr("File open exception!"));
        m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
        m_messageBox->addButton(tr("OK"));
        moveMessageBox();
        m_isOpeningFile = false;
        return;
    }
    if (flag.isEmpty()) {
        slotConnectFailed();
    }
}

void MainWindow::slotDownFileFail(QString fileName)
{
    deleterDialog();
    if (m_isPcSearchInfo) {
        m_isPcSearchInfo = false;
    }
    setToolTipWin(tr("file download failed"), kdk::TipType::Error);
}

void MainWindow::slotMountError(bool isMount, QString error)
{
    qInfo() << "mountFtpError:" << error;
}

void MainWindow::slotAboutWinShow()
{
    // kylin-connectivity Version:
    m_aboutWindow = new kdk::KAboutDialog(this, QIcon::fromTheme("kylin-connectivity"),
                                          QString(tr("kylin-connectivity")), tr("Version:") + "1.0.0.1");
    m_aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
    m_aboutWindow->setBodyText(tr("Mobile Assistant is an interconnection tool of Android device and Kirin operating "
                                  "system, which supports Android file synchronization, file transfer, screen "
                                  "mirroring and other functions, which is simple and fast to operate"));
    m_aboutWindow->setBodyTextVisiable(true);
    m_aboutWindow->setWindowModality(Qt::WindowModal);
    m_aboutWindow->setWindowModality(Qt::ApplicationModal);

    m_aboutWindow->show();
    m_aboutWindow->exec();
}

void MainWindow::slotAllDeviceInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map)
{
    m_connectInterfaceWin->showLoad();
    m_connectInterfaceWin->setItemInfo(map);
    m_connectInterfaceWin->changeFontSize(m_fontSize);
}

void MainWindow::slotRequestReceived(PcScreenManage::ScreenMsg msg)
{
    deleterDialog();
    switch (msg) {
        case PcScreenManage::ScreenMsg::SharingRequest: {
            // 投屏请求
            QString str1 = QString(tr("Received screen projection request from \"")) + m_connectInfo.deviceName +
                           QString(tr("\""));
            QString str2 = QString(tr("After consent, the other party can share the device desktop to this screen."));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
            QPushButton *yesBtn = m_messageBox->addButton(QString(tr("YES")));
            QPushButton *noBtn = m_messageBox->addButton(QString(tr("NO")));
            connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *btn) {
                if (yesBtn == btn) {
                    m_pcScreen->setConnectionRespond(true);
                } else {
                    m_pcScreen->setConnectionRespond(false);
                }
            });
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::RemoteRejection: {
            // 请求被拒绝
            QString str1 = QString(tr("The other party has refused your screen projection request!"));
            QString str2 = QString(tr("Failed to cast the screen. Please contact the other party and try again."));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
            QPushButton *againBtn = m_messageBox->addButton(QString(tr("RECONNECT")));
            QPushButton *closeBtn = m_messageBox->addButton(QString(tr("CLOSE")));
            connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *btn) {
                if (againBtn == btn) {
                    // 重新连接
                    m_pcScreen->connectService(m_connectInfo.address);
                } else {
                    m_connectedWin->restoreScreenButton();
                }
            });
            connect(m_messageBox, &MessageDialog::sigClose, this, [=]() { m_connectedWin->restoreScreenButton(); });
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::Successfully: {
            // 请求成功
            QString str1 = QString(tr("The other party agreed to your screen projection request!"));
            QString str2 = QString(tr("The screen is being cast, please wait..."));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success"));
            m_messageBox->addButton(QString(tr("OK")));
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::StartedScreen: {
            // 投屏启动
            if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::INITIATOR) {
                hide();
            }
        } break;
        case PcScreenManage::ScreenMsg::ScreenExit: {
            // 投屏退出
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            show();
            if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::RECIPIENT && m_connectedWin != nullptr) {
                m_connectedWin->restoreScreenButton();
                m_connectedWin->changeFontSize(m_fontSize);
            }
            QString str1 = QString(tr("End of screen projection"));
            QString str2 = QString(tr("The other party has finished the screen projection function."));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
            m_messageBox->addButton(QString(tr("OK")));
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::ScreenError: {
            // 投屏错误
            QString str1 = QString(tr("Screen projection loading error"));
            QString str2 = QString(
                tr("Please check whether to install the projection expansion package [kylin connectivity tools]"));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
            m_messageBox->addButton(QString(tr("CLOSE")));
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::Disconnect: {
            // 投屏断开
            disconnectScreen();
            show();
            QString str1 = QString(tr("End of screen projection"));
            QString str2 = QString(tr("The other party has finished the screen projection function."));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
            m_messageBox->addButton(QString(tr("OK")));
            moveMessageBox();
        } break;
        case PcScreenManage::ScreenMsg::StartedControl: {
            m_suspendTabBar = new SuspendTabBar(SuspendTabBar::Display::Part, true, this);
            connect(m_suspendTabBar, &SuspendTabBar::sigBtnClicked, this, &MainWindow::slotTabBtnClicked);
            m_suspendTabBar->setTheme(m_theme);
            m_suspendTabBar->showTabBar();
        } break;
        case PcScreenManage::ScreenMsg::ExitControl: {
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
        } break;
        case PcScreenManage::ScreenMsg::HideView: {
            if (m_suspendTabBar == nullptr) {
                m_suspendTabBar = new SuspendTabBar(SuspendTabBar::Display::Whole, true, this);
                connect(m_suspendTabBar, &SuspendTabBar::sigBtnClicked, this, &MainWindow::slotTabBtnClicked);
            }
            m_suspendTabBar->setTheme(m_theme);
            m_suspendTabBar->setDeskBtnState(false);
            m_suspendTabBar->showTabBar();
            show();
            activateWindow();
            setFocus();
        } break;
        case PcScreenManage::ScreenMsg::ShowView: {
            hide();
        } break;
        case PcScreenManage::ScreenMsg::Exit: {
            show();
        } break;
        case PcScreenManage::ScreenMsg::PassiveControl: {
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            hide();
        } break;
    }
}

void MainWindow::slotModeChanged(bool isTable)
{
    qInfo() << "mode changed ---- is table:" << isTable;
    if (isTable) {
        m_model = PublicAttributes::Model::Table;
        setMinimumSize(0, 0);
    } else {
        m_model = PublicAttributes::Model::PcModel;
        setMinimumSize(1024, 680);
    }
    if (m_titlebar != nullptr) {
        m_titlebar->changeMode(m_model);
    }
    if (m_fileView != nullptr) {
        m_fileView->setModelFlag(m_model);
    }
}

void MainWindow::slotSearchLoadFail()
{
    deleterDialog();
    if (m_searchLoadNum > SEARCH_LOAD_MAX_NUM) {
        qWarning() << "Search data loading failed, over loads";
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(tr("Search data loading failed!"));
        m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
        m_messageBox->addButton(QString(tr("OK")));
        moveMessageBox();
        return;
    }
    m_searchLoadNum++;
    if (m_fileSyncManage != nullptr) {
        m_fileSyncManage->downloadSearch();
    }
}

void MainWindow::slotRejectTran(FtpManager::ErrorType type)
{
    deleteTransmissionDialog();
    deleterDialog();
    m_messageBox = new MessageDialog(this);
    if (type == FtpManager::ErrorType::Put) {
        m_messageBox->setText(tr("Transmission interruption"),
                              tr("The other party's device has insufficient local storage!"));
    } else {
        m_messageBox->setText(tr("Transmission interruption"), tr("Insufficient local storage space!"));
    }
    m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
    m_messageBox->addButton(tr("OK"));
    connect(m_messageBox, &MessageDialog::buttonClicked, this, [=](QPushButton *) {
        QStringList list = m_fileManageWin->getPathList();
        checkPage(list);
    });
    connect(m_messageBox, &MessageDialog::sigClose, this, [=]() {
        QStringList list = m_fileManageWin->getPathList();
        checkPage(list);
    });
    moveMessageBox();
}

void MainWindow::slotSearchResult(const QMap<QString, FileInfo> &searchResult)
{
    clearView();
    if (searchResult.isEmpty()) {
        setTipDisplayWin(tr("No relevant results"));
        m_tipDisplayLay->setAllowDrag(false);
        m_fileManageWin->setWidget(m_tipDisplayLay, FileManageWin::TabType::SearchResultsPage);
        return;
    }
    m_fileView = new FileView(this);
    connect(m_fileView, &FileView::sigOpenFile, this, &MainWindow::slotOpenFile);
    connect(m_fileView, &FileView::sigDownFileInfo, this, &MainWindow::slotDownFile);
    connect(m_fileView, &FileView::sigItemSelectAll, this, &MainWindow::slotFileItemSelecAll);
    m_fileView->setViewInfo(searchResult);
    m_fileView->setViewMode(m_viewMode);
    m_fileView->setModelFlag(m_model);
    m_fileView->setTheme(m_theme);
    m_fileView->setItemMode(FileView::ItemSelectMode::Default, true);
    m_fileView->changeFontSize(m_fontSize);
    m_fileManageWin->setWidget(m_fileView, FileManageWin::TabType::SearchResultsPage);
    m_fileView->setAllowDrag(false);
}

void MainWindow::slotUSBConnectOnClicked()
{
    if (m_loadDialog != nullptr) {
        return;
    }
    m_connectionService->connectLocDev();
    startLoadUI();
}

void MainWindow::slotCrumbIndexChange(QString text, QStringList &list)
{
    addBackList(list);
    if (text == m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList)) {
        checkAndroidHomePage();
    } else if (text == m_fileManageWin->crumbText(FileManageWin::CrumbTag::QQ)) {
        m_downFileType = FileSyncManage::FileType::FileNum;
        m_fileKey = FileSyncManage::FileKey::QQ;
        m_fileSyncManage->updateFileInfo(m_fileKey, m_downFileType);
    } else if (text == m_fileManageWin->crumbText(FileManageWin::CrumbTag::WeChat)) {
        m_downFileType = FileSyncManage::FileType::FileNum;
        m_fileKey = FileSyncManage::FileKey::WeChat;
        m_fileSyncManage->updateFileInfo(m_fileKey, m_downFileType);
    } else if (text == m_fileManageWin->crumbText(FileManageWin::CrumbTag::Storage)) {
        slotCdDirectory("/");
    } else {
        QStringList pathList = m_fileManageWin->getPathList();
        checkFileView(pathList);
    }
}

void MainWindow::slotFileManageBtnClicked(FileManageWin::BtnType type)
{
    switch (type) {
        case FileManageWin::BtnType::GoBacktBtn: {
            if (m_backList.isEmpty()) {
                qWarning() << "The back-back path list is empty!";
                return;
            }
            QStringList list = m_backList.takeLast();
            if (m_backList.isEmpty()) {
                m_fileManageWin->setGoBackEnabled(false);
            }
            m_forwardList.append(m_fileManageWin->getPathList());
            m_fileManageWin->setGoForwardEnabled(true);
            checkPage(list);
        } break;
        case FileManageWin::BtnType::GoForwardBtn: {
            if (m_forwardList.isEmpty()) {
                qWarning() << "The forward path list is empty!";
                return;
            }
            QStringList list = m_forwardList.takeLast();
            if (m_forwardList.isEmpty()) {
                m_fileManageWin->setGoForwardEnabled(false);
            }
            m_backList.append(m_fileManageWin->getPathList());
            m_fileManageWin->setGoBackEnabled(true);
            checkPage(list);
        } break;
        case FileManageWin::BtnType::ReturnBtn: {
            m_fileManageWin->setGoBackEnabled(false);
            checkPage(m_lastPathList);
            m_lastPathList.clear();
        } break;
        case FileManageWin::BtnType::EditBtn: {
            if (m_fileView != nullptr) {
                m_fileView->setItemMode(FileView::ItemSelectMode::Edit);
            }
        } break;
        case FileManageWin::BtnType::ListModeBtn: {
            // 切换视图埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantSwitchView);
            m_viewMode = QListView::ListMode;
            if (m_fileView != nullptr) {
                m_fileView->setViewMode(m_viewMode);
            }
        } break;
        case FileManageWin::BtnType::IconModeBtn: {
            // 切换视图埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantSwitchView);
            m_viewMode = QListView::IconMode;
            if (m_fileView != nullptr) {
                m_fileView->setViewMode(m_viewMode);
            }
        } break;
        case FileManageWin::BtnType::RefreshBtn: {
            // 刷新列表埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantRefreshList);
            QStringList list = m_fileManageWin->getPathList();
            checkPage(list);
        } break;
        case FileManageWin::BtnType::SelectBtn: {
            if (m_fileView != nullptr) {
                m_fileView->selectAll();
                m_fileView->setDownloadBtn(true);
            }
        } break;
        case FileManageWin::BtnType::CancelSelectBtn: {
            if (m_fileView != nullptr) {
                m_fileView->clearSelection();
                m_fileView->setDownloadBtn(false);
            }
        } break;
        case FileManageWin::BtnType::EditFinishBtn: {
            if (m_fileView != nullptr) {
                m_fileView->setItemMode(FileView::ItemSelectMode::Default, true);
            }
        } break;
    }
}

void MainWindow::slotSearchTextChanged(QString text)
{
    if (m_lastPathList.isEmpty()) {
        m_lastPathList = m_fileManageWin->getPathList();
    }
    if (m_searchServer != nullptr) {
        Q_EMIT m_searchServer->sigSearchFile(text);
    }
}

void MainWindow::slotDownFile(QString path, QList<FileInfo> &fileList)
{
    qInfo() << "choose path" << path << "select file name list size" << fileList.size();
    // 下载文件埋点
    GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantDownloadFile);
    m_ftpClient->downAllFiles(fileList, path);
    m_tipMesseage = tr("Downloaded to") + path;
    initTransmissionDialog();
}

void MainWindow::slotUploadFile(QString dirName, const QStringList &localPathList)
{
    // 上传文件埋点
    GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantUploadFile);
    if (m_isDusUpload) {
        m_isDusUpload = false;
        m_uploadPath = dirName;
        m_fileManageWin->load();
    } else {
        QStringList pathList = m_fileManageWin->getPathList();
        pathList.removeAll(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Storage));
        pathList.removeAll(m_fileManageWin->crumbText(FileManageWin::CrumbTag::FileList));
        pathList.removeAll(m_connectInfo.deviceName);
        m_uploadPath = "/";
        m_uploadPath.append(pathList.join("/"));
        if (!dirName.isEmpty()) {
            if (m_uploadPath == "/") {
                m_uploadPath.append(dirName);
            } else {
                m_uploadPath.append("/" + dirName);
            }
            m_fileManageWin->load();
        }
    }
    m_ftpClient->uploadAllFiles(localPathList, m_uploadPath);
    m_tipMesseage = tr("Uploaded to") + m_uploadPath;
    initTransmissionDialog();
}

bool MainWindow::setScreenOption()
{
    if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::RECIPIENT) {
        // PC投屏埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantPcScreen);
        if (!m_pcScreen->isSupport()) {
            QString str1 = QString(tr("Screen projection loading error"));
            QString str2 = QString(
                tr("Please check whether to install the projection expansion package [kylin connectivity tools]"));
            m_messageBox = new MessageDialog(this);
            m_messageBox->setText(str1, str2);
            m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-error"));
            m_messageBox->addButton(QString(tr("CLOSE")));
            moveMessageBox();
            return false;
        }
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
            m_pcScreen->connectService(m_connectInfo.address, PcScreenManage::DeviceType::Android);
        } else {
            m_pcScreen->connectService(m_connectInfo.address, PcScreenManage::DeviceType::PC);
        }
    } else {
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
            // 手机投屏埋点
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantMobileScreen);
            if (m_deviceManage == nullptr) {
                m_deviceManage = new DeviceManage();
                connect(m_deviceManage, &DeviceManage::sigScreenConnected, this, [=]() { deleterDialog(); });
                connect(m_deviceManage, &DeviceManage::sigSocketDisconnect, this, &MainWindow::slotExitMobileScreen);
            }
            if (m_connectionService->isSupportADB()) {
                BaseDevice::DeviceParams params;
                params.serial = m_connectionService->getDeviceSerial();
                params.maxFps = static_cast<quint32>(Config::getInstance().getMaxFps());
                params.localPort = USBPORT;
                params.screenIndex = getScreenIndex();
                m_deviceManage->connectDevice(params);
            } else {
                BaseDevice::DeviceParams params;
                params.serial = m_url.host();
                params.localPort = WIFIPORT;
                params.screenIndex = getScreenIndex();

                m_deviceManage->connectWifiDevice(params);
            }
        }
    }
    return true;
}

void MainWindow::slotAndroidBtnClicked(AndroidItem::Type type)
{
    m_fileManageWin->load();
    m_isFileViewPage = false;
    QStringList list = m_fileManageWin->getPathList();
    addBackList(list);

    switch (type) {
        case AndroidItem::Type::Picture: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantPictureList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Picture));
            m_downFileType = FileSyncManage::FileType::Picture;
        } break;
        case AndroidItem::Type::Video: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantVideoList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Video));
            m_downFileType = FileSyncManage::FileType::Video;
        } break;
        case AndroidItem::Type::Music: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantAudioList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Music));
            m_downFileType = FileSyncManage::FileType::Music;
        } break;
        case AndroidItem::Type::Doc: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantDocList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Doc));
            m_downFileType = FileSyncManage::FileType::Doc;
        } break;
        case AndroidItem::Type::QQ: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantQQFileList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::QQ));
            m_fileKey = FileSyncManage::FileKey::QQ;
            m_downFileType = FileSyncManage::FileType::FileNum;
        } break;
        case AndroidItem::Type::WeChat: {
            GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantWechatFileList);
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::WeChat));
            m_fileKey = FileSyncManage::FileKey::WeChat;
            m_downFileType = FileSyncManage::FileType::FileNum;
        } break;
        case AndroidItem::Type::File: {
            m_fileManageWin->pushCrumb(m_fileManageWin->crumbText(FileManageWin::CrumbTag::Storage));
            m_isFileViewPage = true;
            slotCdDirectory("/");
        }
            return;
    }
    m_fileSyncManage->updateFileInfo(m_fileKey, m_downFileType);
}

void MainWindow::slotCdDirectory(QString path)
{
    // 手机存储列表埋点
    GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantMobileStorage);

    m_ftpClient->cdToDir(path);
}

void MainWindow::slotOpenFile(FileInfo::FileType type, QString path)
{
    if (type == FileInfo::FileType::Dir) {
        QStringList list = m_fileManageWin->getPathList();
        addBackList(list);
        QString dirName = path.mid(path.lastIndexOf("/") + 1, path.size());
        m_fileManageWin->pushCrumb(dirName);
        m_fileManageWin->load();
        slotCdDirectory(path);
    } else {
        // 打开文件埋点
        GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantOpenFile);
        m_isOpeningFile = true;
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::PC) {
            m_ftpClient->setTempPath(FILE_DOWN_PATH + m_connectInfo.uuid + TEMP_DOWN_PATH);
            m_ftpClient->downFile(path);
        } else {
            m_fileSyncManage->downTempFile(path);
        }
        startLoadUI();
    }
}

void MainWindow::slotFileItemSelecAll(bool isSelectAll)
{
    m_fileManageWin->setEditType(isSelectAll);
}

void MainWindow::slotContinueSearch()
{
    // 设备发现埋点
    GenerateTools::buriedPoint(kabase::BuriedPoint::PT::KylinMobileAssistantDeviceDiscovery);
    m_connectInterfaceWin->showLoad();
    m_discovery->stopDiscovery();
    m_discovery->startDiscovery();
}

void MainWindow::slotAbortTransmission()
{
    m_ftpClient->abort();
    deleteTransmissionDialog();
    startLoadUI();
}

void MainWindow::slotSearchInfoFinish(QString searchInfoPath)
{
    initSearchServer(searchInfoPath);
}

void MainWindow::slotConnectBtnClicked()
{
    m_mainStackedWin->setCurrentIndex(WinIndex::WinConnect);
    m_connectInterfaceWin->setSearchBtnState(true);
}

void MainWindow::slotInitDiscServerResult(bool flag)
{
    if (flag) {
        m_discClient = new QProcess(this);
        connect(m_discClient, &QProcess::started, this, [=]() {
            qInfo() << "softbus init success";
            GenerateTools::delayMs(500);
            m_discovery->startDiscovery();
        });
        connect(m_discClient, &QProcess::readyReadStandardError, this, [=]() {
            QString errOutput = m_discClient->readAllStandardError();
            qWarning() << "softbus client is not exist, softbus init failed." << errOutput;
        });
        m_discClient->start(DISC_CLIENT_PATH);
    } else {
        qWarning() << "softbus server init failed.";
    }
}

void MainWindow::slotReturnHomePage()
{
    m_mainStackedWin->setCurrentIndex(WinIndex::WinHomePage);
}

void MainWindow::slotConnectServiceUI(QString address)
{
    bool ret = m_connectionService->startClient(address);
    if (ret) {
        m_reconnectAddress = address;
        startTimer();
        QString str1 = QString(tr("Request sent successfully!"));
        QString str2 = QString(tr("The connection request has been sent to the selected device. Please click [YES] in "
                                  "the opposite pop-up window"));
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(str1, str2);
        m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success"));
        m_messageBox->addButton(QString(tr("OK")));
        moveMessageBox();
    } else {
        m_messageBox = new MessageDialog(this);
        m_messageBox->setText(tr("There is currently a connection in progress!"));
        m_messageBox->setIconPixmap(QIcon::fromTheme("dialog-warning"));
        m_messageBox->addButton(QString(tr("OK")));
        moveMessageBox();
    }
}

void MainWindow::slotConnectedWinBtnClicked(ConnectedWin::BtnType type)
{
    switch (type) {
        case ConnectedWin::BtnType::Screen: {
            if (setScreenOption()) {
                QString str1 = QString(tr("Request sent successfully!"));
                QString str2 =
                    QString(tr("The screen projection request has been sent to the connected device. Please click "
                               "[Agree] in the opposite pop-up window"));
                m_messageBox = new MessageDialog(this);
                m_messageBox->setText(str1, str2);
                m_messageBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success"));
                m_messageBox->addButton(QString(tr("OK")));
                moveMessageBox();
            }
        } break;
        case ConnectedWin::BtnType::ExitScreen: {
            if (m_connectInfo.deviceRole == ConnectionService::DeviceRole::RECIPIENT) {
                slotExitPcScreen();
            } else {
                if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
                    slotExitMobileScreen();
                } else {
                    slotExitPcScreen();
                }
            }
        } break;
        case ConnectedWin::BtnType::Disconnect: {
            slotDisconnect();
        } break;
    }
}

void MainWindow::slotExitMobileScreen()
{
    if (m_connectedWin != nullptr) {
        m_connectedWin->restoreScreenButton();
        m_connectedWin->changeFontSize(m_fontSize);
    }
    if (m_connectionService != nullptr) {
        m_connectionService->closeAbility();
    }
    if (m_deviceManage != nullptr) {
        m_deviceManage->deleteLater();
        m_deviceManage = nullptr;
    }
}

void MainWindow::slotExitPcScreen()
{
    if (m_connectedWin != nullptr) {
        m_connectedWin->restoreScreenButton();
        m_connectedWin->changeFontSize(m_fontSize);
    }
    if (m_pcScreen != nullptr) {
        m_pcScreen->disconnected();
    }
}

void MainWindow::slotTabBtnClicked(SuspendTabBar::BtnType type)
{
    switch (type) {
        case SuspendTabBar::BtnType::ControlBtn: {
            // 开始反控
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            hide();
            m_pcScreen->startedControl();
        } break;
        case SuspendTabBar::BtnType::CloseControlBtn: {
            // 退出反控
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            m_pcScreen->exitControl();
        } break;
        case SuspendTabBar::BtnType::ScreenShareBtn: {
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            hide();
            m_pcScreen->showView();
        } break;
        case SuspendTabBar::BtnType::ExitBtn: {
            if (m_suspendTabBar != nullptr) {
                m_suspendTabBar->hideTabBar();
                delete m_suspendTabBar;
                m_suspendTabBar = nullptr;
            }
            m_connectedWin->restoreScreenButton();
            m_connectedWin->changeFontSize(m_fontSize);
            m_pcScreen->disconnected();
        } break;
    }
}
