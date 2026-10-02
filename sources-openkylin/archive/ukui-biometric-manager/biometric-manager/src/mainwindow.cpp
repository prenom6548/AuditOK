/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "mainwindow.h"
#include <QDBusArgument>
#include <QFile>
#include <QProcessEnvironment>
#include <QScrollBar>
#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QMenu>
#include <QDBusInterface>
#include <QRadioButton>
#include <QSettings>
#include <QGSettings>
#include <unistd.h>
#include <pwd.h>
#include <QToolTip>
#include <KWindowSystem>
#include <QPainterPath>
#include <QPainter>
#include "contentpane.h"
#include "customtype.h"
#include "messagedialog.h"
#include "configuration.h"
#include "attributewindow.h"
#include "kalabel.h"
#include "lightlabel.h"
#include "loadingdialog.h"
#include "kswitchbutton.h"
#include "preloadwidget.h"

#define ICON_SIZE 32
#define ICON_TYPE_SCHENA "org.ukui.style"
#define FONT_SIZE "system-font-size"
MainWindow::MainWindow(QString usernameFromCmd, QWidget *parent)
    : KWidget(parent)
    , uiStackedWidget(nullptr)
    , username(usernameFromCmd)
    , verificationStatus(false)
    , loginSwitch(false)
    , saverSwitch(false)
    , systemSwitch(false)
    , dragWindow(false)
    , mWatcher(nullptr)
    , lastDeviceInfo(nullptr)
    , aboutDlg(nullptr)
    , m_uniAuthService(new UniAuthService(this))
{
    // 去除窗管标题栏
    if (!m_pUkuiWindowHelper) {
        m_pUkuiWindowHelper = new UkuiWindowHelper(this);
        m_pUkuiWindowHelper->removeTitleBar();
    }
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    connect(WindowManager::self(), &WindowManager::windowAdded, this, [=](const WindowId &windowId) {
        /*注意：
         * 最新创建的窗体被设置为操作窗体，此demo中每个按钮对应一个接口函数调用，所有接口函数操作的窗口都是该最新创建的窗体
         */
        if (getpid() == WindowManager::getPid(windowId)) {
            m_listWinIds.append(windowId);
        }
    });
    connect(WindowManager::self(), &WindowManager::windowRemoved, this, [=](const WindowId &windowId) {
        if (m_listWinIds.contains(windowId)) {
            m_listWinIds.removeOne(windowId);
        }
    });
    this->setIcon("biometric-manager");
    this->setWidgetName(tr("biometric-manager"));
    this->setLayoutType(HorizontalType);
    setWindowFlags(Qt::WindowCloseButtonHint);
    this->windowButtonBar()->menuButton()->show();
    this->resize(960, 640 + this->windowButtonBar()->height());
    if (!m_preLoadWidget) {
        m_preLoadWidget = new PreloadWidget(this);
    }
}

void MainWindow::delayInit()
{
    if (m_bInited) {
        return;
    }
    prettify();

    initialize();

    if (QGSettings::isSchemaInstalled(ICON_TYPE_SCHENA)) {
        QGSettings *icon_type = new QGSettings(ICON_TYPE_SCHENA);
        connect(icon_type, &QGSettings::changed, this, &MainWindow::onConfigurationChanged);
    }

    m_styleSettings = new QGSettings(ICON_TYPE_SCHENA, "", this);
    m_curFontSize = m_styleSettings->get("system-font-size").toInt();
    m_curStyle = m_styleSettings->get("styleName").toString();
    connect(m_styleSettings, &QGSettings::changed, this, &MainWindow::onConfigurationChanged);
    setCurStyleSheet();
    m_bInited = true;
    initPerwgtSize();
    if (m_preLoadWidget) {
        m_preLoadWidget->hide();
        m_preLoadWidget->deleteLater();
    }
}

MainWindow::~MainWindow()
{
    delete uiStackedWidget;
}

void MainWindow::onConfigurationChanged(QString key)
{
    if (key == "systemFontSize" || key == "systemFont") {
        if (m_styleSettings->get("system-font-size").toFloat() > 0) {
            m_curFontSize = m_styleSettings->get("system-font-size").toFloat();
            initPermissionWidget();
        }
    }
    if (key == "styleName") {
        m_curStyle = m_styleSettings->get("styleName").toString();
        initPermissionWidget();
        setCurStyleSheet();
    }
}

void MainWindow::initPermissionWidget()
{
    QTimer::singleShot(
        50, this, [&, this]() { permissionLabel->setFixedSize(uiStackedWidget->PermissionsWidget->size()); });
}

void MainWindow::setCurStyleSheet()
{
    if (m_curStyle == "ukui-dark" || m_curStyle == "ukui-black") {
        QPixmap noDeviceIcon;
        noDeviceIcon.load(":/images/assets/dark-no-device.svg");
        uiStackedWidget->lblNoDeviceIFace->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIFingerPrint->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIFingerVein->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIIris->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIVoicePrint->setPixmap(noDeviceIcon);
    } else {
        QPixmap noDeviceIcon;
        noDeviceIcon.load(":/images/assets/no-device.svg");
        uiStackedWidget->lblNoDeviceIFace->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIFingerPrint->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIFingerVein->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIIris->setPixmap(noDeviceIcon);
        uiStackedWidget->lblNoDeviceIVoicePrint->setPixmap(noDeviceIcon);
    }
}

void MainWindow::initPerwgtSize()
{
    if (!m_bInited) {
        return;
    }
    QTimer::singleShot(0, this, [&, this]() {
        uiStackedWidget->labelPermissions->setFixedWidth(
            uiStackedWidget->PermissionsWidget->width() - 52 - uiStackedWidget->btnStatus->width());
        uiStackedWidget->labelPermissions_3->setFixedWidth(
            uiStackedWidget->PermissionsWidget->width() - 52 - uiStackedWidget->btnStatus->width());
        permissionLabel->setFixedSize(uiStackedWidget->PermissionsWidget->size());
        systemLabel->setFixedSize(uiStackedWidget->systemWindow->size());
        saverLabel->setFixedSize(uiStackedWidget->saverWindow->size());
        greetLabel->setFixedSize(uiStackedWidget->greeterWindow->size());
    });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!m_bInited) {
        if (m_preLoadWidget) {
            m_preLoadWidget->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
        }
        return;
    }
    initPerwgtSize();
    if (event->oldSize().width() != event->size().width()) {
        update_label_width();
    }
    if (m_dialogLoading && m_dialogLoading->isVisible()) {
        m_dialogLoading->setFixedSize(event->size());
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "labelIcon") {
        if (event->type() == QEvent::ToolTip) {
            DeviceInfo *deviceInfo;
            for (auto info : deviceInfosMap[uiStackedWidget->listWidgetDevicesType->currentIndex()]) {
                if (info->device_shortname == obj->property("devicename").toString()) {
                    if (dialog)
                        return false;
                    deviceInfo = info;
                    dialog = new AttributeWindow(deviceInfo, this);
                    int x = QCursor::pos().x();
                    int y = QCursor::pos().y();
                    dialog->setGeometry(x + 10, y + 10, dialog->width(), dialog->height());
                    dialog->show();
                    m_isShowDialog = true;
                    QTimer::singleShot(100, this, [this]() { m_isShowDialog = false; });
                    break;
                }
            }
        } else if (event->type() == QEvent::Leave) {
            if (dialog == nullptr || m_isShowDialog)
                return false;
            dialog->hide();
            dialog->close();
            dialog = nullptr;
        }
    }
    if (event->type() == QEvent::WindowStateChange) {
        if (obj == this) {
            if (this->windowState() == Qt::WindowMaximized) {
                this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Restore);
            } else {
                this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Maximum);
            }
        }
    }
    return KWidget::eventFilter(obj, event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        if (!daemonIsNotRunning()) {
            showGuide("biometric-manager");
        }
    }
}

int MainWindow::daemonIsNotRunning()
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", service_name);
    return reply.value() == "";
}

void MainWindow::showGuide(QString appName)
{
    qDebug() << Q_FUNC_INFO << appName;

    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());

    QDBusInterface *interface = new QDBusInterface(
        service_name, KYLIN_USER_GUIDE_PATH, KYLIN_USER_GUIDE_INTERFACE, QDBusConnection::sessionBus(), this);

    QDBusMessage msg = interface->call(QStringLiteral("showGuide"), "biometric-manager");
}

void MainWindow::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    dragWindow = false;
}

void MainWindow::prettify()
{
    uiStackedWidget = new Ui::stackedWidget();
    uiStackedWidget->setupUi(this->baseBar());
    uiStackedWidget->stackedWidgetMain->setAutoFillBackground(true);
    uiStackedWidget->stackedWidgetMain->setBackgroundRole(QPalette::Base);

    initLeftsideBar();
    onBtnDashBoardClicked();

    uiStackedWidget->labelBiometric->setText(tr("Biometric"));
    uiStackedWidget->labelBiometric->adjustSize();
    uiStackedWidget->labelDeviceType->setText(tr("Select Device Type"));
    uiStackedWidget->labelDeviceType->adjustSize();
    uiStackedWidget->labelDeviceType->setAlignment(Qt::AlignVCenter);
    uiStackedWidget->labelPermissions->setWordWrap(true);
    uiStackedWidget->labelPermissions->setAlignment(Qt::AlignTop);
    uiStackedWidget->labelPermissions->adjustSize();
    QPalette permisionsPalette = uiStackedWidget->labelPermissions->palette();
    permisionsPalette.setColor(QPalette::WindowText, Qt::gray);
    permisionsPalette.setColor(QPalette::Text, Qt::gray);
    uiStackedWidget->labelPermissions->setPalette(permisionsPalette);
    uiStackedWidget->PermissionsWidget->adjustSize();

    uiStackedWidget->tableWidgetDevices->setAlternatingRowColors(true);
    uiStackedWidget->tableWidgetDevices->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    uiStackedWidget->tableWidgetDevices->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    uiStackedWidget->tableWidgetDevices->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel); // 设置视图将每次滚动一个像素的内容
    QScrollBar *m_scrollBarStep = uiStackedWidget->tableWidgetDevices->verticalScrollBar(); // 获取到tablewidget的滚动条
    m_scrollBarStep->setSingleStep(20); // 设置单步，值越小，下滑越慢

    uiStackedWidget->tableWidgetDevices->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    permissionLabel = new QLabel(uiStackedWidget->PermissionsWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(
        permissionLabel,
        "Biometric Manager",
        "PermissionsWidget",
        "biometric permission settings of permissions widget");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->btnStatus,
        "Biometric Manager",
        "PermissionsWidget",
        "biometric permission settings button of permissions widget");
    permissionLabel->installEventFilter(this);
    permissionLabel->lower();
    systemLabel = new QLabel(uiStackedWidget->systemWindow);
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->btnSystem,
        "Biometric Manager",
        "PermissionsWidget",
        "system permission settings button of permissions widget");
    KDK_EXTEND_ALL_INFO_FORMAT(
        systemLabel, "Biometric Manager", "PermissionsWidget", "system permission settings of permissions widget");
    systemLabel->lower();

    saverLabel = new QLabel(uiStackedWidget->saverWindow);
    KDK_EXTEND_ALL_INFO_FORMAT(
        saverLabel, "Biometric Manager", "PermissionsWidget", "screensaver permission settings of permissions widget");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->btnSaver,
        "Biometric Manager",
        "PermissionsWidget",
        "screensaver permission settings button of permissions widget");
    saverLabel->lower();

    greetLabel = new QLabel(uiStackedWidget->greeterWindow);
    KDK_EXTEND_ALL_INFO_FORMAT(
        greetLabel, "Biometric Manager", "PermissionsWidget", "greeter permission settings of permissions widget");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->btnLogin,
        "Biometric Manager",
        "PermissionsWidget",
        "greeter permission settings button of permissions widget");

    greetLabel->lower();
    uiStackedWidget->biometricLabel->hide();
    uiStackedWidget->greeterWindow->hide();
    uiStackedWidget->saverWindow->hide();
    uiStackedWidget->systemWindow->hide();
    QVBoxLayout *permissionLyt = new QVBoxLayout();
    uiStackedWidget->pagePermissions->setLayout(permissionLyt);
    permissionLyt->addWidget(uiStackedWidget->perWidget);
    uiStackedWidget->labelPermissions->setText(
        tr("After this function is enabled, it can be used for system authentication, including login, screen lock, "
           "system authorization, and Polkit."));

    permissionLabel->setStyleSheet("background: palette(base); border-radius: 8px; border: 1px solid; "
                                   "border-color:palette(button); border-style:solid;");
    systemLabel->setStyleSheet("background: palette(base); border-bottom-left-radius: 8px;border-bottom-right-radius: "
                               "8px;border: 1px; border-color:palette(button); border-style:solid;");
    saverLabel->setStyleSheet("background: palette(base); border-left-width: 1px; border-right-width: 1px; "
                              "border-color:palette(button); border-style:solid;");
    greetLabel->setStyleSheet("background: palette(base); border-top-left-radius: 8px; border-top-right-radius: 8px; "
                              "border: 1px solid; border-color:palette(button); border-style:solid;");

    uiStackedWidget->biometricLabel->hide();
    uiStackedWidget->greeterWindow->hide();
    uiStackedWidget->saverWindow->hide();
    uiStackedWidget->systemWindow->hide();
    initPermissionWidget();

    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->listWidgetDevicesType, "Biometric Manager", "uiStackedWidget", "Biometric type dropdown");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->lblNoDevicePFingerPrint,
        "Biometric Manager",
        "uiStackedWidget",
        "Fingerprint navigation button");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->lblNoDeviceFace, "Biometric Manager", "uiStackedWidget", "Face Recognition Redirect Button");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->lblNoDeviceFingerVein, "Biometric Manager", "uiStackedWidget", "Finger vein toggle button");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->lblNoDeviceVoicePrint,
        "Biometric Manager",
        "uiStackedWidget",
        "Voiceprint recognition jump button");
    KDK_EXTEND_ALL_INFO_FORMAT(
        uiStackedWidget->lblNoDeviceIris, "Biometric Manager", "uiStackedWidget", "Iris jump button");

    // 监听平板模式切换
    m_statusSessionDbus = new QDBusInterface(
        "com.kylin.statusmanager.interface",
        "/",
        "com.kylin.statusmanager.interface",
        QDBusConnection::sessionBus(),
        this);
    tabletStatusChanged(is_tabletmode);
    if (m_statusSessionDbus->isValid()) {
        is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
        tabletStatusChanged(is_tabletmode);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletStatusChanged(bool)));
    } else {
        qDebug() << "Create statusmanager Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }
}

void MainWindow::initLeftsideBar()
{
    QHBoxLayout *siderLayout = new QHBoxLayout();
    siderLayout->setSpacing(0);
    siderLayout->setContentsMargins(0, 0, 0, 0);
    scrollArea = new KNavigationBar(this->sideBar());
    scrollArea->setMinimumHeight(640);

    scrollArea->addTag(tr("Device&Permission"));
    permissionItem = new QStandardItem(QIcon(":/images/assets/Permissions.svg"), tr("Permissions"));
    scrollArea->addItem(permissionItem);

    dashBoardItem = new QStandardItem(QIcon(":/images/assets/Device.svg"), tr("Biometric"));
    scrollArea->addItem(dashBoardItem);

    scrollArea->addTag(tr("Password Setting"));
    fingerPrintItem = new QStandardItem(QIcon(":/images/assets/FingerPrint.svg"), tr("FingerPrint"));
    scrollArea->addItem(fingerPrintItem);
    faceItem = new QStandardItem(QIcon(":/images/assets/faceid.svg"), tr("Face"));
    scrollArea->addItem(faceItem);
    fingerVeinItem = new QStandardItem(QIcon(":/images/assets/FingerVein.svg"), tr("FingerVein"));
    scrollArea->addItem(fingerVeinItem);
    irisItem = new QStandardItem(QIcon(":/images/assets/Iris.svg"), tr("Iris"));
    scrollArea->addItem(irisItem);
    voiceprintItem = new QStandardItem(QIcon(":/images/assets/VoicePrint.svg"), tr("VoicePrint"));
    scrollArea->addItem(voiceprintItem);
    connect(scrollArea->listview(), &QListView::clicked, this, [=](const QModelIndex &index) {
        switch (index.row()) {
            case 1:
                onBtnPermissionsClicked();
                break;
            case 2:
                onBtnDashBoardClicked();
                break;
            case 4:
                onBtnFingerPrintClicked();
                break;
            case 5:
                onBtnFaceClicked();
                break;
            case 6:
                onBtnFingerVeinClicked();
                break;
            case 7:
                onBtnIrisClicked();
                break;
            case 8:
                onBtnVoicePrintClicked();
                break;
            default:
                break;
        }
    });
    scrollArea->listview()->setFixedWidth(184);
    siderLayout->addWidget(scrollArea);
    this->sideBar()->setLayout(siderLayout);
}

QPixmap *MainWindow::getUserAvatar(QString username)
{
    QString iconPath;
    QDBusInterface userIface(
        "org.freedesktop.Accounts",
        "/org/freedesktop/Accounts",
        "org.freedesktop.Accounts",
        QDBusConnection::systemBus());
    if (!userIface.isValid())
        qDebug() << "GUI:"
                 << "userIface is invalid";
    QDBusReply<QDBusObjectPath> userReply = userIface.call("FindUserByName", username);
    if (!userReply.isValid()) {
        qDebug() << "GUI:"
                 << "userReply is invalid";
        iconPath = "/usr/share/kylin-greeter/default_face.png";
    }
    QDBusInterface iconIface(
        "org.freedesktop.Accounts",
        userReply.value().path(),
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());
    if (!iconIface.isValid())
        qDebug() << "GUI:"
                 << "IconIface is invalid";
    QDBusReply<QDBusVariant> iconReply = iconIface.call("Get", "org.freedesktop.Accounts.User", "IconFile");
    if (!iconReply.isValid()) {
        qDebug() << "GUI:"
                 << "iconReply is invalid";
        iconPath = "/usr/share/kylin-greeter/default_face.png";
    }
    iconPath = iconReply.value().variant().toString();
    if (access(qPrintable(iconPath), R_OK) != 0) /* No Access Permission */
        iconPath = "/usr/share/kylin-greeter/default_face.png";
    return new QPixmap(iconPath);
}

void MainWindow::setCurrentUser() {}

void MainWindow::onReviceWindowMessage(QString message)
{
    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        if (!m_listWinIds.isEmpty()) {
            WindowManager::activateWindow(m_listWinIds.back());
        }
    } else {
        KWindowSystem::forceActiveWindow(this->winId());
    }
    this->show();
}

void MainWindow::initialize()
{
    /* 向 QDBus 类型系统注册自定义数据类型 */
    registerCustomTypes();
    /* 连接 DBus Daemon */
    serviceInterface = new QDBusInterface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647); /* 微秒 */

    initSysMenu();

    /* 获取并显示用户 */
    setCurrentUser();

    /* 获取设备列表 */
    getDeviceInfo();

    /* Other initializations */
    initDashboardBioAuthSection();
    initBiometricPage();
    initDeviceTypeList();

    uiStackedWidget->btnStatus->setCheckable(true);
    connect(uiStackedWidget->btnStatus, &KSwitchButton::clicked, this, &MainWindow::on_btnStatus_clicked);
    connect(uiStackedWidget->btnSaver, &KSwitchButton::clicked, this, &MainWindow::on_btnSaver_clicked);
    connect(uiStackedWidget->btnLogin, &KSwitchButton::clicked, this, &MainWindow::on_btnLogin_clicked);
    connect(uiStackedWidget->btnSystem, &KSwitchButton::clicked, this, &MainWindow::on_btnSystem_clicked);
    connect(
        uiStackedWidget->lblNoDevicePFingerPrint,
        &KSwitchButton::clicked,
        this,
        &MainWindow::on_lblNoDevicePFingerPrint_clicked);
    connect(uiStackedWidget->lblNoDevicePFace, &KSwitchButton::clicked, this, &MainWindow::on_lblNoDevicePFace_clicked);
    connect(
        uiStackedWidget->lblNoDevicePFingerVein,
        &KSwitchButton::clicked,
        this,
        &MainWindow::on_lblNoDevicePFingerVein_clicked);
    connect(uiStackedWidget->lblNoDevicePIris, &KSwitchButton::clicked, this, &MainWindow::on_lblNoDevicePIris_clicked);
    connect(
        uiStackedWidget->lblNoDevicePVoicePrint,
        &KSwitchButton::clicked,
        this,
        &MainWindow::on_lblNoDevicePVoicePrint_clicked);
    connect(
        uiStackedWidget->listWidgetDevicesType,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(on_listWidgetDevicesType_currentIndexChanged(int)));
    on_listWidgetDevicesType_currentIndexChanged(uiStackedWidget->listWidgetDevicesType->currentIndex());

    connect(serviceInterface, SIGNAL(USBDeviceHotPlug(int, int, int)), this, SLOT(onUSBDeviceHotPlug(int, int, int)));
    connect(serviceInterface, SIGNAL(FeatureChanged(int, int, int)), this, SLOT(onUpdateDevice(int, int, int)));
}

void MainWindow::initSysMenu()
{
    menu = new QMenu(this);
    QAction *serviceStatusAction = new QAction(tr("Restart Service"), this);
    connect(serviceStatusAction, &QAction::triggered, this, [&] { restartService(); });

    QAction *aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, [&] {
        if (aboutDlg == nullptr)
            aboutDlg = new KAboutDialog(this, QIcon::fromTheme("biometric-manager"), tr("Biometric Manager"));
        aboutDlg->setWindowTitle(tr("Biometric Manager"));
        aboutDlg->setAppVersion(Configuration::instance()->get_current_version());
        aboutDlg->setAppSupport(aboutDlg->appSupport());
        aboutDlg->exec();
    });

    QAction *exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, &QAction::triggered, this, [&] { close(); });

    QAction *helpAction = new QAction(tr("Help"), this);
    connect(helpAction, &QAction::triggered, this, [&] {
        if (!daemonIsNotRunning()) {
            showGuide("biometric-manager");
        }
    });

    menu->addActions({ serviceStatusAction,
                       helpAction,
                       aboutAction,
                       this->windowButtonBar()->menuButton()->feedbackAction(),
                       exitAction });
    this->windowButtonBar()->menuButton()->setPopupMode(QToolButton::InstantPopup);
    this->windowButtonBar()->menuButton()->setMenu(menu);
    this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Maximum);
}

void MainWindow::onBtnDashBoardClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    scrollArea->listview()->setCurrentIndex(dashBoardItem->index());
}

void MainWindow::onBtnPermissionsClicked()
{
    setStackedWidgetMain(uiStackedWidget->pagePermissions);
    scrollArea->listview()->setCurrentIndex(permissionItem->index());
    initPermissionWidget();
    initPerwgtSize();
}

void MainWindow::onBtnFingerPrintClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageFingerPrint);
    scrollArea->listview()->setCurrentIndex(fingerPrintItem->index());
}

void MainWindow::onBtnFingerVeinClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageFingerVein);
    scrollArea->listview()->setCurrentIndex(fingerVeinItem->index());
}

void MainWindow::onBtnIrisClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageIris);
    scrollArea->listview()->setCurrentIndex(irisItem->index());
}

void MainWindow::onBtnVoicePrintClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageVoicePrint);
    scrollArea->listview()->setCurrentIndex(voiceprintItem->index());
}

void MainWindow::onBtnFaceClicked()
{
    setStackedWidgetMain(uiStackedWidget->pageFace);
    scrollArea->listview()->setCurrentIndex(faceItem->index());
}

void MainWindow::on_lblNoDevicePFingerPrint_clicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(0);
}

void MainWindow::on_lblNoDevicePFace_clicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(1);
}

void MainWindow::on_lblNoDevicePFingerVein_clicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(2);
}

void MainWindow::on_lblNoDevicePIris_clicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(3);
}

void MainWindow::on_lblNoDevicePVoicePrint_clicked()
{
    setStackedWidgetMain(uiStackedWidget->pageDashBoard);
    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(4);
}

/**
 * @brief 设备类型到索引的映射
 */
int MainWindow::bioTypeToIndex(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return 0;
        case BIOTYPE_FACE:
            return 1;
        case BIOTYPE_FINGERVEIN:
            return 2;
        case BIOTYPE_IRIS:
            return 3;
        case BIOTYPE_VOICEPRINT:
            return 4;
    }
    return -1;
}

void MainWindow::updateDeviceNum(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            fpNum += 1;
            break;
        case BIOTYPE_FACE:
            fcNum += 1;
            break;
        case BIOTYPE_FINGERVEIN:
            fvNum += 1;
            break;
        case BIOTYPE_IRIS:
            irNum += 1;
            break;
        case BIOTYPE_VOICEPRINT:
            vpNum += 1;
            break;
        default:
            break;
    }
    return;
}

int MainWindow::getDeviceNum(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return fpNum;
        case BIOTYPE_FACE:
            return fcNum;
        case BIOTYPE_FINGERVEIN:
            return fvNum;
        case BIOTYPE_IRIS:
            return irNum;
        case BIOTYPE_VOICEPRINT:
            return vpNum;
    }
    return 0;
}

void MainWindow::clearDeviceNum()
{
    fpNum = 0;
    fcNum = 0;
    fvNum = 0;
    irNum = 0;
    vpNum = 0;
}

/**
 * @brief 获取设备列表并存储起来备用
 */
void MainWindow::getDeviceInfo()
{
    QVariant variant;
    QDBusArgument argument;
    QList<QDBusVariant> qlist;
    QDBusVariant item;
    DeviceInfo *deviceInfo;

    /* 返回值为 i -- int 和 av -- array of variant */
    QDBusPendingReply<int, QList<QDBusVariant>> reply = serviceInterface->call("GetDrvList");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "GUI:" << reply.error();
        deviceCount = 0;
        return;
    }

    /* 解析 DBus 返回值，reply 有两个返回值，都是 QVariant 类型 */
    variant = reply.argumentAt(0);             /* 得到第一个返回值 */
    deviceCount = variant.value<int>();        /* 解封装得到设备个数 */
    variant = reply.argumentAt(1);             /* 得到第二个返回值 */
    argument = variant.value<QDBusArgument>(); /* 解封装，获取QDBusArgument对象 */
    argument >> qlist; /* 使用运算符重载提取 argument 对象里面存储的列表对象 */

    for (int i = 0; i < __MAX_NR_BIOTYPES; i++)
        deviceInfosMap[i].clear();
    ;

    for (int i = 0; i < deviceCount; i++) {
        item = qlist[i];          /* 取出一个元素 */
        variant = item.variant(); /* 转为普通QVariant对象 */
        /* 解封装得到 QDBusArgument 对象 */
        argument = variant.value<QDBusArgument>();
        deviceInfo = new DeviceInfo();
        argument >> *deviceInfo; /* 提取最终的 DeviceInfo 结构体 */

        deviceInfosMap[bioTypeToIndex(deviceInfo->biotype)].append(deviceInfo);
        if (deviceInfo->device_available > 0)
            updateDeviceNum(deviceInfo->biotype);
    }
}

void MainWindow::setLastDeviceSelected()
{
    if (!lastDeviceInfo)
        return;

    QComboBox *lw;
    QStackedWidget *sw;

    switch (lastDeviceInfo->biotype) {
        case BIOTYPE_FINGERPRINT:
            lw = uiStackedWidget->listWidgetFingerPrint;
            sw = uiStackedWidget->stackedWidgetFingerPrint;
            break;
        case BIOTYPE_FINGERVEIN:
            lw = uiStackedWidget->listWidgetFingerVein;
            sw = uiStackedWidget->stackedWidgetFingerVein;
            break;
        case BIOTYPE_IRIS:
            lw = uiStackedWidget->listWidgetIris;
            sw = uiStackedWidget->stackedWidgetIris;
            break;
        case BIOTYPE_FACE:
            lw = uiStackedWidget->listWidgetFace;
            sw = uiStackedWidget->stackedWidgetFace;
            break;
        case BIOTYPE_VOICEPRINT:
            lw = uiStackedWidget->listWidgetVoicePrint;
            sw = uiStackedWidget->stackedWidgetVoicePrint;
            break;
    }

    lastDeviceInfo = nullptr;

    for (int i = 0; i < lw->count(); i++) {
        if (lw->currentText() == lastDeviceInfo->device_shortname) {
            lw->setCurrentIndex(i);
            sw->setCurrentIndex(i);
        }
    }

    lastDeviceInfo = nullptr;
}

void MainWindow::raiseContentPane(DeviceInfo *deviceInfo)
{
    if (deviceInfo->device_available <= 0)
        return;

    QComboBox *lw;
    QStackedWidget *sw;
    switch (deviceInfo->biotype) {
        case BIOTYPE_FINGERPRINT:
            lw = uiStackedWidget->listWidgetFingerPrint;
            sw = uiStackedWidget->stackedWidgetFingerPrint;
            break;
        case BIOTYPE_FINGERVEIN:
            lw = uiStackedWidget->listWidgetFingerVein;
            sw = uiStackedWidget->stackedWidgetFingerVein;
            break;
        case BIOTYPE_IRIS:
            lw = uiStackedWidget->listWidgetIris;
            sw = uiStackedWidget->stackedWidgetIris;
            break;
        case BIOTYPE_FACE:
            lw = uiStackedWidget->listWidgetFace;
            sw = uiStackedWidget->stackedWidgetFace;
            break;
        case BIOTYPE_VOICEPRINT:
            lw = uiStackedWidget->listWidgetVoicePrint;
            sw = uiStackedWidget->stackedWidgetVoicePrint;
            break;
    }
    for (int i = 0; i < lw->count(); i++) {
        if (lw->itemText(i) == deviceInfo->device_shortname) {
            QWidget *widget = sw->widget(i);
            sw->removeWidget(sw->widget(i));
            sw->insertWidget(0, widget);
        }
    }
}

void MainWindow::addContentPane(DeviceInfo *deviceInfo)
{
    QComboBox *lw;
    QStackedWidget *sw;

    switch (deviceInfo->biotype) {
        case BIOTYPE_FINGERPRINT:
            lw = uiStackedWidget->listWidgetFingerPrint;
            sw = uiStackedWidget->stackedWidgetFingerPrint;
            break;
        case BIOTYPE_FINGERVEIN:
            lw = uiStackedWidget->listWidgetFingerVein;
            sw = uiStackedWidget->stackedWidgetFingerVein;
            break;
        case BIOTYPE_IRIS:
            lw = uiStackedWidget->listWidgetIris;
            sw = uiStackedWidget->stackedWidgetIris;
            break;
        case BIOTYPE_FACE:
            lw = uiStackedWidget->listWidgetFace;
            sw = uiStackedWidget->stackedWidgetFace;
            break;
        case BIOTYPE_VOICEPRINT:
            lw = uiStackedWidget->listWidgetVoicePrint;
            sw = uiStackedWidget->stackedWidgetVoicePrint;
            break;
    }

    ContentPane *contentPane = new ContentPane(getuid(), deviceInfo);
    connect(contentPane, &ContentPane::deleteFeatureState, this, &MainWindow::onDeleteFeatureState);
    QString default_Name = m_uniAuthService->getDefaultDevice(getenv("USER"), deviceInfo->biotype);
    QString defaultText = tr("(default) ");
    defaultText = defaultText + deviceInfo->device_shortname;
    if (deviceInfo->device_available > 0) {
        if (deviceInfo->device_shortname == default_Name)
            lw->addItem(defaultText);
        else
            lw->addItem(deviceInfo->device_shortname);
        sw->insertWidget(sw->count(), contentPane);
    }

    contentPaneMap.insert(deviceInfo->device_shortname, contentPane);

    connect(
        lw,
        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        sw,
        &QStackedWidget::setCurrentIndex);
}

#define checkBiometricPage(biometric)                                                                                  \
    do {                                                                                                               \
        if (uiStackedWidget->listWidget##biometric->count() >= 1) {                                                    \
            uiStackedWidget->listWidget##biometric->setCurrentIndex(0);                                                \
            uiStackedWidget->listWidget##biometric->show();                                                            \
            uiStackedWidget->listWidgetA##biometric->show();                                                           \
            uiStackedWidget->listWidgetB##biometric->show();                                                           \
            uiStackedWidget->stackedWidget##biometric->show();                                                         \
            uiStackedWidget->lblNoDeviceW##biometric->hide();                                                          \
        } else {                                                                                                       \
            uiStackedWidget->listWidget##biometric->hide();                                                            \
            uiStackedWidget->listWidgetA##biometric->hide();                                                           \
            uiStackedWidget->listWidgetB##biometric->hide();                                                           \
            uiStackedWidget->stackedWidget##biometric->hide();                                                         \
            uiStackedWidget->lblNoDeviceW##biometric->show();                                                          \
        }                                                                                                              \
    } while (0)

void MainWindow::updateBiometricPage(int type, int devNumNow)
{
    if (type == 0) {
        if (devNumNow == 0) {
            uiStackedWidget->listWidgetFingerPrint->hide();
            uiStackedWidget->stackedWidgetFingerPrint->hide();
            uiStackedWidget->listWidgetAFingerPrint->hide();
            uiStackedWidget->listWidgetBFingerPrint->hide();
            uiStackedWidget->lblNoDeviceWFingerPrint->show();
        } else {
            uiStackedWidget->listWidgetFingerPrint->show();
            uiStackedWidget->stackedWidgetFingerPrint->show();
            uiStackedWidget->listWidgetAFingerPrint->show();
            uiStackedWidget->listWidgetBFingerPrint->show();
            uiStackedWidget->lblNoDeviceWFingerPrint->hide();
        }
    }
    if (type == 1) {
        if (devNumNow == 0) {
            uiStackedWidget->listWidgetFace->hide();
            uiStackedWidget->stackedWidgetFace->hide();
            uiStackedWidget->listWidgetAFace->hide();
            uiStackedWidget->listWidgetBFace->hide();
            uiStackedWidget->lblNoDeviceWFace->show();
        } else {
            uiStackedWidget->listWidgetFace->show();
            uiStackedWidget->stackedWidgetFace->show();
            uiStackedWidget->listWidgetAFace->show();
            uiStackedWidget->listWidgetBFace->show();
            uiStackedWidget->lblNoDeviceWFace->hide();
        }
    }
    if (type == 2) {
        if (devNumNow == 0) {
            uiStackedWidget->listWidgetFingerVein->hide();
            uiStackedWidget->stackedWidgetFingerVein->hide();
            uiStackedWidget->listWidgetAFingerVein->hide();
            uiStackedWidget->listWidgetBFingerVein->hide();
            uiStackedWidget->lblNoDeviceWFingerVein->show();
        } else {
            uiStackedWidget->listWidgetFingerVein->show();
            uiStackedWidget->stackedWidgetFingerVein->show();
            uiStackedWidget->listWidgetAFingerVein->show();
            uiStackedWidget->listWidgetBFingerVein->show();
            uiStackedWidget->lblNoDeviceWFingerVein->hide();
        }
    }
    if (type == 3) {
        if (devNumNow == 0) {
            uiStackedWidget->listWidgetIris->hide();
            uiStackedWidget->stackedWidgetIris->hide();
            uiStackedWidget->listWidgetAIris->hide();
            uiStackedWidget->listWidgetBIris->hide();
            uiStackedWidget->lblNoDeviceWIris->show();
        } else {
            uiStackedWidget->listWidgetIris->show();
            uiStackedWidget->stackedWidgetIris->show();
            uiStackedWidget->listWidgetAIris->show();
            uiStackedWidget->listWidgetBIris->show();
            uiStackedWidget->lblNoDeviceWIris->hide();
        }
    }
    if (type == 4) {
        if (devNumNow == 0) {
            uiStackedWidget->listWidgetVoicePrint->hide();
            uiStackedWidget->stackedWidgetVoicePrint->hide();
            uiStackedWidget->listWidgetAVoicePrint->hide();
            uiStackedWidget->listWidgetBVoicePrint->hide();
            uiStackedWidget->lblNoDeviceWVoicePrint->show();
        } else {
            uiStackedWidget->listWidgetVoicePrint->show();
            uiStackedWidget->stackedWidgetVoicePrint->show();
            uiStackedWidget->listWidgetAVoicePrint->show();
            uiStackedWidget->listWidgetBVoicePrint->show();
            uiStackedWidget->lblNoDeviceWVoicePrint->hide();
        }
    }
}

void MainWindow::initBiometricPage()
{
    uiStackedWidget->listWidgetFingerPrint->clear();
    for (int i = uiStackedWidget->stackedWidgetFingerPrint->count(); i >= 0; i--) {
        QWidget *widget = uiStackedWidget->stackedWidgetFingerPrint->widget(i);
        uiStackedWidget->stackedWidgetFingerPrint->removeWidget(widget);
        if (widget) {
            widget->close();
            widget->deleteLater();
        }
    }
    uiStackedWidget->listWidgetFingerVein->clear();
    for (int i = uiStackedWidget->stackedWidgetFingerVein->count(); i >= 0; i--) {
        QWidget *widget = uiStackedWidget->stackedWidgetFingerVein->widget(i);
        uiStackedWidget->stackedWidgetFingerVein->removeWidget(widget);
        if (widget) {
            widget->close();
            widget->deleteLater();
        }
    }
    uiStackedWidget->listWidgetIris->clear();
    for (int i = uiStackedWidget->stackedWidgetIris->count(); i >= 0; i--) {
        QWidget *widget = uiStackedWidget->stackedWidgetIris->widget(i);
        uiStackedWidget->stackedWidgetIris->removeWidget(widget);
        if (widget) {
            widget->close();
            widget->deleteLater();
        }
    }
    uiStackedWidget->listWidgetVoicePrint->clear();
    for (int i = uiStackedWidget->stackedWidgetVoicePrint->count(); i >= 0; i--) {
        QWidget *widget = uiStackedWidget->stackedWidgetVoicePrint->widget(i);
        uiStackedWidget->stackedWidgetVoicePrint->removeWidget(widget);
        if (widget) {
            widget->close();
            widget->deleteLater();
        }
    }
    uiStackedWidget->listWidgetFace->clear();
    for (int i = uiStackedWidget->stackedWidgetFace->count(); i >= 0; i--) {
        QWidget *widget = uiStackedWidget->stackedWidgetFace->widget(i);
        uiStackedWidget->stackedWidgetFace->removeWidget(widget);
        if (widget) {
            widget->close();
            widget->deleteLater();
        }
    }
    contentPaneMap.clear();

    for (int i = 0; i < __MAX_NR_BIOTYPES; i++) {
        for (auto deviceInfo : deviceInfosMap[i]) {
            addContentPane(deviceInfo);
        }
    }
    checkBiometricPage(FingerPrint);
    checkBiometricPage(FingerVein);
    checkBiometricPage(Iris);
    checkBiometricPage(VoicePrint);
    checkBiometricPage(Face);
}

void MainWindow::onDeleteFeatureState(int nState)
{
    switch (nState) {
        case 0: // 开始删除
            if (!m_isShowDeleteAnimation) {
                m_isShowDeleteAnimation = true;
                if (!m_dialogLoading) {
                    m_dialogLoading = new LoadingDialog(this, 0.4);
                } else {
                    m_dialogLoading->setLoading();
                }
                m_dialogLoading->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
                m_dialogLoading->show();
            }
            break;
        case 1: // 删除完成
            if (m_isShowDeleteAnimation) {
                m_isShowDeleteAnimation = false;
                if (m_dialogLoading) {
                    m_dialogLoading->hide();
                    m_dialogLoading->close();
                }
            }
            break;
    }
}

void MainWindow::sortContentPane()
{
    for (int i = 0; i < __MAX_NR_BIOTYPES; i++)
        for (auto deviceInfo : deviceInfosMap[i])
            raiseContentPane(deviceInfo);
    checkBiometricPage(FingerPrint);
    checkBiometricPage(FingerVein);
    checkBiometricPage(Iris);
    checkBiometricPage(VoicePrint);
    checkBiometricPage(Face);
}

#define SET_TABLE_ATTRIBUTE(tw)                                                                                        \
    do {                                                                                                               \
        tw->setColumnCount(2);                                                                                         \
        tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);                                            \
        tw->setHorizontalHeaderLabels(QStringList() << QString(tr("Device Name")) << QString(tr("Status")));           \
        tw->verticalHeader()->setVisible(false);                                                                       \
        tw->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);                                \
        tw->setSelectionMode(QAbstractItemView::NoSelection);                                                          \
    } while (0);

void MainWindow::initDashboardBioAuthSection()
{
    connect(m_uniAuthService, &UniAuthService::bioAuthStatusChanged, this, &MainWindow::setVerificationStatus);
    connect(
        m_uniAuthService,
        &UniAuthService::defaultDeviceChanged,
        this,
        [&](const QString &userName, int bType, const QString &deviceName) {
            if (userName == getenv("USER")
                && bioTypeToIndex(bType) == uiStackedWidget->listWidgetDevicesType->currentIndex()) {
                for (auto btn : btnGroup) {
                    if (btn) {
                        btn->setChecked(false);
                    }
                }

                QString objName = deviceName + "-" + QString::number(bType);
                QRadioButton *check = findChild<QRadioButton *>(objName);
                if (check) {
                    check->setChecked(true);
                }
                for (auto defaultLabel : label_name) {
                    if (defaultLabel) {
                        if (!deviceName.isEmpty() && deviceName == defaultLabel->objectName()) {
                            defaultLabel->setText(tr("(default) ") + defaultLabel->objectName());
                        } else {
                            defaultLabel->setText(defaultLabel->objectName());
                        }
                    }
                }
            }
        });
    for (int i = 0; i <= 3; i++) {
        setVerificationStatus(getenv("USER"), i, m_uniAuthService->getBioAuthStatus(getenv("USER"), i));
    }
}

void MainWindow::initLoginSwitch()
{
    if (loginSwitch)
        setLoginSwitch(true);
    else
        setLoginSwitch(false);
}

void MainWindow::initSaverSwitch()
{
    if (saverSwitch)
        setSaverSwitch(true);
    else
        setSaverSwitch(false);
}

void MainWindow::initSystemSwitch()
{
    if (systemSwitch)
        setSystemSwitch(true);
    else
        setSystemSwitch(false);
}

void MainWindow::initDeviceTypeList()
{
    QStringList devicesTypeText = { tr("FingerPrint"), tr("Face"), tr("FingerVein"), tr("Iris"), tr("VoicePrint") };
    for (int i = 0; i < devicesTypeText.size(); i++)
        uiStackedWidget->listWidgetDevicesType->insertItem(
            uiStackedWidget->listWidgetDevicesType->count(), devicesTypeText[i]);

    uiStackedWidget->listWidgetDevicesType->setCurrentIndex(0);
}

void MainWindow::setVerificationStatus(QString userName, int type, bool status)
{
    if (userName != getenv("USER"))
        return;
    switch (type) {
        case MAIN_SWITCH:
            verificationStatus = status;
            if (status) {
                uiStackedWidget->biometricLabel->show();
                uiStackedWidget->greeterWindow->show();
                uiStackedWidget->saverWindow->show();
                uiStackedWidget->systemWindow->show();
                uiStackedWidget->btnStatus->setChecked(true);
            } else {
                uiStackedWidget->biometricLabel->hide();
                uiStackedWidget->greeterWindow->hide();
                uiStackedWidget->saverWindow->hide();
                uiStackedWidget->systemWindow->hide();
                uiStackedWidget->btnStatus->setChecked(false);
            }
            break;
        case SAVER_SWITCH:
            saverSwitch = status;
            if (status) {
                uiStackedWidget->btnSaver->setChecked(true);
            } else {
                uiStackedWidget->btnSaver->setChecked(false);
            }
            break;
        case GREETER_SWITCH:
            loginSwitch = status;
            if (status) {
                uiStackedWidget->btnLogin->setChecked(true);
            } else {
                uiStackedWidget->btnLogin->setChecked(false);
            }
            break;
        case POLKIT_SWITCH:
            systemSwitch = status;
            if (status) {
                uiStackedWidget->btnSystem->setChecked(true);
            } else {
                uiStackedWidget->btnSystem->setChecked(false);
            }
            break;
        default:
            break;
    }
}

void MainWindow::on_btnStatus_clicked()
{
    if (verificationStatus) {
        m_uniAuthService->setBioAuthStatus(MAIN_SWITCH, false);
    } else {
        m_uniAuthService->setBioAuthStatus(MAIN_SWITCH, true);
        initPerwgtSize();
    }
}

void MainWindow::on_btnLogin_clicked()
{
    if (loginSwitch) {
        m_uniAuthService->setBioAuthStatus(GREETER_SWITCH, false);
    } else {
        m_uniAuthService->setBioAuthStatus(GREETER_SWITCH, true);
    }
}

void MainWindow::on_btnSaver_clicked()
{
    if (saverSwitch) {
        m_uniAuthService->setBioAuthStatus(SAVER_SWITCH, false);
    } else {
        m_uniAuthService->setBioAuthStatus(SAVER_SWITCH, true);
    }
}

void MainWindow::on_btnSystem_clicked()
{
    if (systemSwitch) {
        m_uniAuthService->setBioAuthStatus(POLKIT_SWITCH, false);
    } else {
        m_uniAuthService->setBioAuthStatus(POLKIT_SWITCH, true);
    }
}

void MainWindow::setSaverSwitch(bool status)
{
    saverSwitch = status;
    if (status) {
        uiStackedWidget->btnSaver->setChecked(true);
    } else {
        uiStackedWidget->btnSaver->setChecked(false);
    }
}

void MainWindow::setLoginSwitch(bool status)
{
    loginSwitch = status;
    if (status) {
        uiStackedWidget->btnLogin->setChecked(true);
    } else {
        uiStackedWidget->btnLogin->setChecked(false);
    }
}

void MainWindow::setSystemSwitch(bool status)
{
    systemSwitch = status;
    if (status) {
        uiStackedWidget->btnSystem->setChecked(true);
    } else {
        uiStackedWidget->btnSystem->setChecked(false);
    }
}

void MainWindow::onDefaultDeviceChanged(bool checked)
{
    // 如果选中的是默认设备，直接return(每种设备类型至少有一个默认设备)
    if (!checked) {
        for (auto cb : btnGroup)
            if (cb == sender())
                cb->setChecked(true);
        return;
    }
    QString objName = sender()->objectName();
    int spliter = objName.lastIndexOf('-');
    QString deviceName = objName.left(spliter);
    int deviceType = objName.right(objName.length() - spliter - 1).toInt();
    for (auto cb : btnGroup)
        if (cb != sender())
            cb->setChecked(false);

    if (checked)
        m_uniAuthService->setDefaultDevice(deviceType, deviceName);
    else
        m_uniAuthService->setDefaultDevice(deviceType, "");
    on_listWidgetDevicesType_currentIndexChanged(uiStackedWidget->listWidgetDevicesType->currentIndex());
    initBiometricPage();
}

void MainWindow::update_label_width()
{
    for (int i = 0; i < label_name.count(); ++i) {
        if (label_name.at(i)->toolTip() != "")
            label_name.at(i)->setFixedWidth((this->width() - 300) / 4 - 50);
    }
}

void MainWindow::on_listWidgetDevicesType_currentIndexChanged(int currentRow)
{
    int deviceType = (currentRow);
    QStringList headerData;
    headerData << "    " + tr("Name") << tr("Status") << tr("Default") << tr("Driver");
    m_listSwitchBtns.clear();
    uiStackedWidget->tableWidgetDevices->hide();
    uiStackedWidget->tableWidgetDevices->clear();
    uiStackedWidget->tableWidgetDevices->setRowCount(0);
    uiStackedWidget->tableWidgetDevices->setColumnCount(4);
    uiStackedWidget->tableWidgetDevices->setHorizontalHeaderLabels(headerData);
    uiStackedWidget->tableWidgetDevices->setSelectionMode(QAbstractItemView::NoSelection);

    uiStackedWidget->tableWidgetDevices->setFocusPolicy(Qt::NoFocus);

    for (int i = 0; i < headerData.size(); i++) {
        if (i == 0)
            uiStackedWidget->tableWidgetDevices->horizontalHeaderItem(i)->setTextAlignment(
                Qt::AlignLeft | Qt::AlignVCenter);
        else
            uiStackedWidget->tableWidgetDevices->horizontalHeaderItem(i)->setTextAlignment(
                Qt::AlignHCenter | Qt::AlignVCenter);
    }

    uiStackedWidget->tableWidgetDevices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    int column = 0;

    while (!btnGroup.isEmpty()) {
        QRadioButton *box = btnGroup.last();
        btnGroup.removeLast();
        delete box;
    }

    for (int i = 0; i < deviceInfosMap[deviceType].count(); i++) {
        if (deviceInfosMap[deviceType].at(i)->device_available > 0) {
            deviceInfosMap[deviceType].move(i, 0);
        }
    }
    label_name.clear();
    for (auto deviceInfo : deviceInfosMap[deviceType]) {
        if (bioTypeToIndex(deviceInfo->biotype) == deviceType) {
            int row_index = uiStackedWidget->tableWidgetDevices->rowCount();
            if (column == 0)
                uiStackedWidget->tableWidgetDevices->insertRow(row_index);
            else
                row_index--;

            // 第一 设备名称
            QWidget *item_name = new QWidget(uiStackedWidget->tableWidgetDevices);

            QPushButton *labelIcon = new QPushButton(this);
            KDK_EXTEND_ALL_INFO_FORMAT(labelIcon, "Biometric Manager", "tableWidgetDevices", "driver information");

            labelIcon->setObjectName("labelIcon");
            labelIcon->installEventFilter(this);
            labelIcon->setFlat(true);
            labelIcon->setEnabled(false);
            labelIcon->setMaximumSize(20, 20);
            KALabel *labelName = new KALabel(this);
            QHBoxLayout *layout_name = new QHBoxLayout;
            layout_name->addWidget(labelIcon);
            labelIcon->setIcon(QIcon::fromTheme("preferences-system-details-symbolic"));
            layout_name->setProperty("useIconHighlightEffect", 0x8);

            layout_name->addWidget(labelName);
            labelName->setText(deviceInfo->device_shortname);
            labelName->setObjectName(deviceInfo->device_shortname);
            labelName->adjustSize();
            layout_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            item_name->setLayout(layout_name);
            labelIcon->setProperty("devicename", deviceInfo->device_shortname);
            KDK_EXTEND_ALL_INFO_FORMAT(labelName, "Biometric Manager", "tableWidgetDevices", "driver name");
            uiStackedWidget->tableWidgetDevices->setCellWidget(row_index, column, item_name);

            // 第三 默认设备（是否设为默认设备）
            QWidget *item_default = new QWidget(uiStackedWidget->tableWidgetDevices);
            item_default->setObjectName("itemDefalut");
            QRadioButton *cbDefault = new QRadioButton(this);
            KDK_EXTEND_ALL_INFO_FORMAT(
                cbDefault, "Biometric Manager", "tableWidgetDevices", "Is this driver the default device?");
            if (m_uniAuthService->getDefaultDevice(getenv("USER"), deviceInfo->biotype) == deviceInfo->device_shortname)
                cbDefault->setChecked(true);
            btnGroup.push_back(cbDefault);
            cbDefault->setObjectName(deviceInfo->device_shortname + "-" + QString::number(deviceInfo->biotype));
            connect(cbDefault, &QRadioButton::clicked, this, &MainWindow::onDefaultDeviceChanged);
            if (cbDefault->isChecked()) {
                labelName->setText(tr("(default) ") + deviceInfo->device_shortname);
            } else {
                labelName->setText(deviceInfo->device_shortname);
            }
            label_name.append(labelName);
            QHBoxLayout *layout_default = new QHBoxLayout;
            layout_default->addWidget(cbDefault);
            layout_default->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            item_default->setLayout(layout_default);
            uiStackedWidget->tableWidgetDevices->setCellWidget(row_index, column + 2, item_default);

            int flag = 0;

            // 第四 驱动状态（是否使能）
            QWidget *item_drvStatus = new QWidget();
            KSwitchButton *btnDrvStatus = new KSwitchButton(this);
            KDK_EXTEND_ALL_INFO_FORMAT(
                btnDrvStatus, "Biometric Manager", "tableWidgetDevices", "Drive to turn on the switch");
            btnDrvStatus->setObjectName(deviceInfo->device_shortname + "_" + QString::number(deviceType));
            btnDrvStatus->setFixedSize(40, 20);
            if (deviceInfo->driver_enable > 0) {
                btnDrvStatus->setChecked(true);
            } else {
                flag = 1;
                btnDrvStatus->setChecked(false);
            }
            connect(btnDrvStatus, &KSwitchButton::clicked, this, &MainWindow::onDriverStatusClicked);
            m_listSwitchBtns.append(btnDrvStatus);

            QVBoxLayout *layout = new QVBoxLayout(item_drvStatus);
            layout->addWidget(btnDrvStatus, 0, Qt::AlignVCenter | Qt::AlignHCenter);
            layout->setMargin(0);
            item_drvStatus->setLayout(layout);
            uiStackedWidget->tableWidgetDevices->setCellWidget(row_index, column + 3, item_drvStatus);

            // 第二 设备状态（是否连接）
            QWidget *item_devStatus = new QWidget(uiStackedWidget->tableWidgetDevices);
            QHBoxLayout *layout_Status = new QHBoxLayout;
            KALabel *label = new KALabel(this);
            layout_Status->addWidget(label);
            KALabel *label1 = new KALabel(this);
            layout_Status->addWidget(label1);
            setDeviceStatus(
                label,
                label1,
                deviceInfo->device_available > 0,
                flag,
                deviceInfo->biotype,
                deviceInfo->device_shortname);
            layout_Status->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            item_devStatus->setLayout(layout_Status);
            uiStackedWidget->tableWidgetDevices->setCellWidget(row_index, column + 1, item_devStatus);
            uiStackedWidget->tableWidgetDevices->setRowHeight(row_index, 45);
        }
    }
    uiStackedWidget->tableWidgetDevices->show();
}

void MainWindow::onDriverStatusClicked()
{
    QString objNameStr = sender()->objectName();
    KSwitchButton *switchBtn = qobject_cast<KSwitchButton *>(sender());
    qDebug() << objNameStr;
    int spliter = objNameStr.lastIndexOf('_');
    QString deviceName = objNameStr.left(spliter);
    int deviceType = objNameStr.right(objNameStr.length() - spliter - 1).toInt();

    qDebug() << deviceName << deviceType;

    if (deviceType != uiStackedWidget->listWidgetDevicesType->currentIndex())
        return;
    DeviceInfo *deviceInfo;
    for (auto info : deviceInfosMap[deviceType]) {
        if (info->device_shortname == deviceName) {
            deviceInfo = info;
            break;
        }
    }
    changeDeviceStatus(deviceInfo, switchBtn);
}

bool MainWindow::changeDeviceStatus(DeviceInfo *deviceInfo, KSwitchButton *switchBtn)
{
    if (!m_dialogLoading) {
        m_dialogLoading = new LoadingDialog(this, 0.4);
    } else {
        m_dialogLoading->setLoading();
    }
    m_dialogLoading->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
    m_dialogLoading->show();
    bool toEnable = deviceInfo->driver_enable <= 0 ? true : false;
    QString cmd;
    m_procEnableDriver = new QProcess(this);
    connect(
        m_procEnableDriver,
        static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        this,
        [&, this, toEnable, switchBtn](int exitCode, QProcess::ExitStatus status) {
            qDebug() << "Exit code:" << exitCode;
            if (exitCode == 0) {
                this->lastDeviceInfo = deviceInfo;
            } else {
                QMessageBox criitical(
                    QMessageBox::Critical, "", tr("Fail to change device status"), QMessageBox::NoButton, this);
                //            criitical.setWindowTitle(tr("Biometric Manager"));
                QPushButton *agreeBtn = criitical.addButton(tr("OK"), QMessageBox::AcceptRole);
                criitical.exec();
                if (switchBtn && this->m_listSwitchBtns.contains(switchBtn)) {
                    switchBtn->setChecked(!toEnable);
                }
                update();
                if (this->m_dialogLoading) {
                    this->m_dialogLoading->hide();
                    this->m_dialogLoading->close();
                    update();
                }
            }
            m_procEnableDriver->deleteLater();
        });

    if (toEnable) {
        cmd = "pkexec biodrvctl enable " //"pkexec biometric-config-tool enable-driver "
              + deviceInfo->device_shortname;
        qDebug() << cmd;
        m_procEnableDriver->start(cmd);
    } else {
        qDebug() << "disable" << deviceInfo->device_shortname;
        cmd = "pkexec biodrvctl disable " //"pkexec biometric-config-tool disable-driver "
              + deviceInfo->device_shortname;
        qDebug() << cmd;
        m_procEnableDriver->start(cmd);
    }
    return true;
}

bool MainWindow::restartService()
{
    if (!m_dialogLoading) {
        m_dialogLoading = new LoadingDialog(this, 0.4);
    } else {
        m_dialogLoading->setLoading();
    }
    m_dialogLoading->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
    m_dialogLoading->show();
    if (!m_procRestartService) {
        m_procRestartService = new QProcess(this);
        connect(
            m_procRestartService,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            [&, this](int exitCode, QProcess::ExitStatus status) {
                qDebug() << "Exit code:" << exitCode;
                if (this->m_dialogLoading) {
                    this->m_dialogLoading->hide();
                    this->m_dialogLoading->close();
                    update();
                }
                if (exitCode == 0) {
                    // 自定义icon
                    QMessageBox customMsgBox(this);
                    //                customMsgBox.setWindowTitle(tr("Biometric Manager"));
                    QPushButton *agreeBtn = customMsgBox.addButton(tr("OK"), QMessageBox::AcceptRole);
                    agreeBtn->setProperty("isImportant", true);
                    customMsgBox.setIconPixmap(
                        QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(24, 24))); // 可以设置png图形
                    customMsgBox.setText(tr("The service has restarted."));
                    customMsgBox.exec();
                } else {
                    QMessageBox criitical(
                        QMessageBox::Critical, "", tr("Service restart failure."), QMessageBox::NoButton, this);
                    QPushButton *agreeBtn = criitical.addButton(tr("OK"), QMessageBox::AcceptRole);
                    agreeBtn->setProperty("isImportant", true);
                    criitical.exec();
                }
            });
    }
    QString cmd = QString("pkexec biorestart");
    m_procRestartService->start(cmd);
    return true;
}

void MainWindow::updateDeviceListWidget(int biotype)
{
    QComboBox *lw = nullptr;

    switch (biotype) {
        case BIOTYPE_FINGERPRINT:
            lw = uiStackedWidget->listWidgetFingerPrint;
            break;
        case BIOTYPE_FINGERVEIN:
            lw = uiStackedWidget->listWidgetFingerVein;
            break;
        case BIOTYPE_IRIS:
            lw = uiStackedWidget->listWidgetIris;
            break;
        case BIOTYPE_FACE:
            lw = uiStackedWidget->listWidgetFace;
            break;
        case BIOTYPE_VOICEPRINT:
            lw = uiStackedWidget->listWidgetVoicePrint;
            break;
    }
    if (!lw)
        return;

    for (int row = 0; row < lw->count(); row++) {
        QList<DeviceInfo *> list = deviceInfosMap[bioTypeToIndex(biotype)];
        auto iter = std::find_if(list.begin(), list.end(), [&](DeviceInfo *deviceInfo) {
            return deviceInfo->device_shortname == lw->currentText();
        });
    }
}

void MainWindow::updateDevice()
{
    // wait for service restart and dbus is ready
    QTimer::singleShot(3000, this, [&, this]() {
        getDeviceInfo();
        on_listWidgetDevicesType_currentIndexChanged(uiStackedWidget->listWidgetDevicesType->currentIndex());

        initBiometricPage();
    });
}

void MainWindow::on_tableWidgetDevices_cellDoubleClicked(int row, int column)
{
    if (column % 4 != 0)
        return;
    int index = row;

    if (index < deviceInfosMap[uiStackedWidget->listWidgetDevicesType->currentIndex()].size()) {
        int deviceType = uiStackedWidget->listWidgetDevicesType->currentIndex();
        DeviceInfo *deviceInfo = deviceInfosMap[deviceType][index];
        if (deviceInfo->device_available > 0) {
            QComboBox *lw;
            switch (deviceInfo->biotype) {
                case BIOTYPE_FINGERPRINT:
                    lw = uiStackedWidget->listWidgetFingerPrint;
                    onBtnFingerPrintClicked();
                    break;
                case BIOTYPE_FINGERVEIN:
                    lw = uiStackedWidget->listWidgetFingerVein;
                    onBtnFingerVeinClicked();
                    break;
                case BIOTYPE_IRIS:
                    lw = uiStackedWidget->listWidgetIris;
                    onBtnIrisClicked();
                    break;
                case BIOTYPE_FACE:
                    lw = uiStackedWidget->listWidgetFace;
                    onBtnFaceClicked();
                    break;
                case BIOTYPE_VOICEPRINT:
                    lw = uiStackedWidget->listWidgetVoicePrint;
                    onBtnVoicePrintClicked();
                    break;
            }
            lw->setCurrentIndex(index);
        }
    }
}

void MainWindow::onUpdateDevice(int drvid, int uid, int cType)
{
    usleep(500000); // 延迟500ms，否则当控制面板进行重命名时，会出现设备忙导致，指纹列表为空
    for (int type : deviceInfosMap.keys()) {
        auto &deviceInfoList = deviceInfosMap[type];
        for (int i = 0; i < deviceInfoList.size(); i++) {
            auto deviceInfo = deviceInfoList[i];
            if (deviceInfo->device_id == drvid) {
                if (contentPaneMap.contains(deviceInfo->device_shortname)) {
                    ContentPane *pane = contentPaneMap[deviceInfo->device_shortname];
                    if (pane && !pane->getIsShowDialog()) {
                        pane->showFeatures();
                    }
                }
            }
        }
    }
}

void MainWindow::onUSBDeviceHotPlug(int drvid, int action, int devNumNow)
{
    qDebug() << "device" << (action > 0 ? "insert:" : "pull out:");
    qDebug() << "id:" << drvid;
    sleep(1);
    for (int type : deviceInfosMap.keys()) {
        auto &deviceInfoList = deviceInfosMap[type];
        for (int i = 0; i < deviceInfoList.size(); i++) {
            auto deviceInfo = deviceInfoList[i];
            if (deviceInfo->device_id == drvid) {
                qDebug() << "name:" << deviceInfo->device_shortname;
                // 更新结构体
                deviceInfo->device_available = devNumNow;
                // 更新标签页中的设备状态
                clearDeviceNum();
                getDeviceInfo();
                ContentPane *pane = contentPaneMap[deviceInfo->device_shortname];
                updateBiometricPage(deviceInfo->biotype, getDeviceNum(deviceInfo->biotype));
                on_listWidgetDevicesType_currentIndexChanged(uiStackedWidget->listWidgetDevicesType->currentIndex());
                initBiometricPage();
                return;
            }
        }
    }
}

void MainWindow::tabletStatusChanged(bool tablet_mode)
{
    Q_UNUSED(tablet_mode);
}

void MainWindow::setDeviceStatus(
    KALabel *iconLabel, KALabel *textLabel, bool connected, int flag, int bioType, QString defaultName)
{
    QString deviceStatus;
    iconLabel->setFixedSize(8, 8);
    if (connected && flag == 0) {
        deviceStatus = tr("Connected");
        iconLabel->setStyleSheet("background: #4DF46D; border-radius: 4px; opacity: 1;");
        // 如果设备状态为已连接且当前设备类型未设置默认设备，直接将该设备设为默认设备
        if (m_uniAuthService->getDefaultDevice(getenv("USER"), bioType) == "") {
            qDebug() << "设备首次接入未设置默认设备，type : " << bioType << "defaultName :" << defaultName;
            m_uniAuthService->setDefaultDevice(bioType, defaultName);
        }
    } else if (!connected && flag == 0) {
        deviceStatus = tr("Unconnected");
        iconLabel->setStyleSheet("background: #939393; border-radius: 4px; opacity: 1;");
    } else if (flag == 1) {
        deviceStatus = tr("disable");
        iconLabel->setStyleSheet("background: #FD1C1C; border-radius: 4px; opacity: 1;");
    }
    textLabel->setText(deviceStatus);
    KDK_EXTEND_ALL_INFO_FORMAT(iconLabel, "Biometric Manager", "tableWidgetDevices", "Connect information icon");
    KDK_EXTEND_ALL_INFO_FORMAT(textLabel, "Biometric Manager", "tableWidgetDevices", "Connect information");
}

void MainWindow::onServiceStatusChanged(bool activate)
{
    if (!activate) {
        // 关闭子窗口
        QMap<QString, ContentPane *>::iterator itContentPane = contentPaneMap.begin();
        for (; itContentPane != contentPaneMap.end(); itContentPane++) {
            ContentPane *pane = itContentPane.value();
            if (pane) {
                pane->clearSubWidgets();
            }
        }
        if (!m_dialogLoading) {
            m_dialogLoading = new LoadingDialog(this, 0.4);
        } else {
            m_dialogLoading->setLoading();
        }
        m_dialogLoading->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
        m_dialogLoading->show();
    } else {
        qDebug() << "onServiceStatusChanged :service restarted!";
        QTimer::singleShot(1500, this, [&, this]() {
            getDeviceInfo();
            on_listWidgetDevicesType_currentIndexChanged(uiStackedWidget->listWidgetDevicesType->currentIndex());
            initBiometricPage();
            if (this->m_dialogLoading) {
                m_dialogLoading->hide();
                this->m_dialogLoading->close();
                update();
            }
        });
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    if (!m_bInited) {
        return;
    }
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRoundedRect(240, 0, uiStackedWidget->stackedWidgetMain->width(), this->height(), 6, 6);
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void MainWindow::setStackedWidgetMain(QWidget *newWidget)
{
    uiStackedWidget->stackedWidgetMain->setCurrentWidget(newWidget);
    if (newWidget == uiStackedWidget->pageDashBoard) {
        scrollArea->listview()->setCurrentIndex(dashBoardItem->index());
    }
    update();
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!m_bInited) {
        QTimer::singleShot(50, this, [&]() { delayInit(); });
    }
    KWidget::showEvent(event);
}
