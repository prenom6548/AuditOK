/* greeterwindow.cpp
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
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QProcess>
#include <QBitmap>
#include <QLayout>
#include <QTranslator>
#include <QStandardPaths>
#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>
#include <QX11Info>
#include <QDesktopWidget>
#include <QWindow>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QtDBus/QDBusInterface>
#include <QtConcurrent/QtConcurrentRun>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QLightDM/SessionsModel>
#include <kylin-nm/kynetworkicon.h>
#include "kylin-nm/kylin-nm-interface.h"
#include <QPluginLoader>
#include "globalv.h"
#include "loginwindow.h"
#include "loginauthinterface.h"
#include "usersview.h"
#include "sessionsview.h"
#include "usersmodel.h"
#include "powerwindow.h"
#include "common/configuration.h"
#include "mainwindow.h"
#include "languagewidget.h"
#include "virtualkeyboardwidget.h"
#include "language.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libintl.h>
#include "mytabwidget.h"
#include "ktooltip.h"
#include "klabel.h"

float scale;
int fontSize;

#define TIME_TYPE_SCHEMA "org.ukui.control-center.panel.plugins"
#define ON_TAB_SHEET_WIDGET "#widgetNetworkManager{background-color: rgba(255,255,255,15%); border-radius: 4px; border: 2px solid #2C73C8;}"
#define ON_NORMAL_SHEET_WIDGET "#widgetNetworkManager{text-align:center;color: rgba(255, 255, 255, 255);border: none;border-radius: 4px;outline: none;}"
#define ON_TAB_SHEET "QPushButton{background-color: rgba(255,255,255,15%); border-radius: 4px; border: 2px solid #2C73C8;}"
#define ON_NORMAL_SHEET "QPushButton{text-align:center;color: rgba(255, 255, 255, 255);border: none;border-radius: 4px;outline: none;} \
                        QPushButton::hover{background-color: rgba(255,255,255,15%);} \
                        QPushButton::pressed {background-color: rgba(255,255,255,40%);}\
                        QPushButton::checked {background-color: rgba(255, 255, 255, 40%);}"

GreeterWindow::GreeterWindow(QWidget *parent)
    : QWidget(parent),
      m_userWnd(nullptr),
      m_loginWnd(nullptr),
      m_scrollAreaSession(nullptr),
      m_powerWnd(nullptr),
      m_sessionLB(nullptr),
      m_virtualKeyboard(nullptr),
      m_greeterServive(GreeterService::instance()),
      m_greeter(GreeterWrapper::instance()),
      m_usersModel(new UsersModel(m_greeter->hideUsersHint())),
      m_sessionsModel(new QLightDM::SessionsModel(QLightDM::SessionsModel::LocalSessions)),
      m_configuration(Configuration::instance()),
      m_sessionHasChanged(false)
//      m_kylinNM(new KylinNM(this))
{
    scale = 1.0;
    if(m_greeter->hasGuestAccountHint()){    //允许游客登录
        qDebug() << "allow guest";
        m_usersModel->setShowGuest(true);
    }
    if(m_greeter->showManualLoginHint()) {    //允许手动输入用户名
        qDebug() << "allow manual login";
        m_usersModel->setShowManualLogin(true);
    }

    m_greeter->setSession(m_greeter->defaultSessionHint());

    connect(m_greeter, SIGNAL(authenticationComplete()),this, SLOT(onAuthenticationComplete1()));
    connect(m_greeter, SIGNAL(authenticationSucess(QString)),this, SIGNAL(authenticationSucess(QString)));
    connect(m_greeterServive, &GreeterService::userChanged, this, &GreeterWindow::onServiceUserChanged);

    connect(XEventMonitor::instance(), SIGNAL(keyPress(const QString &)), this,
            SLOT(onGlobalKeyPress(const QString &)));
    connect(XEventMonitor::instance(), SIGNAL(keyRelease(const QString &)), this,
            SLOT(onGlobalkeyRelease(const QString &)));

    installEventFilter(this);

}

/*卸载网络插件*/
void GreeterWindow::unloadNetPlugin()
{
    if(wlanloader.isLoaded()){
        mkylinNM->removeTab(1);
        wlanloader.unload();
    }

    if(netloader.isLoaded()){
	mkylinNM->removeTab(0);
    	netloader.unload();
    }

    if(mkylinNM){
	mkylinNM->hide();
        mkylinNM->deleteLater();
        mkylinNM = nullptr;
    }

    if(m_kylinNM){
	m_kylinNM->hide();
        m_kylinNM->deleteLater();
        m_kylinNM = nullptr;
    }
}

void GreeterWindow::loadNetPlugin()
{
    if(netloader.isLoaded())
	return ;

    if(!m_kylinNM){
        m_kylinNM = new QWidget(this);
        m_kylinNM->setFixedSize(420,480);
        m_kylinNM->setObjectName("m_kylinNM");
        m_kylinNM->setStyleSheet("#m_kylinNM{background-color: white; border-radius:12px;}");
    }

    if(!mkylinNM){
        mkylinNM = new MyTabWidget(m_kylinNM);
        mkylinNM->setFixedSize(420,480);
        mkylinNM->tabBar()->setFixedHeight(40);
        mkylinNM->installEventFilter(this);

        QHBoxLayout *m_tabBarLayout = new QHBoxLayout(mkylinNM);
        m_tabBarLayout->setContentsMargins(0,0,0,0);
        QLabel *m_lanLabel = new QLabel(tr("LAN"));
        m_lanLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        QLabel *m_wlanLabel = new QLabel(tr("WLAN"));
        m_wlanLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        m_tabBarLayout->addWidget(m_lanLabel);
        m_tabBarLayout->addWidget(m_wlanLabel);
        mkylinNM->tabBar()->setLayout(m_tabBarLayout);
        mkylinNM->tabBar()->setFixedWidth(420);
    }

    netloader.setFileName("/usr/lib/kylin-nm/libnetconnect.so"); //有线
    wlanloader.setFileName("/usr/lib/kylin-nm/libwlanconnect.so");//无线
    
    if (netloader.load()) {
        QObject * plugin = netloader.instance();

        if (plugin) {
            Interface * pluginInstance = qobject_cast<Interface *>(plugin);
            // 插件是否启用
            if (!pluginInstance) {
                return;
            }

            pluginInstance->setPluginType(SIMPLE);
            pluginInstance->setParentWidget(this);
            QWidget *widget = pluginInstance->pluginUi();
            mkylinNM->addTab(widget,"");

        } else {
                qDebug() <<  "Load Failed: " << netloader.errorString() << "\n";
               return;
        }
    } else {
        qDebug() <<  "Load Failed: " << netloader.errorString() << "\n";
        return;
    }
    if (wlanloader.load()) {
        QObject * plugin = wlanloader.instance();

        if (plugin) {
            Interface * pluginInstance = qobject_cast<Interface *>(plugin);
            // 插件是否启用
            if (!pluginInstance) {
                return;
           }

            pluginInstance->setPluginType(SIMPLE);
	    /*需要先调用setParentWidget,再调用pluginUi,否则可能会出现wifi密码输入错误时
	     * 密码输入弹窗不能弹出来的问题*/
	    pluginInstance->setParentWidget(this);
            QWidget *widget = pluginInstance->pluginUi();
            mkylinNM->addTab(widget,"");

            //  平板模式输入状态下自动调出虚拟键盘
            connect(pluginInstance, SIGNAL(needShowVirtualKeyboard()), this, SLOT(netInPutStatus()));
        } else {
            qDebug() <<  "Load Failed: " << wlanloader.errorString() << "\n";
            return;
        }
    } else {
        qDebug() <<  "Load Failed: " << wlanloader.errorString() << "\n";
        return;
    }

    QPalette pal = qApp->palette();
    pal.setBrush(QPalette::Background, Qt::white);
    //m_kylinNM->setPalette(pal);
    mkylinNM->widget(0)->setPalette(pal);
    mkylinNM->widget(1)->setPalette(pal);

    m_kylinNM->setGeometry(this->width() - mkylinNM->width() - 20,
                 this->height() - mkylinNM->height() - 72 - 8,
                  mkylinNM->width(),
                  mkylinNM->height());

}

int GreeterWindow::getNetStatus()
{

}



void GreeterWindow::onGlobalKeyPress(const QString &key)
{
}

QString GreeterWindow::getLongFormatDate(int type)
{
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(currentUser.toLatin1().data());
    if (type == DATE) {
        QString date = dateInfo->date;
        QString week = dateInfo->week;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return  date + " " + week;
    } else if (type == TIME) {
        QString time = dateInfo->time;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        //qDebug() << " time = " << time;
        return time;
    }
}

void GreeterWindow::onGlobalkeyRelease(const QString &key)
{
//    if (key == "Escape") {
//        key_exit_release();
//    } else if (key == "Up" || key == "Down") {
//        if(scrollArea && scrollArea->isVisible())
//            m_userWnd->keyBdRelease(key);
//    } else if (key == "Return" || key == "KP_Enter") {
//        key_enter_release(key);
//    } else if (key == "Shift_L+Tab" || key == "Tab") {
//        key_tab_release(key);
//    } else if (key == "Left" || key == "Right") {
//        key_LR_release(key);
//    }
}

void GreeterWindow::setBatteryIcon(const QString &str)
{
    if (str.isEmpty() || !m_batteryiconLB)
        return;
    QPixmap pixmap = QIcon::fromTheme(str).pixmap(QSize(24, 24));
    QImage img = pixmap.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            color.setRed(255);
            color.setGreen(255);
            color.setBlue(255);
            img.setPixelColor(x, y, color);
        }
    }
    m_batteryiconLB->setIcon(QPixmap::fromImage(img));
}

void GreeterWindow::dealMessage(QDBusMessage msg)
{
    setBatteryIcon(getBatteryIconName());
}

bool GreeterWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress){
        if(obj == mkylinNM)
            return true;

	if (obj == m_virtualKeyboard) {
            return true;
        }
        if (m_virtualKeyboard && obj != m_virtualKeyboard && obj != m_keyboardLB && obj != m_powerLB &&  obj != m_nmLB && m_virtualKeyboard->isVisible()) {
            hideBottomPlugins();
        }

        if(obj == m_keyboardLB || obj == m_powerLB || obj == m_nmLB || obj == m_btnSwitchUser
           || (m_batteryiconLB && obj == m_batteryiconLB) || (m_sessionLB && obj == m_sessionLB))
            return false;

        if(tabAt != LINEEDIT) {
            m_loginWnd->setFocusin(REMOVE);
            if(tabAt == BOTTMBTN) {
                tabAt = LINEEDIT;
                setBottomBtnSheet();
                tabAt = BOTTMBTN;
            }
        }

        if(m_powerWnd && m_powerWnd->isVisible()){
            m_powerWnd->close();
	    m_loginWnd->show();
            if(!m_loginWnd->getLineeditStatus()) {
                tabAt = LINEEDIT;
                m_loginWnd->setFocusin(IN_LINEEDIT);
                if(isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
                    showVirtualKeyboard();
                }
            } else {
                tabAt = MESSAGEBTN;
                m_loginWnd->setFocusin(ON_MESSAGEBTN);
            }
            setBottomBtnSheet();
            m_powerWnd->clearStatus();
            at_power = false;
            update();
        }

        if(m_kylinNM && m_kylinNM->isVisible()){
            setCheckedSheet(NETWORKBTN, false);
            at_plugins = false;
        }

        if(mBatteryWidget && mBatteryWidget->isVisible()){
            setCheckedSheet(BATTERYBTN, false);
            at_plugins = false;
        }

        if(scrollArea && scrollArea->isVisible()) {
            setCheckedSheet(SWITCHBTN,false);
            at_plugins = false;
        }

        if(m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
            setCheckedSheet(SESSIONBTN,false);
            at_plugins = false;
        }
    }
    if(obj == m_userWnd && event->type() == 76){
        m_userWnd->onShowUsermenu();
    }
    if(obj == m_sessionsWnd && event->type() == 76){
        m_sessionsWnd->onShowSessionmenu();
    }

    if (event->type() == QEvent::ToolTip) {
        if(obj == m_keyboardLB)
            toolTipShow(tr("VirtualKeyboard"));
        else if(obj == m_powerLB)
            toolTipShow(tr("Power"));
        else if(obj == m_btnSwitchUser)
            toolTipShow(tr("SwitchUser"));
        else if(obj == m_batteryiconLB)
            toolTipShow(tr("PowerInfo"));
    } else if (event->type() == QEvent::Leave) {
        if (obj == m_keyboardLB || obj == m_powerLB || obj == m_btnSwitchUser
            || obj == m_batteryiconLB) {
            toolTipHide();
        }
    }
    return false;
}

void GreeterWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange){
        refreshTranslate();
    }
}

void GreeterWindow::refreshTranslate()
{
    if (m_usersModel) {
        m_usersModel->translateManualLogin();
    }
}

void GreeterWindow::initUI()
{
    installEventFilter(this);

    // 检查并获取第三方认证插件配置信息
    if (PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).size() > 0) {
        LoginAuthInterface *plugin = dynamic_cast<LoginAuthInterface *>(PluginsLoader::instance() \
                                                                        .findModulesByType(LoginPluginInterface::MODULETYPE_AUTH) \
                                                                        .values().first());
        QJsonObject rootObj;
        rootObj["MsgType"] = "GetInformations";
        rootObj["Key"] = "Configures";
        QString strMsg = plugin->onMessage(QJsonDocument(rootObj).toJson());
        onGetCustomPluginMsg(strMsg);
    }

    widgetTime = new QWidget(this);
    QVBoxLayout *widgetlayout = new QVBoxLayout(widgetTime);
    lblDate = new KLabel(this);
    lblTime = new KLabel(this);
    //电源对话框打开按钮
    m_powerLB = new QPushButton(this);

    //切换用户按钮
    m_btnSwitchUser = new QPushButton(this);

    //网络连接按钮
    widgetNetworkManager = new QWidget(this);
    widgetNetworkManager->setObjectName("widgetNetworkManager");
    widgetNetworkManager->setFixedSize(48,48);
    m_nmLB = new KyNetworkIcon(widgetNetworkManager);

    local = QLocale::system().language();

    widgetlayout->addWidget(lblTime);
    widgetlayout->addWidget(lblDate);

    lblTime->setAlignment(Qt::AlignCenter);
    lblTime->setStyleSheet("QLabel{color:white}");

    lblDate->setStyleSheet("QLabel{color:white;}");
    lblDate->setAlignment(Qt::AlignCenter);


    widgetTime->adjustSize();

    QString btnStyle = "QPushButton{text-align:center;color: rgb(255, 255, 255, 255);border: none;border-radius: 4px;outline: none;} \
			QPushButton::hover{background-color: rgb(255,255,255,15%);} \
            QPushButton::pressed {background-color: rgba(255,255,255,40%);}\
            QPushButton::checked {background-color: rgba(255, 255, 255, 40%)}";
    m_powerLB->setObjectName(QStringLiteral("powerButton"));
    m_powerLB->setIcon(QPixmap(":/images/shutdown.svg"));
    m_powerLB->setIconSize(QSize(24, 24));
    m_powerLB->setStyleSheet(btnStyle);
    m_powerLB->setFocusPolicy(Qt::NoFocus);
    m_powerLB->setFixedSize(48, 48);
    m_powerLB->setCursor(Qt::PointingHandCursor);
    m_powerLB->installEventFilter(this);
    connect(m_powerLB, &QPushButton::clicked, this, &GreeterWindow::showPowerWnd);

    if(QX11Info::isPlatformX11() ){
	//虚拟键盘启动按钮
        m_keyboardLB = new QPushButton(this);
        m_keyboardLB->setStyleSheet(btnStyle);
        m_keyboardLB->setObjectName(QStringLiteral("keyboardButton"));
        m_keyboardLB->setIcon(QPixmap(":/images/keyboard.svg"));
        m_keyboardLB->setIconSize(QSize(24, 24));
        m_keyboardLB->setFocusPolicy(Qt::NoFocus);
        m_keyboardLB->setFixedSize(48, 48);
        m_keyboardLB->setCursor(Qt::PointingHandCursor);
        m_keyboardLB->installEventFilter(this);
        connect(m_keyboardLB, &QPushButton::clicked,
	        this, &GreeterWindow::showVirtualKeyboard);
    }

    if(m_usersModel->rowCount() < 2){
        m_btnSwitchUser->hide();
        is_switchBtn = false;
    }

    m_btnSwitchUser->setStyleSheet(btnStyle);
    m_btnSwitchUser->setCheckable(true);
    m_btnSwitchUser->setObjectName(QStringLiteral("btnSwitchUser"));
    m_btnSwitchUser->setIcon(QIcon(":/images/switchuser.svg"));
    m_btnSwitchUser->setIconSize(QSize(24, 24));
    m_btnSwitchUser->setFocusPolicy(Qt::NoFocus);
    m_btnSwitchUser->setFixedSize(48, 48);
    m_btnSwitchUser->setCursor(Qt::PointingHandCursor);
    m_btnSwitchUser->installEventFilter(this);
    connect(m_btnSwitchUser, &QPushButton::clicked,
            this, &GreeterWindow::showUserMenu);


    //m_nmLB->setStyleSheet(btnStyle);
    m_nmLB->setCheckable(true);
    m_nmLB->setFixedSize(48,48);
    m_nmLB->setIconSize(QSize(24,24));
    m_nmLB->setFocusPolicy(Qt::NoFocus);
    m_nmLB->setCursor(Qt::PointingHandCursor);
    m_nmLB->installEventFilter(this);
//    m_nmLB->setIcon(QIcon(":/images/icon-nosignal.png"));

    connect(m_nmLB,&QPushButton::clicked
            ,this, [&,this](){
        this->showNetManager();
        this->setCursor(Qt::ArrowCursor);
    });

    //电池电量显示按钮
    batInterface = new QDBusInterface(
        "org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", QDBusConnection::systemBus(), this);

    QString batteryPath = "";
    dface = new QDBusInterface(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> reply = dface->call("EnumerateDevices");
    if (dface->isValid()) {
        for (QDBusObjectPath op : reply.value()) {
            if (op.path().contains("battery_")) {
                batteryPath = op.path();
                qDebug() << "battery path is :" << batteryPath;
                break;
            }
        }
    } else {
        qDebug() << "Enumerate devices failed";
    }
    iface = new QDBusInterface(UPOWER_SERVICE, batteryPath, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    if (dface->isValid()) {
        QDBusReply<QVariant> reply = batInterface->call("Get", "org.freedesktop.UPower", "LidIsPresent");
        if (reply.isValid()) {
            if (true == reply.value().toBool()) {
                m_batteryiconLB = new QPushButton(this);
                m_batteryiconLB->setStyleSheet(btnStyle);
                m_batteryiconLB->setObjectName(QStringLiteral("batteryButton"));
                m_batteryiconLB->setIcon(QPixmap(":/images/keyboard.svg"));
                m_batteryiconLB->setIconSize(QSize(24, 24));
                m_batteryiconLB->setFocusPolicy(Qt::NoFocus);
                m_batteryiconLB->setFixedSize(48, 48);
                m_batteryiconLB->setCursor(Qt::PointingHandCursor);
                m_batteryiconLB->installEventFilter(this);
                m_batteryiconLB->raise();

                setBatteryIcon(getBatteryIconName());
                QDBusConnection::systemBus().connect(
                    UPOWER_SERVICE, batteryPath, FREEDESKTOP_UPOWER, "PropertiesChanged", this, SLOT(dealMessage(QDBusMessage)));
                m_batteryiconLB->setFixedSize(48,48);
                m_batteryiconLB->setIconSize(QSize(24,24));
                m_batteryiconLB->setFocusPolicy(Qt::NoFocus);
                m_batteryiconLB->installEventFilter(this);
                mBatteryWidget = new BatteryWidget(QPoint(m_batteryiconLB->x(), m_batteryiconLB->y()), this);
                mBatteryWidget->hide();
                connect(m_batteryiconLB,&QPushButton::clicked
                    ,this,&GreeterWindow::showBattery);
                is_batteryBtn = true;
            } else
                is_batteryBtn = false;
        } else
            is_batteryBtn = false;
    }
    //桌面环境选择按钮
    if(m_sessionsModel->rowCount() > 1)
    {
        m_sessionLB = new QPushButton(this);
        m_sessionLB->setObjectName((QStringLiteral("sessionButton")));
	m_sessionLB->setStyleSheet(btnStyle);
        m_sessionLB->setIconSize(QSize(24, 24));
        m_sessionLB->setFocusPolicy(Qt::NoFocus);
        m_sessionLB->setFixedSize(48, 48);
        m_sessionLB->setCursor(Qt::PointingHandCursor);
        m_sessionLB->installEventFilter(this);
        m_sessionLB->setIcon(QIcon(IMAGE_DIR + QString("badges/unknown_badge.svg")));
        connect(m_sessionLB, &QPushButton::clicked, this, &GreeterWindow::showSessionWnd);
    }

    //用户列表
    scrollArea = new QScrollArea(this);
//    scrollArea->setAttribute(Qt::WA_TranslucentBackground);
    scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //屏蔽水平滚动条
    scrollArea->setStyleSheet("QScrollArea {background-color: rgba(255,255,255,20%);border-radius:8px;}");
    scrollArea->viewport()->setStyleSheet("background-color:transparent;");

    scrollContents = new QWidget(scrollArea);
    scrollContents->installEventFilter(this);
    scrollArea->setWidget(scrollContents);
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_userWnd = new UsersView(scrollContents);
    m_userWnd->installEventFilter(this);
    connect(m_userWnd, &UsersView::currentUserChanged, this, &GreeterWindow::onCurrentUserChanged);
    connect(m_userWnd, &UsersView::userSelected, this, &GreeterWindow::onUserSelected);
    scrollArea->hide();

    //登录窗口
    m_loginWnd = new LoginWindow(m_greeter, this);
    m_loginWnd->setCustomAuthDefault(m_isCustomDefault);
   // m_loginWnd->hide();
    connect(m_loginWnd, &LoginWindow::userChangedByManual,
            this, &GreeterWindow::onUserChangedByManual);
    connect(m_loginWnd, &LoginWindow::clickPassword,
            this,&GreeterWindow::onClickPassword);
    connect(m_loginWnd, &LoginWindow::showMessageBtn,
            this,&GreeterWindow::onShowMessageBtn);
    connect(m_loginWnd, &LoginWindow::setKToolTip,
            this,&GreeterWindow::toolTipShow);
    connect(m_loginWnd, &LoginWindow::hideKTooltip,
            this,&GreeterWindow::toolTipHide);
    connect(m_loginWnd, &LoginWindow::customRequestAccount, 
            this, &GreeterWindow::onCustomRequestAccount);
    connect(m_userWnd, &UsersView::userNotFound, m_loginWnd, &LoginWindow::setUserNotInView);
    connect(m_userWnd, &UsersView::setKToolTip, this, &GreeterWindow::toolTipShow);
    connect(m_userWnd, &UsersView::hideKTooltip, this, &GreeterWindow::toolTipHide);
    connect(m_loginWnd, &LoginWindow::setKToolTip, this, &GreeterWindow::toolTipShow);
    connect(m_loginWnd, &LoginWindow::hideKTooltip, this, &GreeterWindow::toolTipHide);
    connect(m_loginWnd,&LoginWindow::loginOptionClicked,this,&GreeterWindow::resetNavigation);

    m_userWnd->setModel(m_usersModel);
    setFocusProxy(m_loginWnd);
        //显示lightdm传过来的被选中的用户且自动进入认证界面 -- SwitchToUser()
        WillLoginUserInfo willSwitchUser = m_greeterServive->getWillSwitchUser();
        m_greeterServive->SwitchToUser("");
        QString selectedUser = "";
        if (willSwitchUser.strUserName.isEmpty() || !m_greeterServive->isUserNameValid(willSwitchUser.strUserName)) {
            qDebug()<<"WillSwitchUser is NULL or invalid:"<<willSwitchUser.strUserName;
            selectedUser = m_greeter->selectUserHint();
            willSwitchUser.strUserName.clear();
        }
        bool isSelectUsr = false;
        if (!willSwitchUser.strUserName.isEmpty()) {
            onServiceUserChanged(willSwitchUser);
            currentUser = willSwitchUser.strUserName;
            isSelectUsr = true;
        }
        else if(!selectedUser.isEmpty())
        {
            qDebug() << "SelectUserHint: " << selectedUser;
            if (m_userWnd->getIsUser(selectedUser)) {
                m_userWnd->setCurrentUser(selectedUser, true);
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
        else if(m_greeter->selectGuestHint())
        {
            qDebug() << "SelectGuest";
            m_userWnd->setCurrentUser("*guest", true);
        }
        //如果没有可显示出来的用户，则开启手动输入用户名登录
        else if(m_usersModel->rowCount() == 0)
        {
            m_usersModel->setShowManualLogin(true);
            QString userName = m_usersModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
            m_userWnd->setCurrentUser(userName, true);
            currentUser = userName;
        }
        //如果只有一个用户，直接进入认证界面
        else if(m_usersModel->rowCount() == 1)
        {
            QString userName = m_usersModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
            m_userWnd->setCurrentUser(userName, true);
            currentUser = userName;
        }
        else
        {
            QString lastLoginUser = Configuration::instance()->getLastLoginUser();
            if(m_userWnd->getIsUser(lastLoginUser)) {
                if( m_userWnd->getLoginStaus(lastLoginUser)){
                    m_userWnd->setCurrentUser(lastLoginUser);
                    currentUser = lastLoginUser;
                    if (m_isShowUserSwitch) {
                        //switchWnd(0);
                        showUserMenu();
                        in_switchUser = true;
		        //点击切换用户时用户选择界面会弹出来，如果分辨率较低，会出现点击
		        //选择用户界面，实际点到另一个窗口到问题，因此需要将选择用户界面置顶
		        scrollArea->raise();
                    }
                }
                else{
                    m_userWnd->setCurrentUser(lastLoginUser);
                    currentUser = lastLoginUser;
                }
            }else{
                QString userName = m_usersModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
                m_userWnd->setCurrentUser(userName);
                currentUser = userName;
            }
        }
        setWindowOpacity(0.5);
    //m_kylinNM->hide();
//获取当前屏幕尺寸
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect clientRect = desktopWidget->availableGeometry();
    width1 = clientRect.width();
    height1 = clientRect.height();

    if(is_batteryBtn)
        horAT = BATTERYBTN;
    else if(!is_batteryBtn && is_switchBtn)
        horAT = SWITCHBTN;
    else
        horAT = NETWORKBTN;

    libswitch = new LibinputSwitchEvent;
    isTableMode  = libswitch->geInitDevicesStatus();
//    if(isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
//        showVirtualKeyboard();
//    }
    connect(libswitch , &LibinputSwitchEvent::tabletModeStatusChanged, this, [ = ](int tablet_mode) {
        isTableMode = tablet_mode;
        if(isTableMode && tabAt == LINEEDIT && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {

        } else if (!isTableMode && (tabAt == BOTTMBTN && horAT == KEYBOARDBTN) && (m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
            QTimer::singleShot(100, this, [=]{
                if (!this->isTableMode) {
                    hideBottomPlugins();
                }
            });
        }
    });

    //睡眠唤醒后无外接键盘自动呼出虚拟键盘
    QDBusInterface *loginIface = new QDBusInterface("org.freedesktop.login1",
                                               "/org/freedesktop/login1",
                                               "org.freedesktop.login1.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
    connect(loginIface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
    
    m_agreementWindow = new AgreementWindow(this);

    connect(m_agreementWindow, &AgreementWindow::switchToGreeter, this,
        [this]() {
        if(m_greeter->isAuthenticated() && !m_greeter->autologinUserHint().isEmpty() && !m_greeter->authenticationUser().isEmpty()) {
            m_greeter->startSession();
            return ;
        }

        switchWnd(0);
        update();
    });

    if(m_agreementWindow->getShowLoginPrompt()){
        switchWnd(1);
        m_agreementWindow->initUI();
    }else{
        m_agreementWindow->hide();
    }

    connect(m_greeter, SIGNAL(autologinTimerExpired()),this, SLOT(timedAutologin()));

}

QString GreeterWindow::getCurUser()
{
    return currentUser;
}

QString GreeterWindow::getBatteryIconName()
{
    if (iface->isValid() && dface->isValid()) {
        bool batteryState = false;
        QDBusReply<QVariant> reply = batInterface->call("Get", UPOWER_SERVICE, "OnBattery");
        if (reply.isValid()) {
            batteryState = reply.value().toBool();
        }
        double percentage = -1.0;
        QDBusReply<QVariant> percentage_reply = iface->call("Get", UPOWER_DIVICES_SERVICE, "Percentage");
        if (percentage_reply.isValid()) {
            percentage = percentage_reply.value().toDouble();
        }

        if (true == batteryState) {
            return QString("battery-level-%1-symbolic").arg((int)percentage / 10 * 10);
        } else {
            return QString("battery-level-%1-charging-symbolic").arg((int)percentage / 10 * 10);
        }
    }
    return QString();
}

void GreeterWindow::setRootWindow()
{
    if(m_greeter->getdrawBackgroundIsStarted()){
        if(future.isStarted()&& future.isRunning()){
            future.waitForFinished();
            future = QtConcurrent::run([=](){
                m_greeter->setrootWindow();
            });
        }else{
            future = QtConcurrent::run([=](){
                m_greeter->setrootWindow();
            });
        }
    }else{
        future = QtConcurrent::run([=](){
            m_greeter->setrootWindow();
        });
    }
}

void GreeterWindow::setCurUserName(QString strUserName)
{
    m_greeter->curUserName(strUserName);
}

void GreeterWindow::showNetManager(bool keynavigation)
{
    if(!keynavigation) {
        tabAt = LINEEDIT;
        setBottomBtnSheet();
        tabAt = BOTTMBTN;
        horAT = NETWORKBTN;
    }
    if(!m_kylinNM){
	/*这里放在点击按钮后加载，一个原因是启动优化，另一个原因是如果放在构造函数中加载，则必须
         * 先让网络窗口显示出来一次，否则会在锁屏程序退出时，锁屏窗口在closeEvent中崩溃，原因未知
         * 同时，可以尽量降低在未操作网络时，网络程序对锁屏窗口的影响*/
        loadNetPlugin();
	m_kylinNM->hide(); 
    }

    if(m_kylinNM->isVisible())
    {
        setCheckedSheet(NETWORKBTN, false);
    } else {
        if(scrollArea && scrollArea->isVisible())
            setCheckedSheet(SWITCHBTN, false);
        setCheckedSheet(NETWORKBTN, true);

        if(mBatteryWidget && mBatteryWidget->isVisible()){
            setCheckedSheet(BATTERYBTN, false);
        }
        if(m_scrollAreaSession && m_scrollAreaSession->isVisible()){
            setCheckedSheet(SESSIONBTN, false);
        }
        m_kylinNM->raise();
        m_kylinNM->setGeometry(this->width() - m_kylinNM->width() - 20,
                                      this->height() - m_kylinNM->height() - 72 - 8,
                                      m_kylinNM->width(),
                                      m_kylinNM->height());
        if (m_virtualKeyboard && m_virtualKeyboard->isVisible()) {
            m_virtualKeyboard->raise();
        }
        m_loginWnd->setFocusin(REMOVE);
        at_plugins = true;
    }
}

void GreeterWindow::onShowMessageBtn(bool is_show)
{
    if(in_switchUser) {
        m_loginWnd->setFocusin(REMOVE);
        return ;
    }
    if(is_show) {
        tabAt = MESSAGEBTN;
        m_loginWnd->setFocusin(ON_MESSAGEBTN);
    }
}

void GreeterWindow::onClickPassword()
{
    tabAt = LINEEDIT;
    if(is_switchBtn)
        horAT = SWITCHBTN;
    else
        horAT = NETWORKBTN;
    setBottomBtnSheet();
    m_loginWnd->setFocusin(IN_LINEEDIT);

    if(m_kylinNM && m_kylinNM->isVisible()) {
        setCheckedSheet(NETWORKBTN, false);
        at_plugins = false;
    }

    if(scrollArea && scrollArea->isVisible()) {
        setCheckedSheet(SWITCHBTN, false);
        at_plugins = false;
    }

    if(mBatteryWidget && mBatteryWidget->isVisible()) {
        setCheckedSheet(BATTERYBTN, false);
        at_plugins = false;
    }

    if(m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
        setCheckedSheet(SESSIONBTN, false);
        at_plugins = false;
    }

//    if(m_virtualKeyboard && m_virtualKeyboard->isVisible()) {
//        m_virtualKeyboard->hide();
//        at_plugins = false;
//    }
    if(isTableMode) {
        if(!(m_virtualKeyboard && m_virtualKeyboard->isVisible()))
            showVirtualKeyboard();
    }
}

void GreeterWindow::showBattery()
{
    if(mBatteryWidget->isVisible())
        setCheckedSheet(BATTERYBTN, false);
    else
        setCheckedSheet(BATTERYBTN, true);
    if(scrollArea && scrollArea->isVisible()){
        setCheckedSheet(SWITCHBTN, false);
    }
    if (m_kylinNM && m_kylinNM->isVisible()){
        setCheckedSheet(NETWORKBTN, false);
    }
    if (m_scrollAreaSession && m_scrollAreaSession->isVisible()){
        setCheckedSheet(SESSIONBTN, false);
    }
}

void GreeterWindow::switchToSureDialog(int stateType, int lockType)
{
    powermanagerType = stateType;
    sureWidget->setStatus(stateType, lockType);
    if(m_kylinNM && m_kylinNM->isVisible())
        m_kylinNM->hide();
    if(scrollArea && scrollArea->isVisible())
        scrollArea->hide();
    if(mBatteryWidget && mBatteryWidget->isVisible())
        mBatteryWidget->hide();
    if(m_scrollAreaSession && m_scrollAreaSession->isVisible())
        m_scrollAreaSession->hide();

    m_loginWnd->hide();
    m_powerWnd->hide();
    sureWidget->show();
    widgetTime->hide();
    m_powerLB->hide();
    widgetNetworkManager->hide();
    if (m_usersModel->rowCount() > 1) {
        m_btnSwitchUser->hide();
    }
    m_keyboardLB->hide();
    if (m_batteryiconLB)
        m_batteryiconLB->hide();
}

QString GreeterWindow::getHibited_tr(int type)
{
    switch (type) {
    case SHOTDOWN:
        return tr("Shut Down");
        break;
    case REBOOT:
        return tr("Restart");
        break;
    case SLEEP:
        return tr("Sleep");
        break;
    case HIBERNATE:
        return tr("Hibernate");
        break;
    default:
        break;
    }
}

void GreeterWindow::hideSureDialog()
{
    sureWidget->hide();
    m_powerWnd->show();
    widgetTime->show();
    m_powerLB->show();
    widgetNetworkManager->show();
    if (m_usersModel->rowCount() > 1) {
        m_btnSwitchUser->show();
    }
    if (m_keyboardLB)
        m_keyboardLB->show();
    if (m_batteryiconLB)
        m_batteryiconLB->show();
    if (m_sessionLB)
        m_sessionLB->show();
    m_powerWnd->cancelClicked();
}

void GreeterWindow::confirmClicked()
{
    if(m_kylinNM && m_kylinNM->isVisible())
        m_kylinNM->hide();
    if(scrollArea && scrollArea->isVisible())
        scrollArea->hide();
    if(mBatteryWidget && mBatteryWidget->isVisible())
        mBatteryWidget->hide();
    sureWidget->hide();
    m_powerWnd->hide();
    m_loginWnd->show();
    widgetTime->show();
    m_powerLB->show();
    widgetNetworkManager->show();
    m_btnSwitchUser->show();
    m_keyboardLB->show();
    if (m_batteryiconLB)
        m_batteryiconLB->show();
    if (m_sessionLB)
        m_sessionLB->show();
    m_powerWnd->doEvent(powermanagerType);
}

void GreeterWindow::toolTipShow(const QString &text)
{
    if(text == "")
        return ;
    if(is_Show)//如果KTooltip已经show出来,直接返回
        return ;
    kTooltip = new Tooltip(this);
    kTooltip->popup(kTooltip, text, 16 + curFontSize);
    kTooltip->setStyleSheet(QString("background-color:#FFFFFF; color:black; border-radius: 4px;"));
    if(QCursor::pos().x() + kTooltip->width() > width1)
        kTooltip->move(QCursor::pos().x() - kTooltip->width() - 5, QCursor::pos().y() + 20);
    else
        kTooltip->move(QCursor::pos().x() + 5, QCursor::pos().y() + 20);
    kTooltip->show();
    is_Show = true;
}

void GreeterWindow::toolTipHide()
{
    if(!is_Show){//如果本来没有收到tooltip事件，直接返回
        return ;
    }
    kTooltip->tipClose(kTooltip);
    is_Show = false;
    kTooltip = nullptr;
}

void GreeterWindow::resetNavigation()
{
    tabAt = LINEEDIT;
    setBottomBtnSheet();
    m_loginWnd->setFocusin(IN_LINEEDIT);
}

void GreeterWindow::switchWnd(int index)
{
    if(m_userWnd)
        scrollArea->hide();
    if(m_loginWnd)
        m_loginWnd->hide();
    if(m_agreementWindow)
        m_agreementWindow->hide();
    if(widgetTime)
        widgetTime->show();

    repaint();
	
    switch (index) {
    case 0:
        if(m_agreementWindow){
            m_agreementWindow->clearFocus();
        }
        if(m_loginWnd){
            m_loginWnd->show();
            m_loginWnd->setFocus();
        }
        setBottomBtnVisible(true);
        break;
    case 1:
        if(m_agreementWindow)
            m_agreementWindow->show();
        if(widgetTime)
            widgetTime->hide();

        setBottomBtnVisible(false);
        m_loginWnd->clearFocus();
        m_agreementWindow->setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_agreementWindow);
        m_agreementWindow->setFocus();
    default:
        break;
    }
}

void GreeterWindow::setBottomBtnVisible(bool visible)
{
    if(visible){
        if(m_btnSwitchUser){
            m_btnSwitchUser->setVisible(btnVisMap.value(m_btnSwitchUser));
        }
        if(m_powerLB){
            m_powerLB->setVisible(btnVisMap.value(m_powerLB));
        }
        if(m_keyboardLB){
            m_keyboardLB->setVisible(btnVisMap.value(m_keyboardLB));
        }
        if(m_sessionLB){
            m_sessionLB->setVisible(btnVisMap.value(m_sessionLB));
        }
        if(m_nmLB){
            m_nmLB->setVisible(btnVisMap.value(m_nmLB));
        }
        if(m_batteryiconLB){
            m_batteryiconLB->setVisible(btnVisMap.value(m_batteryiconLB));
        }
    }else{
        if(m_btnSwitchUser){
            btnVisMap.insert(m_btnSwitchUser,!m_btnSwitchUser->isHidden());
            m_btnSwitchUser->setVisible(visible);
        }
        if(m_powerLB){
            btnVisMap.insert(m_powerLB,!m_powerLB->isHidden());
            m_powerLB->setVisible(visible);
        }
        if(m_keyboardLB){
            btnVisMap.insert(m_keyboardLB,!m_keyboardLB->isHidden());
            m_keyboardLB->setVisible(visible);
        }
        if(m_sessionLB){
            btnVisMap.insert(m_sessionLB,!m_sessionLB->isHidden());
            m_sessionLB->setVisible(visible);
        }
        if(m_nmLB){
            btnVisMap.insert(m_nmLB,!m_nmLB->isHidden());
            m_nmLB->setVisible(visible);
        }
        if(m_batteryiconLB){
            btnVisMap.insert(m_batteryiconLB,!m_batteryiconLB->isHidden());
            m_batteryiconLB->setVisible(visible);
        }
    }
}

QString GreeterWindow::guessBackground()
{
    QString selectedUser = m_greeter->selectUserHint();

    if(!selectedUser.isEmpty())
    {
        return m_configuration->getDefaultBackgroundName();
    }
    else if(m_greeter->selectGuestHint())
    {
        return m_configuration->getDefaultBackgroundName();
    }
    else if(m_usersModel->rowCount() == 0)
    {
        return m_configuration->getDefaultBackgroundName();
    }
    else
    {
        QString userName;
        if(m_usersModel->rowCount() == 1)
            userName = m_usersModel->index(0, 0).data(QLightDM::UsersModel::NameRole).toString();
        else
            userName = Configuration::instance()->getLastLoginUser();

        struct passwd *user = NULL;
        user = getpwnam(userName.toLatin1().data());

        if(user){
            QString   backgroundPath = getAccountBackground(user->pw_uid);
            if(backgroundPath.isEmpty())
                backgroundPath = m_configuration->getDefaultBackgroundName();

            QString userConfigurePath = m_greeter->getEnsureShareDir(userName) + "/ukui-greeter.conf";
            QFile backgroundFile(userConfigurePath);
            if(backgroundFile.exists()){
                QSettings settings(userConfigurePath,QSettings::IniFormat);
                settings.beginGroup("greeter");
                if(settings.contains("backgroundPath")){
                    QString filepath = settings.value("backgroundPath").toString();
                    if(!filepath.isEmpty() && isPicture(filepath)){
                        return filepath;
                    }
                }
            }
	    return backgroundPath;
        }
        return m_configuration->getDefaultBackgroundName();
    }
}

/**
 * @brief GreeterWindow::resizeEvent
 * @param event
 * GreeterWindow的大小发生改变
 */
void GreeterWindow::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    //重新计算缩放比例
    scale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    //设置认证窗口左右居中
    if (m_loginWnd) {
        m_loginWnd->setChildrenGeometry();
        if(scale >= 1.3) {
            int nHeight = (height()*3/4)-(height()/(4-0.7*scale)-height()/4+72);
            m_loginWnd->setGeometry((width()-m_loginWnd->geometry().width())/2,height()/(4-0.7*scale), \
                                m_loginWnd->width(), nHeight);
        } else if(scale <= 0.9) {
            int nHeight = (height()*3/4)-(height()/(4-0.9*scale)-height()/4+72);
            m_loginWnd->setGeometry((width()-m_loginWnd->geometry().width())/2,height()/(4-0.9*scale), \
                                    m_loginWnd->width(), nHeight);
        } else {
            int nHeight = (height()*3/4)-(height()/(4-0.65*scale)-height()/4+72);
            m_loginWnd->setGeometry((width()-m_loginWnd->geometry().width())/2,height()/(4-0.65*scale), \
                                    m_loginWnd->width(), nHeight);
        }
    }

    if(scale > 1)
        scale = 1;
    fontSize = scale > 0.5 ? 10 : 8;

    qDebug() << "GreeterWindow resize to " << size;
    qDebug() << "scale: " << scale;
//对距离进行缩放
    widgetTime->move((width()-widgetTime->geometry().width())/2, 59*scale); //距离顶部59*scale的距离

//    if(scrollArea){
//        QRect userRect(0.05*width(),
//                       0.2*height(),
//                       width()*0.9, height()*0.6);
//        scrollArea->setGeometry(userRect);
//    }

//    if(scrollContents){
//        scrollContents->setFixedWidth(scrollArea->width()-20);
//    }

//    if(m_userWnd){
//        m_userWnd->resize(m_userWnd->getSize());
//        if(m_userWnd->height() + 2*30*scale  < scrollArea->height()){
//            m_userWnd->move((scrollArea->width()-m_userWnd->width())/2,(scrollArea->height() - m_userWnd->height())/2);
//        }else{
//            m_userWnd->move((scrollArea->width()-m_userWnd->width())/2,30*scale);
//        }
//        scrollContents->setFixedHeight(m_userWnd->height() + m_userWnd->y() );
//    }
    
    if(m_powerWnd){
    	m_powerWnd->move((width()-m_powerWnd->width())/2,(height() - m_powerWnd->height())/2);
    }

    if(sureWidget){
        sureWidget->move((width()-sureWidget->width())/2,(height() - sureWidget->height())/2);
    }

    if(m_agreementWindow){
        m_agreementWindow->setGeometry(this->geometry());
    }

    //电源按钮位置,根据x，y的值进行计算，初始值为到右下角的距离

    int x = 24;
    int y = 72;

    if(m_powerLB){
        x+=m_powerLB->width();
        m_powerLB->move(this->width() - x,height() - y);
    }

    //虚拟键盘按钮位置
    if(m_keyboardLB){
        x += (m_keyboardLB->width() + 16); //24为间隔
        m_keyboardLB->move(this->width() - x, height() - y);
    }

    //网络按钮位置
    if(widgetNetworkManager)
    {
        x += (widgetNetworkManager->width() + 16); //24为间隔
        widgetNetworkManager->move(this->width() - x, height() - y);
    }

    //切换用户按钮位置
    if(is_switchBtn){
        x += (m_btnSwitchUser->width() + 16); //24为间隔
        m_btnSwitchUser->move(this->width() - x, height() - y);
    }

    if (m_batteryiconLB) {
        x += (m_batteryiconLB->width() + 16); //24为间隔
        m_batteryiconLB->move(this->width() - x, height() - y);
    }

    //桌面环境选择按钮位置
    if(m_sessionLB)
    {
        x += (m_sessionLB->width() + 16); //24为间隔
        m_sessionLB->move(this->width() - x, height() - y);
    }

    //虚拟键盘位置
    setVirkeyboardPos();

    m_userWnd->adjustSize();
    scrollContents->setFixedSize(m_userWnd->width(),m_userWnd->height());
    if(scrollArea){
        if(scrollContents->height() < height()/2){
            scrollArea->setFixedSize(m_userWnd->width(),scrollContents->height()+1);
	    scrollArea->clearMask();
        }else{
            scrollArea->setFixedSize(m_userWnd->width() + 16,height()/2);
            /*当scrollArea区域小于scrollContent大小时，使用样式表绘制的圆角外部透明部分，会显示出scrollContent的内容
            * 导致圆角外部有阴影，因此使用另一种方式再次绘制圆角，该方式绘制出来的圆角存在一定锯齿，所以只在会有阴影
            * 的情况下执行。*/
            QBitmap bitMap(scrollArea->width(),scrollArea->height()); // 定义一个窗口大小的位图
            bitMap.fill();                                // 填充整个位图
            QPainter painter(&bitMap);                    // 定义绘图设备
            painter.setBrush(Qt::black);                  // 必须定义为黑色
            painter.setPen(Qt::NoPen);                    // 只能为黑色或者不设置画笔
            painter.setRenderHint(QPainter::Antialiasing);// 反走样
            painter.drawRoundedRect(bitMap.rect(),8,8); // 绘制圆角矩形
            scrollArea->setMask(bitMap);
	}
        scrollArea->move(m_btnSwitchUser->geometry().x()- scrollArea->width()/2 + 24, \
                    height() - y - scrollArea->height() - 8);
    }

    if(m_kylinNM){
        m_kylinNM->setGeometry(this->width() - m_kylinNM->width() - 20,
                               this->height() - y - m_kylinNM->height() - 8,
                               m_kylinNM->width(),
                               m_kylinNM->height());
    }

    // session widget
    if (m_sessionsWnd)
        m_sessionsWnd->adjustSize();
    if (m_scrollContentSession)
        m_scrollContentSession->setFixedSize(m_sessionsWnd->width(),m_sessionsWnd->height());
    if(m_scrollAreaSession){
        if(m_scrollContentSession->height() < height()/2){
            m_scrollAreaSession->setFixedSize(m_sessionsWnd->width(),m_scrollContentSession->height()+1);
        }else{
            m_scrollAreaSession->setFixedSize(m_sessionsWnd->width() + 16,height()/2);
        }
        m_scrollAreaSession->move(m_sessionLB->geometry().x()- m_scrollAreaSession->width()/2 + 24, \
                    height() - y - m_scrollAreaSession->height() - 8);
    }

    if (mBatteryWidget && m_batteryiconLB) {
        mBatteryWidget->setFixedSize(314, 112);
        mBatteryWidget->setPoint(QPoint(width() - 334, m_batteryiconLB->y() - 16));
    }

}

void GreeterWindow::updateNetIcon(int status)
{
}

void GreeterWindow::setVirkeyboardPos()
{
    if(m_virtualKeyboard)
    {
	    //设置屏幕键盘的高度为窗口高度的三分之一
       m_virtualKeyboard->adjustGeometry();
    }
}

void GreeterWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_K:
        if(e->modifiers() & Qt::ControlModifier)
            m_keyboardLB->click();
        break;
    case Qt::Key_P:
        if(e->modifiers() & Qt::ControlModifier)
            m_powerLB->click();
        break;
    case Qt::Key_Escape:
        key_exit_release();
        break;
    }
    if (e->key() == Qt::Key_Tab) {
        key_tab_release(e->key());
    } else if(e->key() == Qt::Key_Backtab){
        if(e->modifiers() & Qt::ShiftModifier)
            key_shiftTab_release();
    } else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {  // "Up" "Down"
        if(scrollArea && scrollArea->isVisible()) {
            key_OB_release(e->key());
        } else if(m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
            key_OB_release(e->key());
        }
    } else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right ) { // "Left" "Right"
        key_LR_release(e->key());
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space) {  // "Return" "KP_Enter" "Space"
        key_enter_release(e->key());
    }
    QWidget::keyReleaseEvent(e);
}

QString GreeterWindow::getAccountBackground(uid_t uid)
{

    QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)uid);
    if(!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface("org.freedesktop.Accounts", userPath.value().path(),
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply = userIface.call("Get", "org.freedesktop.Accounts.User", "BackgroundFile");
        if(backgroundReply.isValid())
            return  backgroundReply.value().variant().toString();
    }
    return "";
}

void GreeterWindow::setBackground(const QModelIndex &index)
{
    QString backgroundPath;
    QString accountBackground; //记录来自accountsservice的背景图片

    backgroundPath = index.data(QLightDM::UsersModel::BackgroundPathRole).toString();
    accountBackground = backgroundPath;

    QFile backgroundPathFile(backgroundPath);
    if(backgroundPath.isEmpty() || !backgroundPathFile.exists())
    {
        uid_t uid = index.data(QLightDM::UsersModel::UidRole).toUInt();
        backgroundPath = getAccountBackground(uid);
        accountBackground = backgroundPath;
	QFile accountBackgroundFile(accountBackground);
        if(backgroundPath.isEmpty() || !accountBackgroundFile.exists())
            backgroundPath = m_configuration->getDefaultBackgroundName();
    }
    //登录后绘制桌面背景而不是登录背景
    m_greeter->setRootWindowBgOptions(m_configuration->getRootBackgroundOption(index.data(QLightDM::UsersModel::NameRole).toString()));
    m_greeter->setrootWindowBackground(rootWinPicture,0,backgroundPath);

    //读取/var/lib/lightdm-date/用户名/ukui-greeter.conf,
    //判断是否设置了该用户的登陆界面的背景图片.
    QString userConfigurePath = m_greeter->getEnsureShareDir(index.data(QLightDM::UsersModel::NameRole).toString()) + "/ukui-greeter.conf";
    QFile backgroundFile(userConfigurePath);
    if(backgroundFile.exists()){
        QSettings settings(userConfigurePath,QSettings::IniFormat);
        settings.beginGroup("greeter");
        if(settings.contains("backgroundPath")){
            QString filepath = settings.value("backgroundPath").toString();
	    QFile backgroundFile(filepath);
	    //如果filepath不为空，格式为图片，文件存在
            if(!filepath.isEmpty() && isPicture(filepath) && backgroundFile.exists()){
                backgroundPath = filepath;
            }
        }
        if(accountBackground.isEmpty() && settings.contains("color")){
            QString drawBackgroundColor = settings.value("color").toString();
       	    if(drawBackgroundColor.length() == 7 && drawBackgroundColor.startsWith("#")){
            drawBackgroundColor = drawBackgroundColor.remove(0,1);
                bool ok;
                int color = drawBackgroundColor.toUInt(&ok,16);
                m_greeter->setrootWindowBackground(rootWinColor,color,NULL);
            }
        }
        settings.endGroup();
    }else{
	    //如果配置文件不存在，就创建一个
    	QSettings settingsbak(userConfigurePath,QSettings::IniFormat);
        settingsbak.beginGroup("greeter");
        settingsbak.setValue("backgroundPath",m_configuration->getDefaultBackgroundName());
        settingsbak.endGroup();
        settingsbak.sync();
        QFile file(userConfigurePath);
	//给写权限，让控制面板可以设置
        file.setPermissions(QFile::WriteOther | QFile::ReadOther);
    }

    QSharedPointer<Background> background(new Background);
    background->type = BACKGROUND_IMAGE;
    background->image = backgroundPath;
    static_cast<MainWindow*>(parentWidget())->setBackground(background);

}

void GreeterWindow::onUserSelected(const QModelIndex &index)
{
    qDebug() << index.data(QLightDM::UsersModel::NameRole).toString() << "selected";
    m_loginWnd->setWillLoginUser("", false);
    m_loginWnd->setUserIndex(index);

    //switchWnd(1);

}

void GreeterWindow::updateLanguage(QString userName)
{
    QString language;
    QString formatsLocale;
    QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserByName", userName);
    if(!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface("org.freedesktop.Accounts", userPath.value().path(),
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> languageReply = userIface.call("Get", "org.freedesktop.Accounts.User", "Language");
        if(!languageReply.isValid())
            qWarning() << "Get User's language error" << languageReply.error();
        else {
            language = languageReply.value().variant().toString();
            if(!language.isEmpty()){
                    onLanguageChanged(language);
             }
        }

        QDBusReply<QDBusVariant> fomatsLocalReply = userIface.call("Get", "org.freedesktop.Accounts.User", "FormatsLocale");
        if(!fomatsLocalReply.isValid())
            qWarning() << "Get User's language error" << languageReply.error();
        else {
            formatsLocale = fomatsLocalReply.value().variant().toString();
            if(!formatsLocale.isEmpty()){
                if(formatsLocale.startsWith("zh")){
                    local = QLocale::Chinese;
                }
                else{
                    local = QLocale::English;
        }
            }
        }

    }
}

void GreeterWindow::updateSession(QString userName)
{
    QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserByName", userName);
    if(!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface("org.freedesktop.Accounts", userPath.value().path(),
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> sessionReply = userIface.call("Get", "org.freedesktop.Accounts.User", "XSession");
        if(!sessionReply.isValid())
            qWarning() << "Get User's session error" << sessionReply.error();
        else {
            QString session = sessionReply.value().variant().toString();
             onSessionChanged(session);
        }
    }
}

void GreeterWindow::onCurrentUserChanged(const QModelIndex &index)
{
    for(int i = 0;i<m_userWnd->userlist.count();i++)
    {
        UserEntry *entry = m_userWnd->userlist.at(i);
          if(entry->userIndex().data(QLightDM::UsersModel::NameRole).toString() == "*login")
               entry->setUserName(tr("Login"));
    }

    setBackground(index);


    if(m_greeter->getdrawBackgroundIsStarted()){
        if(future.isStarted()&& future.isRunning()){
            future.waitForFinished();
        }else{
            future = QtConcurrent::run([=](){
                m_greeter->setrootWindow();
            });
        }
    }else{
        future = QtConcurrent::run([=](){
            m_greeter->setrootWindow();
        });
    }

    QString realName = index.data(QLightDM::UsersModel::NameRole).toString();
    bool islogin = index.data(QLightDM::UsersModel::LoggedInRole).toBool();

    if(islogin || realName == "*login" || realName == "*guest"){
        if(m_sessionLB){
            m_sessionLB->hide();
            m_isSessionBtnShow = false;
        }
    }
    else{
        if(m_sessionLB){
            m_sessionLB->show();
            m_isSessionBtnShow = true;
        }
    }

    if(realName == "*guest" || realName == "*login")
        return;

    updateLanguage(realName);
    currentUser = realName;
    Q_EMIT currentUserChange(realName);
    if(!m_sessionHasChanged && m_sessionsModel->rowCount() > 1)
    {
        QString session = index.data(QLightDM::UsersModel::SessionRole).toString();
        onSessionChanged(session);
    }
    m_configuration->getCurrentUser(realName);
    QVariant varTimeType = m_configuration->getUserConfig("timeType");
    QVariant varDateType = m_configuration->getUserConfig("dateType");
    timeType = varTimeType.toInt();
    dataType = varDateType.toString();
    if(QGSettings::isSchemaInstalled(TIME_TYPE_SCHEMA)){
        QGSettings *time_type = new QGSettings(TIME_TYPE_SCHEMA);
        if (time_type) {
            QStringList keys = time_type->keys();
            if (!varTimeType.isValid() && keys.contains("hoursystem")) {
                timeType = time_type->get("hoursystem").toInt();
            }
            if (!varDateType.isValid() && keys.contains("date")) {
                dataType = time_type->get("date").toString();
            }
            time_type->deleteLater();
        }
    }
    curFontSize = m_configuration->getUserConfig("fontSize").toDouble();
    lblTime->setFontSize(36 + curFontSize);
    lblDate->setFontSize(18 + curFontSize);

    if(!timer){
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [&]{
//            if(timeType == 12){
//                QString time = local.toString(QDateTime::currentDateTime(),"A hh:mm");
//                lblTime->setText(time);
//            }else{
//                QString time = local.toString(QDateTime::currentDateTime(),"hh:mm");
//                lblTime->setText(time);
//            }
            lblTime->setText(getLongFormatDate(TIME));
//            if(dataType == "cn"){
//                QString date = local.toString(QDate::currentDate(),"yyyy/MM/dd ddd").replace("周", "星期");
//                lblDate->setText(date);
//            }else{
//                QString date = local.toString(QDate::currentDate(),"yyyy-MM-dd ddd").replace("周", "星期");
//                lblDate->setText(date);
//            }
            lblDate->setText(getLongFormatDate(DATE));
        });
        timer->start(1000);
    }

//    if(timeType == 12){
//        QString time = local.toString(QDateTime::currentDateTime(),"A hh:mm");
//        lblTime->setText(time);
//    }else{
//        QString time = local.toString(QDateTime::currentDateTime(),"hh:mm");
//        lblTime->setText(QDateTime::currentDateTime().toString("hh:mm"));
//    }
    lblTime->setText(getLongFormatDate(TIME));
//    if(dataType == "cn"){
//        QString date = local.toString(QDate::currentDate(),"yyyy/MM/dd ddd").replace("周", "星期");
//        lblDate->setText(date);
//    }else{
//        QString date = local.toString(QDate::currentDate(),"yyyy-MM-dd ddd").replace("周", "星期");
//        lblDate->setText(date);
//    }
    lblDate->setText(getLongFormatDate(DATE));
    widgetTime->adjustSize();
    widgetTime->move((width()-widgetTime->geometry().width())/2, 59*scale); //距离顶部59*scale的距离
    update();
}

void GreeterWindow::onUserChangedByManual(const QString &userName)
{
    m_userWnd->setCurrentUser(userName, true);

    updateLanguage(userName);
    updateSession(userName);
}

void GreeterWindow::setWindowVisible()
{
    if(m_powerWnd && m_powerWnd->isVisible())
        m_powerWnd->close();

    m_loginWnd->show();
    update();
}
/**
 * @brief GreeterWindow::showPowerWnd
 * 显示电源对话框
 */
void GreeterWindow::showPowerWnd(bool keynavigation)
{
    if (!at_power && (m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
        m_isShowKeyboard = true;
        hideBottomPlugins();
    } else if (!at_power && (m_virtualKeyboard && !m_virtualKeyboard->isVisible())) {
        m_isShowKeyboard = false;
    }

    //如果已经打开了电源对话框则关闭
    if(m_powerWnd && !m_powerWnd->isHidden()){
        m_powerWnd->close();
        //m_powerWnd->clearStatus();
        //switchWnd(1);
        m_loginWnd->show();

        if(!m_loginWnd->getLineeditStatus()) {
            tabAt = LINEEDIT;
            setBottomBtnSheet();
            m_loginWnd->setFocusin(IN_LINEEDIT);
//            if (m_isShowKeyboard && isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
//                showVirtualKeyboard();
//            }
        } else {
            tabAt = MESSAGEBTN;
            m_loginWnd->setFocusin(ON_MESSAGEBTN);
        }
        at_power = false;
        update();
        return;
    }
    if(m_kylinNM && m_kylinNM->isVisible())
        setCheckedSheet(NETWORKBTN, false);
    if(scrollArea && scrollArea->isVisible())
        setCheckedSheet(SWITCHBTN,false);
    if(mBatteryWidget && mBatteryWidget->isVisible())
        setCheckedSheet(BATTERYBTN,false);
    if(m_scrollAreaSession && m_scrollAreaSession->isVisible())
        setCheckedSheet(SESSIONBTN,false);
    m_loginWnd->hide();

    m_powerWnd = new PowerManager( m_userWnd->getLoginNum(), this);
    sureWidget = new Surewidget(this);
    m_powerWnd->adjustSize();
//    m_powerWnd->setGeometry((width()-ITEM_WIDTH*5)/2,
//                              (height()-ITEM_HEIGHT)/2,
//                              ITEM_WIDTH*5,ITEM_HEIGHT);
    m_powerWnd->setFixedSize(m_powerWnd->windowSize());
    m_powerWnd->move((width()-m_powerWnd->width())/2,(height() - m_powerWnd->height())/2);
    sureWidget->move((width()-sureWidget->width())/2,(height() - sureWidget->height())/2);
    sureWidget->hide();

//    connect(m_powerWnd,SIGNAL(switchToUser())
//            ,this,SLOT(setWindowVisible()));
//    connect(m_powerWnd, &PowerManager::switchToUser, this,
//            [this]() {
//        if(m_powerWnd && m_powerWnd->isVisible()) {
//            m_powerWnd->close();
//            m_loginWnd->show();
//            update();
//        }
//    });
    connect(m_powerWnd, SIGNAL(is_blockInhibited(int, int)),
            this,SLOT(switchToSureDialog(int, int)));
    connect(m_powerWnd, SIGNAL(suspendClicked()),
            this,SLOT(showPowerWnd()));
    connect(sureWidget, SIGNAL(cancelButtonclicked()),
            this, SLOT(hideSureDialog()));
    connect(sureWidget, SIGNAL(confirmButtonclicked()),
            this, SLOT(confirmClicked()));
    connect(m_powerWnd, SIGNAL(setKToolTip(QString)),
            this,SLOT(toolTipShow(QString)));
    connect(m_powerWnd, SIGNAL(hideKTooltip()),
            this,SLOT(toolTipHide()));

    m_powerWnd->setObjectName(QStringLiteral("powerWnd"));
    m_powerWnd->show();
    if(keynavigation){
        m_powerWnd->keyBdRelease(Qt::Key_Right);
        tabAt = POWERMANAGER;
        setBottomBtnSheet();
    } else {
        tabAt = LINEEDIT;
        setBottomBtnSheet();
        tabAt = BOTTMBTN;
        horAT = POWERBTN;
    }
    at_power = true;
    update();
}

void GreeterWindow::netInPutStatus()
{
    if(isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible()))
        showVirtualKeyboard();
}

/**
 * @brief GreeterWindow::showVirtualKeyboard
 * 显示或隐藏虚拟键盘
 */
void GreeterWindow::showVirtualKeyboard()
{
    tabAt = LINEEDIT;
    setBottomBtnSheet();
    tabAt = BOTTMBTN;
    horAT = KEYBOARDBTN;
//    if(m_kylinNM && m_kylinNM->isVisible())
//        m_kylinNM->hide();

    if(!m_virtualKeyboard)
    {
        m_virtualKeyboard = new VirtualKeyboardWidget(this);
        connect(m_virtualKeyboard, &VirtualKeyboardWidget::aboutToClose,
                this, &GreeterWindow::hideBottomPlugins);
        connect(m_virtualKeyboard, &VirtualKeyboardWidget::aboutToFloat,
                this, &GreeterWindow::netResetLocation);
        m_virtualKeyboard->installEventFilter(this);
    }
    m_virtualKeyboard->setVisible(!m_virtualKeyboard->isVisible());
    if(!m_virtualKeyboard->isHidden()){
       if(m_kylinNM && m_kylinNM->isVisible() && !m_virtualKeyboard->getFloatStatus()) {
           m_kylinNM->move(this->width() - m_kylinNM->width() - 20,
                                       this->height() - m_kylinNM->height() - m_virtualKeyboard->height());
           m_kylinNM->raise();
       } else
           m_loginWnd->setFocusin(IN_LINEEDIT);
       if(mBatteryWidget && mBatteryWidget->isVisible()){
           setCheckedSheet(BATTERYBTN,false);
       }
       if(scrollArea && scrollArea->isVisible()){
           setCheckedSheet(SWITCHBTN,false);
       }
       if(m_scrollAreaSession && m_scrollAreaSession->isVisible()){
           setCheckedSheet(SESSIONBTN,false);
       }
    }
    m_virtualKeyboard->raise();
    //setVirkeyboardPos();
}

void GreeterWindow::netResetLocation()
{
    if (m_kylinNM && m_kylinNM->isVisible()) {
        if (m_virtualKeyboard->getFloatStatus()) {
            m_kylinNM->move(this->width() - m_kylinNM->width() - 20,
                                              this->height() - m_kylinNM->height() - 72 - 8) ;
        } else {
            m_kylinNM->move(this->width() - m_kylinNM->width() - 20,
                                        this->height() - m_kylinNM->height() - m_virtualKeyboard->height());
        }
        m_kylinNM->raise();
    }
    if (m_virtualKeyboard && m_virtualKeyboard->isVisible()) {
        m_virtualKeyboard->raise();
    }
}

void GreeterWindow::showUserMenu()
{
    tabAt = LINEEDIT;
    setBottomBtnSheet();
    tabAt = BOTTMBTN;
    horAT = SWITCHBTN;

    if(scrollArea && scrollArea->isVisible()){
        scrollArea->hide();
        setCheckedSheet(SWITCHBTN,false);
        at_plugins = false;
        m_loginWnd->setFocusin(REMOVE);
    } else{
        if(m_kylinNM && m_kylinNM->isVisible()) {
            m_kylinNM->hide();
            setCheckedSheet(NETWORKBTN,false);
        }

        if(mBatteryWidget && mBatteryWidget->isVisible()){
            mBatteryWidget->hide();
            setCheckedSheet(BATTERYBTN,false);
        }

        if(m_scrollAreaSession && m_scrollAreaSession->isVisible()){
            m_scrollAreaSession->hide();
            setCheckedSheet(SESSIONBTN,false);
        }

        scrollArea->show();
        setCheckedSheet(SWITCHBTN,true);
        scrollArea->raise();
        m_loginWnd->setFocusin(REMOVE);
        at_plugins = true;
    }
}

void GreeterWindow::onPrepareForSleep(bool sleep)
{
    if(isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
        showVirtualKeyboard();
    }
}

void GreeterWindow::hideBottomPlugins()
{
    if(m_kylinNM && m_kylinNM->isVisible()) {
        m_kylinNM->move(this->width() - m_kylinNM->width() - 20,
                                          this->height() - m_kylinNM->height() - 72 - 8) ;
        m_kylinNM->raise();
    }

    if(scrollArea && scrollArea->isVisible())
        setCheckedSheet(SWITCHBTN, false);

    if(m_virtualKeyboard && m_virtualKeyboard->isVisible()) {
        m_virtualKeyboard->hide();
        m_loginWnd->setFocusin(REMOVE);
        at_plugins = false;
    }
    if(m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
        setCheckedSheet(SESSIONBTN, false);
        at_plugins = false;
    }
    //tabAt = EMPTY;
    if(!m_loginWnd->getLineeditStatus())
        tabAt = LINEEDIT;
    setBottomBtnSheet();
}

/**
 * @brief GreeterWindow::timedAutologin
 * 处理延时自动登录
 */
void GreeterWindow::timedAutologin()
{
    if(m_agreementWindow && m_agreementWindow->getShowLoginPrompt()){
        return;
    }

    if(m_greeter->isAuthenticated()) {
        if(!m_greeter->authenticationUser().isEmpty())
            m_greeter->startSession();
        else if(m_greeter->autologinGuestHint())
            m_greeter->startSession();
        else if(!m_greeter->autologinUserHint().isEmpty())
            m_greeter->authenticate(m_greeter->autologinUserHint());
    }
    else
        m_greeter->authenticateAutologin();
}

void GreeterWindow::setWindowPos(QWidget *widget, Qt::Alignment align)
{
    QRect rect;
    rect.setSize(widget->size());

    if(align & Qt::AlignHCenter)
    {
        rect.setLeft((width() - widget->width())/2);
    }
    if(align & Qt::AlignVCenter)
    {
        rect.setTop((height() - widget->height())/2);
    }
    if(align & Qt::AlignBottom)
    {
        rect.setTop(height() - widget->height());
    }
    widget->setGeometry(rect);
}

void GreeterWindow::onLanguageChanged(QString languageCode)
{
    if(languageCode == "")
        return ;

    if(m_greeter->lang() == languageCode)
    {
        return;
    }

    qDebug()<<"onLanguageChanged "<<languageCode;

    qApp->removeTranslator(m_configuration->m_trans);
    delete m_configuration->m_trans;
    m_configuration->m_trans = new QTranslator();
    QString qmFile;

    if(languageCode.startsWith("zh")){ /*这里写的有问题，忘记之前为什么写了，会导致繁体也会加载zh_CN*/
        qmFile = QM_DIR + QString("%1.qm").arg("zh_CN");
        setenv("LANGUAGE","zh_CN",1);
        setenv("LANG","zh_CN",1);
        setlocale(LC_ALL,"zh_CN.utf8");
	QLocale lang("zh_CN");
	/*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
	 * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
	QLocale::setDefault(lang);
    }
    else{
        qmFile = QM_DIR + QString("%1.qm").arg(languageCode);
        setenv("LANGUAGE",languageCode.toLatin1().data(),1);
        setenv("LANG",languageCode.toLatin1().data(),1);
        setlocale(LC_ALL,"");
	QLocale lang(languageCode);
	/*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
	QLocale::setDefault(languageCode);
    }
 
    /*修改语言时卸载网络插件，此次不需要重新加载插件，点击网络按钮时会重新加载*/ 
    unloadNetPlugin();

    m_configuration->m_trans->load(qmFile);
    qApp->installTranslator(m_configuration->m_trans);

    m_greeter->setLang(languageCode);

}

void GreeterWindow::showSessionWnd()
{
    //会话列表
    if (!m_scrollAreaSession) {
        m_scrollAreaSession = new QScrollArea(this);
        m_scrollAreaSession->viewport()->setAttribute(Qt::WA_TranslucentBackground);
        m_scrollAreaSession->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //屏蔽水平滚动条
        m_scrollAreaSession->setStyleSheet("QScrollArea {background-color: rgba(255,255,255,20%);border-radius:10px;}");
        m_scrollAreaSession->viewport()->setStyleSheet("background-color:transparent;");

        m_scrollContentSession = new QWidget(m_scrollAreaSession);
        m_scrollContentSession->installEventFilter(this);
        m_scrollAreaSession->setWidget(m_scrollContentSession);
        m_scrollAreaSession->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
        m_scrollAreaSession->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
        m_sessionsWnd = new SessionsView(m_scrollContentSession);
        m_sessionsWnd->installEventFilter(this);
        connect(m_sessionsWnd, &SessionsView::currentSessionChanged, this, &GreeterWindow::onSessionChanged);
        m_sessionsWnd->setModel(m_sessionsModel);
        m_sessionsWnd->setCurrentSession(m_greeter->session());
        // session widget
        int y = 72;
        if (m_sessionsWnd)
            m_sessionsWnd->adjustSize();
        if (m_scrollContentSession)
            m_scrollContentSession->setFixedSize(m_sessionsWnd->width(),m_sessionsWnd->height());
        if(m_scrollAreaSession){
            if(m_scrollContentSession->height() < height()/2){
                m_scrollAreaSession->setFixedSize(m_sessionsWnd->width(),m_scrollContentSession->height()+1);
            }else{
                m_scrollAreaSession->setFixedSize(m_sessionsWnd->width() + 16,height()/2);
            }
            m_scrollAreaSession->move(m_sessionLB->geometry().x()- m_scrollAreaSession->width()/2 + 24, \
                        height() - y - m_scrollAreaSession->height() - 8);
        }
        m_scrollAreaSession->hide();
    }
    m_sessionHasChanged = true;

    tabAt = LINEEDIT;
    setBottomBtnSheet();
    tabAt = BOTTMBTN;
    horAT = SESSIONBTN;

    if(m_scrollAreaSession && m_scrollAreaSession->isVisible()){
        m_scrollAreaSession->hide();
        setCheckedSheet(SESSIONBTN,false);
        at_plugins = false;
        m_loginWnd->setFocusin(REMOVE);
    } else{
        if(m_kylinNM && m_kylinNM->isVisible()) {
            m_kylinNM->hide();
            setCheckedSheet(NETWORKBTN,false);
        }

        if(mBatteryWidget && mBatteryWidget->isVisible()){
            mBatteryWidget->hide();
            setCheckedSheet(BATTERYBTN,false);
        }

        if(scrollArea && scrollArea->isVisible()){
            scrollArea->hide();
            setCheckedSheet(SWITCHBTN,false);
        }

        m_scrollAreaSession->show();
        setCheckedSheet(SESSIONBTN,true);
        m_scrollAreaSession->raise();
        m_loginWnd->setFocusin(REMOVE);
        at_plugins = true;
    }
}

void GreeterWindow::onSessionChanged(const QString &session)
{
    qDebug() << "session changed: " << session;
    QString sessionTmp(session);
    if(session.isEmpty() || !sessionIsValid(session))
    {   //session不存在
        QString defaultSession = m_greeter->defaultSessionHint();

        //默认session存在
        if(defaultSession != session && sessionIsValid(defaultSession))
        {
            sessionTmp = defaultSession;
        }
        //列表中的第一个session
        else if(m_sessionsModel && m_sessionsModel->rowCount() > 0)
        {
            sessionTmp = m_sessionsModel->index(0, 0)
                    .data(QLightDM::SessionsModel::KeyRole).toString();
        }
        else
        {
            sessionTmp = "";
        }
    }
	
    QString sessionPrefix = sessionTmp.left(sessionTmp.indexOf('-'));
    if(sessionTmp == "ukui-wayland")
	    sessionPrefix = "ukui_wayland";
    QString sessionIcon = IMAGE_DIR + QString("badges/%1_badge.svg")
            .arg(sessionPrefix.toLower());
    QFile iconFile(sessionIcon);
    if(!iconFile.exists()){
        sessionIcon = IMAGE_DIR + QString("badges/unknown_badge.svg");
    }
    qDebug() << sessionIcon;

    if(m_sessionLB)
        m_sessionLB->setIcon(QIcon(sessionIcon));

    m_greeter->setSession(sessionTmp);

    if(m_userWnd && !m_userWnd->isHidden())
    {
        m_userWnd->setFocus();
    }

    QTimer::singleShot(150, this, [=]{
        setCheckedSheet(SESSIONBTN, false);
        at_plugins = false;
    });
}

bool GreeterWindow::sessionIsValid(const QString &session)
{
    for(int i = 0; i < m_sessionsModel->rowCount(); i++)
    {
        QString sessionKey = m_sessionsModel->index(i, 0)
                .data(QLightDM::SessionsModel::KeyRole).toString();
        if(sessionKey == session)
            return true;
    }
    return false;
}

void GreeterWindow::onAuthenticationComplete1()
{
    for(int i = 0;i<m_userWnd->userlist.count();i++)
    {
        UserEntry *entry = m_userWnd->userlist.at(i);
          if(entry->userIndex().data(QLightDM::UsersModel::NameRole).toString() == "*login")
               entry->setUserName(tr("Login"));
    }
}

void GreeterWindow::onServiceUserChanged(WillLoginUserInfo willSwitchUser)
{
    if(willSwitchUser.strUserName.isEmpty())
        return;
    static QString strLastUserName = "";
    static quint64 llLastChangeTick = 0;

    // 清空 willSwitchUser
    m_greeterServive->SwitchToUser("");

    quint64 curChangeTick = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if (willSwitchUser.strUserName == strLastUserName && curChangeTick - llLastChangeTick <= 2000) {
        qDebug()<<"onServiceUserChanged same name and time short:"<<willSwitchUser.strUserName<<","<<curChangeTick - llLastChangeTick;
        return ;
    }
    strLastUserName = willSwitchUser.strUserName;
    llLastChangeTick = curChangeTick;

    if (willSwitchUser.strUserName.isEmpty() || !m_greeterServive->isUserNameValid(willSwitchUser.strUserName)) {
        qDebug()<<"WillSwitchUser is NULL or invalid:"<<willSwitchUser.strUserName;
        return ;
    }
    if (!m_userWnd) {
        return ;
    }
    if (m_userWnd->getIsUser(willSwitchUser.strUserName)) {
        qDebug()<<"onServiceUserChanged:" << willSwitchUser.strUserName <<currentUser;
        if (m_loginWnd && m_loginWnd->checkAuthOK(willSwitchUser.strUserName)) {
            qDebug()<<"checkAuthOK:"<<willSwitchUser.strUserName;
            return;
        }
        m_userWnd->setCurrentUser(willSwitchUser.strUserName, true);
        if (m_loginWnd) {
            m_loginWnd->setWillLoginUser(willSwitchUser.strUserName, willSwitchUser.isOneKeyLogin);
        }
    } else if (m_userWnd->getIsUser("*login")) {
        // 手动输入登录
        qDebug()<<"onServiceUserChanged1:" << willSwitchUser.strUserName << currentUser;
        if (m_loginWnd && m_loginWnd->checkAuthOK(willSwitchUser.strUserName)) {
            qDebug()<<"checkAuthOK1:"<<willSwitchUser.strUserName;
            return;
        }
        m_userWnd->setCurrentUser("*login", true);
        if (m_loginWnd) {
            m_loginWnd->setWillLoginUser(willSwitchUser.strUserName, willSwitchUser.isOneKeyLogin);
        }
    }
}

void GreeterWindow::onCustomRequestAccount(QString account)
{
    if(m_userWnd)
        m_userWnd->setCurrentUser(account, true);
}

void GreeterWindow::onGetCustomPluginMsg(QString strMsg)
{
    QJsonParseError jsonParseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strMsg.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        qWarning()<<"Parse message json failed!!";
        return ;
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
             qWarning()<<"Message Json is null!!";
             return ;
        } else {
            QJsonObject contentObj = rootObj.value("Content").toObject();
            if (!contentObj.isEmpty()) {
                QJsonObject configObj = contentObj.value("Configures").toObject();
                if (!configObj.isEmpty()) {
                    m_isCustomDefault = configObj.value("DefaultAuth").toBool(false);
                    m_isShowNetwork = configObj.value("ShowNetwork").toBool(true);
                    m_isShowUserSwitch = configObj.value("ShowUserSwitch").toBool(true);
                }
            }
        }
    }
}

void GreeterWindow::key_tab_release(int key)
{
    if (at_plugins) {
        m_loginWnd->setFocusin(REMOVE);
        return;
    }
    switch (tabAt) {
    case LINEEDIT:
        m_loginWnd->setFocusin(IN_LIGIN);
        tabAt = ENTERBTN;
        break;
    case MESSAGEBTN :
        tabAt = BOTTMBTN;
        m_loginWnd->setFocusin(REMOVE);
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
    case ENTERBTN:
        if(m_loginWnd->getBioNum() >1) {
            m_loginWnd->setFocusin(BIO_RIGHT);
            tabAt = BIOBTN;
        } else {
            tabAt = BOTTMBTN;
            m_loginWnd->setFocusin(REMOVE);
            if (m_isSessionBtnShow) {
                horAT = SESSIONBTN;
                setBottomBtnSheet();
            } else if(is_batteryBtn) {
                horAT = BATTERYBTN;
                setBottomBtnSheet();
            } else if(m_usersModel->rowCount() > 1){
                horAT = SWITCHBTN;
                setBottomBtnSheet();
            } else {
                horAT =NETWORKBTN;
                setBottomBtnSheet();
            }
        }
        break;
    case BIOBTN:
        tabAt = BOTTMBTN;
        m_loginWnd->setFocusin(REMOVE);
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
    case BOTTMBTN:
        if(!at_power) {
            if(!m_loginWnd->getLineeditStatus()) {
                m_loginWnd->setFocusin(IN_LINEEDIT);
                tabAt = LINEEDIT;
            } else {
                m_loginWnd->setFocusin(ON_MESSAGEBTN);
                tabAt = MESSAGEBTN;
            }
            setBottomBtnSheet();
        } else if (at_power) {
            m_powerWnd->keyBdRelease(Qt::Key_Right);
            tabAt = POWERMANAGER;
            setBottomBtnSheet();
        }
        break;
    case POWERMANAGER:
        m_powerWnd->clearStatus();
        tabAt = BOTTMBTN;
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
    case EMPTY:
        m_loginWnd->setFocusin(IN_LINEEDIT);
        tabAt = LINEEDIT;
        setBottomBtnSheet();
    }
}

void GreeterWindow::key_shiftTab_release()
{
    if (at_plugins) {
        m_loginWnd->setFocusin(REMOVE);
        return;
    }
    switch (tabAt) {
    case LINEEDIT:
        tabAt = BOTTMBTN;
        m_loginWnd->setFocusin(REMOVE);
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
        m_loginWnd->setFocusin(IN_LIGIN);
        tabAt = ENTERBTN;
        break;
    case MESSAGEBTN :
        tabAt = BOTTMBTN;
        m_loginWnd->setFocusin(REMOVE);
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
    case ENTERBTN:
        tabAt = LINEEDIT;
        m_loginWnd->setFocusin(IN_LINEEDIT);
        break;
    case BIOBTN:
        m_loginWnd->setFocusin(IN_LIGIN);
        tabAt = ENTERBTN;
        setBottomBtnSheet();
        break;
    case BOTTMBTN:
        if(!at_power) {
            if(!m_loginWnd->getLineeditStatus()) {
                if(m_loginWnd->getBioNum() >1) {
                    m_loginWnd->setFocusin(BIO_RIGHT);
                    tabAt = BIOBTN;
                    setBottomBtnSheet();
                } else {
                    tabAt = ENTERBTN;
                    m_loginWnd->setFocusin(IN_LIGIN);
                    setBottomBtnSheet();
                }
            } else {
                m_loginWnd->setFocusin(ON_MESSAGEBTN);
                tabAt = MESSAGEBTN;
            }
        } else if (at_power) {
            m_powerWnd->keyBdRelease(Qt::Key_Right);
            tabAt = POWERMANAGER;
            setBottomBtnSheet();
        }
        break;
    case POWERMANAGER:
        m_powerWnd->clearStatus();
        tabAt = POWERBTN;
        if (m_isSessionBtnShow) {
            horAT = SESSIONBTN;
            setBottomBtnSheet();
        } else if(is_batteryBtn) {
            horAT = BATTERYBTN;
            setBottomBtnSheet();
        } else if(m_usersModel->rowCount() > 1){
            horAT = SWITCHBTN;
            setBottomBtnSheet();
        } else {
            horAT =NETWORKBTN;
            setBottomBtnSheet();
        }
        break;
    case EMPTY:
        m_loginWnd->setFocusin(IN_LINEEDIT);
        tabAt = LINEEDIT;
        setBottomBtnSheet();
    }
}

void GreeterWindow::key_enter_release(int key)
{
    if (tabAt == BOTTMBTN) {
        switch (horAT) {
        case BATTERYBTN:
            showBattery();
            m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case SWITCHBTN:
            if(scrollArea && scrollArea->isVisible()) {
                m_userWnd->keyBdRelease(key);
                showUserMenu();
                tabAt = LINEEDIT;
            } else {
                showUserMenu();
                m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            }
            break;
        case NETWORKBTN:
            if(!m_kylinNM || !(m_kylinNM && m_kylinNM->isVisible()))
                showNetManager(true);
	    widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_loginWnd->setFocusin(REMOVE);
            break;
        case KEYBOARDBTN:
            if(!m_virtualKeyboard || !(m_virtualKeyboard && m_virtualKeyboard->isVisible()))
                showVirtualKeyboard();
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case POWERBTN:
            showPowerWnd(true);
            break;
        case SESSIONBTN:
            if(m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
                m_sessionsWnd->keyBdRelease(key);
                showSessionWnd();
            } else {
                showSessionWnd();
                if (m_sessionLB)
                    m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            }
            break;
        default:
            break;
        }
    } else if (tabAt == BIOBTN) {
        m_loginWnd->setClick();
    } else if (tabAt == ENTERBTN) {
        m_loginWnd->checkPassword();
        tabAt = LINEEDIT;
    } else if (tabAt == POWERMANAGER) {
        m_powerWnd->keyBdRelease(key);
    }
}

void GreeterWindow::key_LR_release(int key)
{
    if(key == Qt::Key_Right) {
        if(tabAt == BOTTMBTN) {
            if (at_plugins)
                return;
            switch (horAT) {
            case SESSIONBTN:
                if (is_batteryBtn)
                    horAT = BATTERYBTN;
                else if (is_switchBtn)
                    horAT = SWITCHBTN;
                else
                    horAT = NETWORKBTN;
                break;
            case BATTERYBTN:
                if(is_switchBtn)
                    horAT = SWITCHBTN;
                else
                    horAT = NETWORKBTN;
                break;
            case SWITCHBTN:
                horAT = NETWORKBTN;
                break;
            case NETWORKBTN:
                horAT = KEYBOARDBTN;
                break;
            case KEYBOARDBTN:
                horAT = POWERBTN;
                break;
            case POWERBTN:
                if (m_isSessionBtnShow)
                    horAT = SESSIONBTN;
                else if(is_batteryBtn)
                    horAT = BATTERYBTN;
                else if(is_switchBtn)
                    horAT = SWITCHBTN;
                else
                    horAT = NETWORKBTN;
                break;
            default:
                break;
            }
            setBottomBtnSheet();
        } else if (tabAt == BIOBTN) {
            m_loginWnd->setFocusin(BIO_RIGHT);
        } else if (tabAt == POWERMANAGER) {
            m_powerWnd->keyBdRelease(key);
        }
    } else if (key == Qt::Key_Left) {
        if(tabAt == BOTTMBTN) {
            if (at_plugins)
                return;
            switch (horAT) {
            case SESSIONBTN:
                horAT = POWERBTN;
                break;
            case BATTERYBTN:
                if (m_isSessionBtnShow)
                    horAT = SESSIONBTN;
                else
                    horAT = POWERBTN;
                break;
            case SWITCHBTN:
                if(is_batteryBtn)
                    horAT = BATTERYBTN;
                else if (m_isSessionBtnShow)
                    horAT = SESSIONBTN;
                else
                    horAT = POWERBTN;
                break;
            case NETWORKBTN:
                if(is_switchBtn)
                    horAT = SWITCHBTN;
                else if(is_batteryBtn)
                    horAT = BATTERYBTN;
                else if (m_isSessionBtnShow)
                    horAT = SESSIONBTN;
                else
                    horAT = POWERBTN;;
                break;
            case KEYBOARDBTN:
                horAT = NETWORKBTN;
                break;
            case POWERBTN:
                horAT = KEYBOARDBTN;
                break;
            default:
                break;
            }
            setBottomBtnSheet();
        } else if (tabAt == BIOBTN) {
            m_loginWnd->setFocusin(BIO_LEFT);
        } else if (tabAt == POWERMANAGER) {
            m_powerWnd->keyBdRelease(key);
        }
    }
}

void GreeterWindow::key_OB_release(int key)
{
    if(scrollArea && scrollArea->isVisible()) {
        m_userWnd->keyBdRelease(key);
    } else if (m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
        m_sessionsWnd->keyBdRelease(key);
    }
}

void GreeterWindow::key_exit_release()
{
    if(mBatteryWidget && mBatteryWidget->isVisible()) {
        setCheckedSheet(BATTERYBTN, false);
        at_plugins = false;
        if(tabAt == BOTTMBTN && horAT == BATTERYBTN)
            m_batteryiconLB->setStyleSheet(ON_TAB_SHEET);
    }
    if (m_scrollAreaSession && m_scrollAreaSession->isVisible()) {
        setCheckedSheet(SESSIONBTN, false);
        at_plugins = false;
        if(tabAt == BOTTMBTN && horAT == SESSIONBTN && m_sessionLB)
            m_sessionLB->setStyleSheet(ON_TAB_SHEET);
    }
    if(scrollArea && scrollArea->isVisible()){
        setCheckedSheet(SWITCHBTN, false);
        at_plugins = false;
        if(tabAt == BOTTMBTN && horAT == SWITCHBTN && m_btnSwitchUser)
            m_btnSwitchUser->setStyleSheet(ON_TAB_SHEET);
    } else if(m_kylinNM && m_kylinNM->isVisible()){
        setCheckedSheet(NETWORKBTN, false);
        at_plugins = false;
        if(tabAt == BOTTMBTN && horAT == NETWORKBTN)
            widgetNetworkManager->setStyleSheet(ON_TAB_SHEET_WIDGET);
    } else if(m_virtualKeyboard && m_virtualKeyboard->isVisible()) {
        m_virtualKeyboard->hide();
        at_plugins = false;
        if(tabAt == BOTTMBTN && horAT == KEYBOARDBTN)
            m_keyboardLB->setStyleSheet(ON_TAB_SHEET);
    } else if(m_powerWnd && m_powerWnd->isVisible()){
        m_powerWnd->close();
        m_loginWnd->show();
        if(!m_loginWnd->getLineeditStatus()) {
            tabAt = LINEEDIT;
            m_loginWnd->setFocusin(IN_LINEEDIT);
            if(isTableMode && !(m_virtualKeyboard && m_virtualKeyboard->isVisible())) {
                showVirtualKeyboard();
            }
        } else {
            tabAt = MESSAGEBTN;
            m_loginWnd->setFocusin(ON_MESSAGEBTN);
        }
        setBottomBtnSheet();
        at_power = false;
        update();
        return ;
    }
    at_plugins = false;
    if(tabAt != LINEEDIT && tabAt != ENTERBTN && tabAt != BIOBTN && tabAt != MESSAGEBTN)
        m_loginWnd->setFocusin(REMOVE);
}

void GreeterWindow::setBottomBtnSheet()
{
    if (tabAt == BOTTMBTN) {
        switch (horAT) {
        case SWITCHBTN:
            m_btnSwitchUser->setStyleSheet(ON_TAB_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case NETWORKBTN:
            widgetNetworkManager->setStyleSheet(ON_TAB_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case KEYBOARDBTN:
            m_keyboardLB->setStyleSheet(ON_TAB_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case POWERBTN:
            m_powerLB->setStyleSheet(ON_TAB_SHEET);
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case BATTERYBTN:
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_TAB_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        case SESSIONBTN:
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_TAB_SHEET);
            break;
        default:
            m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
            widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
            m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
            m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_batteryiconLB)
                m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
            if (m_sessionLB)
                m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
            break;
        }
    } else {
        m_btnSwitchUser->setStyleSheet(ON_NORMAL_SHEET);
        widgetNetworkManager->setStyleSheet(ON_NORMAL_SHEET_WIDGET);
        m_keyboardLB->setStyleSheet(ON_NORMAL_SHEET);
        m_powerLB->setStyleSheet(ON_NORMAL_SHEET);
        if (m_batteryiconLB)
            m_batteryiconLB->setStyleSheet(ON_NORMAL_SHEET);
        if (m_sessionLB)
            m_sessionLB->setStyleSheet(ON_NORMAL_SHEET);
    }
}

void GreeterWindow::setCheckedSheet(int type, bool show)
{
    switch (type) {
    case SWITCHBTN:
        if(show) {
            if (scrollArea)
                scrollArea->show();
            if (m_btnSwitchUser)
                m_btnSwitchUser->setChecked(true);
        } else {
            if (scrollArea)
                scrollArea->hide();
            if (m_btnSwitchUser)
                m_btnSwitchUser->setChecked(false);
        }
        break;
    case NETWORKBTN:
        if(show) {
            if (m_kylinNM)
                m_kylinNM->show();
            if (m_nmLB)
                m_nmLB->setChecked(true);
        } else {
            if (m_kylinNM)
                m_kylinNM->hide();
            if (m_nmLB)
                m_nmLB->setChecked(false);
        }
        break;
    case BATTERYBTN:
        if(show) {
            if (mBatteryWidget)
                mBatteryWidget->show();
            if (m_batteryiconLB)
                m_batteryiconLB->setChecked(true);
        } else {
            if (mBatteryWidget)
                mBatteryWidget->hide();
            if (m_batteryiconLB)
                m_batteryiconLB->setChecked(false);
        }
        break;
    case SESSIONBTN:
        if(show) {
            if (m_scrollAreaSession) {
                m_scrollAreaSession->show();
                m_scrollAreaSession->raise();
            } else {
                showSessionWnd();
            }

            if (m_sessionLB)
                m_sessionLB->setChecked(true);
        } else {
            if (m_scrollAreaSession) {
                m_scrollAreaSession->hide();
                setFocus();
            }
            if (m_sessionLB)
                m_sessionLB->setChecked(false);
        }
        break;
    default:
        break;
    }
}
