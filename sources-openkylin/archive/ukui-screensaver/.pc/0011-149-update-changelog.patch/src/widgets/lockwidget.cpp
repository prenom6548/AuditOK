/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "lockwidget.h"
#include <QPixmap>
#include <QPainter>
#include <QScreen>
#include <QPainterPath>
#include <QDebug>
#include <QProcess>
#include <QKeyEvent>
#include <QListView>
#include <QListWidget>
#include <QPalette>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QHBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include "poweritemwidget.h"
#include "useritemwidget.h"
#include "mylistwidget.h"
#include "userlistwidget.h"
#include "sessionlistwidget.h"
#include "statusbutton.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "iconedit.h"
#include "commonfunc.h"
#include "authdialog.h"
#include "powerlistwidget.h"
#include "timewidget.h"
#include "networkbutton.h"
#include "definetypes.h"
#include "mynetworkwidget.h"
#include "batterywidget.h"
#include "VirtualKeyboard/src/virtualkeyboardwidget.h"
#include "blockwidget.h"
#include "pluginsloader.h"
#include "global_utils.h"
#include "../lock-dialog/languagesetting.h"
#include "../lock-dialog/rootWindowBackground.h"
#include "configuration.h"
#include "utils.h"
#include "resetpwdwidget.h"
#include <syslog.h>

#define BLUR_RADIUS 300
#define RIGHT_MARGIN 24
#define BOTTOM_MARGIN 24

#define PREFIX QString(RESOURCE_PATH)
#define IMAGE_DIR PREFIX + "/images/"
float scale;
LockWidget::LockWidget(LockDialogModel *model, QWidget *parent)
    : QWidget(parent), m_modelLockDialog(model), m_pSecurityQuestionAnswer(new SecurityQuestionAnswer(this))
{
    scale = 1.0;
    m_curUserInfo = m_modelLockDialog->findUserByName(m_modelLockDialog->defaultUserName());
    initUI();
    initConnections();
    initUsdMediaKeys();
    installEventFilter(this);
}

bool LockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        MyListWidget *listWidget = qobject_cast<MyListWidget *>(obj);
        if (listWidget) {
            if (listWidget && listWidget->hasFocus()) { // Tab键切入时，设置焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::SelectCurrent);
                listWidget->currentItemChanged(listWidget->currentItem(), nullptr);
            } else { // Tab键切出时，清空焦点状态
                listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::Clear);
            }
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        /// TODO
        if ((m_networkWidget && m_networkWidget == obj) || (m_userListWidget && m_userListWidget == obj)
            || (m_sessionListWidget && m_sessionListWidget == obj)
            || (m_virtualKeyboardWidget && m_virtualKeyboardWidget == obj)) {
            return true;
        } else if (buttonListWidget && obj != buttonListWidget) {
            exitSubWidget(false, false);
        }
    } else if (event->type() == QEvent::Show) {
        if (obj == this) {
            if (m_mediaControlWidget) {
                m_mediaControlWidget->initMultimediaControl();
            }
        }
    } else if (event->type() == QEvent::Hide) {
        if (obj == this) {
            if (authDialog && !authDialog->isHidden()) {
                authDialog->hide();
            }
        }
    }

    return false;
}

bool LockWidget::exitSubWidget(bool isForScreensaver, bool hideVirkeyboard)
{
    if (m_isSessionTools) {
        authDialog->setFocus();
        authDialog->inhibitShow(false);
        buttonListWidget->show();
        m_timeWidget->show();
        m_powerListWidget->hide();
        if (m_systemMonitorBtn && m_systemMonitorBtn->isVisible()) {
            m_systemMonitorBtn->hide();
        }
        m_blockWidget->hide();
        m_powerManagerButton->setClickedStatus(NORMAL);
        if (!isForScreensaver) {
            Q_EMIT sessionToolsExit();
        }
        m_isSessionTools = false;
        if (m_widgetMediaControl) {
            m_widgetMediaControl->show();
        }
        return false;
    }
    bool allExited = true;
    if (m_userListWidget && !m_userListWidget->isHidden()) {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
        allExited = false;
    }
    if (m_powerListWidget && !m_powerListWidget->isHidden()) {
        authDialog->setFocus();
        authDialog->inhibitShow(false);
        if (m_isShowResetPasswdBtn) {
            m_pResetPwdBtn->show();
        }
        if (m_widgetMediaControl) {
            m_widgetMediaControl->show();
        }
        m_powerListWidget->hide();
        buttonListWidget->show();
        if (m_systemMonitorBtn && !m_systemMonitorBtn->isHidden()) {
            m_systemMonitorBtn->hide();
        }
        m_powerManagerButton->setClickedStatus(NORMAL);
        allExited = false;
    }
    if (m_networkWidget && !m_networkWidget->isHidden()) {
        m_networkWidget->hide();
        m_networkButton->setClickedStatus(NORMAL);
        //        sendNetPluginVisible(false);
        allExited = false;
    }
    if (hideVirkeyboard && m_virtualKeyboardWidget && !m_virtualKeyboardWidget->isHidden()) {
        m_virtualKeyboardWidget->hide();
        m_virKbButton->setClickedStatus(NORMAL);
        allExited = false;
    }
    if (batteryWidget && !batteryWidget->isHidden()) {
        batteryWidget->hide();
        m_batteryonButton->setClickedStatus(NORMAL);
        allExited = false;
    }
    if (m_sessionListWidget && !m_sessionListWidget->isHidden()) {
        m_sessionListWidget->hide();
        m_sessionButton->setClickedStatus(NORMAL);
        allExited = false;
    }
    if (m_blockWidget && !m_blockWidget->isHidden()) {
        m_blockWidget->hide();
        m_timeWidget->show();
        m_powerListWidget->show();
        buttonListWidget->show();
        allExited = false;
    }
    if (m_pResetPwdWidget && m_pResetPwdWidget->isHidden()) {
        m_pResetPwdWidget->hide();
        m_pResetPwdWidget->switchToConfidentialDig();
        authDialog->setFocus();
        authDialog->inhibitShow(false);
        if (m_pResetPwdBtn) {
            m_pResetPwdBtn->show();
        }
        allExited = false;
    }
    authDialog->setFocus();

    return allExited;
}

void LockWidget::paintEvent(QPaintEvent *event)
{
    return QWidget::paintEvent(event);
}

void LockWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    // 重新计算缩放比例
    scale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    authDialog->setGeometry(
        (width() - this->geometry().width()) / 2,
        148 * scale,
        this->width(),
        (height() - 150 * scale - (24 + 48 + 16) * scale));
    authDialog->updateAuthSize();

    m_timeWidget->updateUI();
    m_timeWidget->move((width() - m_timeWidget->width()) / 2, 48 * scale);
    if (m_userListWidget) {
        m_userListWidget->setMaxHeight(height() / 2);
        m_userListWidget->updateWidgetSize();
        qDebug() << "m_userListWidget:" << m_userListWidget->geometry();
    }
    m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height() + m_systemMonitorBtn->height()));
    m_powerListWidget->updateWidgetSize();

    if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
        m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
    } else { // 居中显示
        m_powerListWidget->move(
            (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
    }

    updateBottomButton();
    if (m_curUserInfo->isLoggedIn() && m_sessionButton) {
        buttonListWidget->setFixedSize(QSize(
            48 * (buttonListWidget->count() - 1) + buttonListWidget->spacing() * 2 * (buttonListWidget->count() - 1),
            64));
    } else {
        buttonListWidget->setFixedSize(
            QSize(48 * buttonListWidget->count() + buttonListWidget->spacing() * 2 * buttonListWidget->count(), 64));
    }
    m_pBottomWidget->setGeometry(0, height() - buttonListWidget->height() - BOTTOM_MARGIN * scale, this->width(), 64);

    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->setGeometry(
            (this->width() - m_pResetPwdBtn->width()) / 2,
            height() - m_pResetPwdBtn->height() - BOTTOM_MARGIN * scale + 8,
            m_pResetPwdBtn->width(),
            m_pResetPwdBtn->height());
    }

    if (m_sessionListWidget) {
        m_sessionListWidget->setMaxHeight(height() / 2);
        m_sessionListWidget->updateWidgetSize();
        setSessionListWidgetLocate();
    }

    if (m_networkWidget) {
        setNetWorkWidgetLocate();
    }
    setBatteryWidgetLocate();

    if (m_userListWidget) {
        setUserListWidgetLocate();
    }

    if (m_systemMonitorBtn) {
        m_systemMonitorBtn->move(
            (width() - m_systemMonitorBtn->width()) / 2, height() - m_systemMonitorBtn->height() - 40);
    }
    onSetVirkeyboardPos();
}

void LockWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_K) {
        if (e->modifiers() & Qt::ControlModifier)
            onShowVirtualKeyboard();
    }

    if (e->key() == Qt::Key_Return && m_systemMonitorBtn->hasFocus()) {
        doSystemMonitor();
        exitSubWidget();
    }

    // 监听按键事件实现快捷键功能
    switch (e->key()) {
        case Qt::Key_VolumeMute:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(MUTE_KEY);
            break;
        case Qt::Key_VolumeDown:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(VOLUME_DOWN_KEY);
            break;
        case Qt::Key_VolumeUp:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(VOLUME_UP_KEY);
            break;
        case Qt::Key_MonBrightnessUp:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(BRIGHT_UP_KEY);
            break;
        case Qt::Key_MonBrightnessDown:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(BRIGHT_DOWN_KEY);
            break;
        case Qt::Key_TouchpadOn:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(TOUCHPAD_ON_KEY);
            break;
        case Qt::Key_TouchpadOff:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(TOUCHPAD_OFF_KEY);
            break;
        default:
            break;
    }

    //  实现锁屏截图功能
    QKeySequence keySequence = QKeySequence(e->modifiers() + e->key());
    if (keySequence == listFromString(m_areaScreenShot)) {
        Q_EMIT m_modelLockDialog->usdExternalDoAction(AREA_SCREENSHOT_KEY);
    } else if (keySequence == listFromString(m_areaScreenShot2)) {
        Q_EMIT m_modelLockDialog->usdExternalDoAction(AREA_SCREENSHOT_KEY);
    } else if (keySequence == listFromString(m_screenShot)) {
        Q_EMIT m_modelLockDialog->usdExternalDoAction(SCREENSHOT_KEY);
    } else if (keySequence == listFromString(m_screenShot2)) {
        Q_EMIT m_modelLockDialog->usdExternalDoAction(SCREENSHOT_KEY);
    } else if (keySequence == listFromString(m_windowScreenshot)) {
        Q_EMIT m_modelLockDialog->usdExternalDoAction(WINDOW_SCREENSHOT_KEY);
    }
    QWidget::keyReleaseEvent(e);
}

void LockWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}

void LockWidget::initUI()
{
    // 检查并获取第三方认证插件配置信息
    if (PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).size() > 0) {
        LoginAuthInterface *plugin = dynamic_cast<LoginAuthInterface *>(
            PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).values().first());
        QJsonObject rootObj;
        rootObj["MsgType"] = "GetInformations";
        rootObj["Key"] = "Configures";
        QString strMsg = plugin->onMessage(QJsonDocument(rootObj).toJson());
        onGetCustomPluginMsg(strMsg);
    }

    authDialog = new AuthDialog(m_modelLockDialog, m_curUserInfo, this);
    authDialog->installEventFilter(this);
    authDialog->hide();
    m_curFont = m_modelLockDialog->getCurFont();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    m_curFontSize = m_modelLockDialog->getCurFontSize();
    QLocale local;
    systemLang = local.name();
    initTimeWidget();
    initPowerWidget();
    initButtonWidget();
    initBlockWidget();
    initSystemMonitorBtn();
    initMediaControlWidget();
    //    authDialog->setFocus();
    //    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
}

void LockWidget::initConnections()
{
    connect(UniAuthService::instance(), &UniAuthService::userChanged, this, &LockWidget::onServiceUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::usersInfoChanged, this, &LockWidget::onUsersInfoChanged);
    connect(m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &LockWidget::onCurUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::currentSessionChanged, this, &LockWidget::onSessionChanged);
    connect(m_modelLockDialog, &LockDialogModel::sessionActiveChanged, this, &LockWidget::onSessionActiveChanged);
    connect(m_modelLockDialog, &LockDialogModel::tabletModeChanged, this, &LockWidget::onTabletModeChanged);
    connect(m_modelLockDialog, &LockDialogModel::fontChanged, this, &LockWidget::onFontChanged);
    connect(m_modelLockDialog, &LockDialogModel::fontSizeChanged, this, &LockWidget::onFontSizeChanged);
    if (authDialog) {
        connect(authDialog, SIGNAL(authSucceed(QString)), this, SIGNAL(authSucceed(QString)));
        connect(authDialog, &AuthDialog::authSucceed, this, [=] {
            if (m_isShowResetPasswdBtn) {
                m_pResetPwdBtn->hide();
                m_isShowResetPasswdBtn = false;
            }
        });
        connect(authDialog, &AuthDialog::userChangedByManual, this, &LockWidget::onUserChangedByManual);
        connect(authDialog, &AuthDialog::customRequestAccount, this, &LockWidget::onCustomRequestAccount);
        connect(authDialog, &AuthDialog::lineEditClicked, this, [=] { onLineEditClicked(); });
        connect(authDialog, &AuthDialog::bottomPositionChanged, this, &LockWidget::onAuthDialogBottomPosChanged);
        connect(authDialog, &AuthDialog::showResetPasswdBtn, this, &LockWidget::onShowResetPasswdBtn);
    }
    connect(LanguageSetting::instance(this), &LanguageSetting::languageChanged, this, &LockWidget::onLanguageChanged);
}

void LockWidget::initTimeWidget()
{
    m_timeWidget = new TimeWidget(m_modelLockDialog, this);
    m_timeWidget->move((width() - m_timeWidget->width()) / 2, 0);
}

void LockWidget::initUserWidget()
{
    if (!m_userListWidget) {
        m_userListWidget = new UserListWidget(m_modelLockDialog, this);
        m_userListWidget->installEventFilter(this);
        m_userListWidget->setMaxHeight(height() / 2);
        m_userListWidget->updateWidgetSize();
        m_userListWidget->hide();
        connect(m_userListWidget, &UserListWidget::userSelected, m_modelLockDialog, [=](QString strUserName) {
            exitSubWidget(false, false);
            SwitchToUser(strUserName);
        });
        connect(m_userListWidget, &UserListWidget::widgetSizeChanged, m_modelLockDialog, [=]() {
            setUserListWidgetLocate();
        });
        m_userListWidget->clearFocus();
        setUserListWidgetLocate();
    }
}

void LockWidget::initSessionWidget()
{
    if (!m_sessionListWidget) {
        m_sessionListWidget = new SessionListWidget(m_modelLockDialog, this);
        m_sessionListWidget->installEventFilter(this);
        m_sessionListWidget->setMaxHeight(height() / 2);
        m_sessionListWidget->updateWidgetSize();
        m_sessionListWidget->hide();
        connect(
            m_sessionListWidget,
            &SessionListWidget::sessionSelected,
            m_modelLockDialog,
            &LockDialogModel::setCurrentSession);
        m_sessionListWidget->clearFocus();
        setSessionListWidgetLocate();
    }
}

void LockWidget::initPowerWidget()
{
    m_powerListWidget = new PowerListWidget(m_modelLockDialog, this);
    m_powerListWidget->move(
        (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2 - 80);
    m_powerListWidget->hide();
    connect(m_powerListWidget, &MyListWidget::currentItemChanged, this, &LockWidget::onPowerItemChanged);
    connect(m_powerListWidget, &PowerListWidget::showInhibitWarning, this, &LockWidget::onShowInhibitWarning);
    connect(m_powerListWidget, &PowerListWidget::mulUsersLogined, this, &LockWidget::onMulUsersLogined);
    connect(m_powerListWidget, &PowerListWidget::sureShutDown, this, &LockWidget::onSureShutDown);
    connect(m_powerListWidget, &PowerListWidget::lockScreenClicked, this, &LockWidget::onLockScreenClicked);
    connect(m_powerListWidget, &PowerListWidget::suspendClicked, this, &LockWidget::onSuspendClicked);
    connect(m_powerListWidget, &PowerListWidget::itemClicked, this, &LockWidget::onPowerItemClicked);
    connect(m_powerListWidget, &PowerListWidget::switchuserClicked, this, &LockWidget::onSwitchuserClicked);
    connect(m_powerListWidget, &PowerListWidget::powerWidgetClicked, this, [=] {
        exitSubWidget();
        Q_EMIT sessionToolsExit();
        m_isSessionTools = false;
    });
    //    m_powerListWidget->clearFocus();
}

void LockWidget::initButtonWidget()
{
#ifdef USDFIXED
    initUsdMediaStateKeys();
#endif
    m_pBottomWidget = new QWidget(this);
    QHBoxLayout *bottomWidgetLayout = new QHBoxLayout(m_pBottomWidget);
    bottomWidgetLayout->setContentsMargins(RIGHT_MARGIN * scale - 16, 0, RIGHT_MARGIN * scale - 16, 0);
    buttonListWidget = new MyListWidget(m_pBottomWidget);
    buttonListWidget->installEventFilter(this);
    buttonListWidget->setFlow(QListWidget::LeftToRight);
    buttonListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    buttonListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString buttonListStyle = "QListWidget{ background:rgba(255, 255, 255, 0%); border: 0px;}"
                              "QListWidget::item{border-radius: 4px; background:rgba(255, 255, 255, 0%);}";
    buttonListWidget->setStyleSheet(buttonListStyle);
    buttonListWidget->setSpacing(8);

    if (m_modelLockDialog->sessionsInfo().count() > 1) {
        m_sessionButton = new StatusButton(this, BOTBUTTON);
        m_btnItemSession = new QListWidgetItem();
        m_btnItemSession->setSizeHint(QSize(48, 48));
        buttonListWidget->insertItem(buttonListWidget->count(), m_btnItemSession);
        buttonListWidget->setItemWidget(m_btnItemSession, m_sessionButton);
        buttonListWidget->addItem(m_btnItemSession);
        m_sessionButton->setObjectName("BtnSession");
        if (m_curUserInfo->isLoggedIn()) {
            buttonListWidget->setItemHidden(m_btnItemSession, true);
        } else {
            buttonListWidget->setItemHidden(m_btnItemSession, false);
        }
        QString strCurSession = m_modelLockDialog->currentSession();
        QString sessionPrefix = strCurSession.left(strCurSession.indexOf('-'));
        if (strCurSession == "ukui-wayland")
            sessionPrefix = "ukui_wayland";
        if (strCurSession == "kylin-wlcom")
            sessionPrefix = "kylin_wlcom";
        strCurSession = IMAGE_DIR + QString("badges/%1_badge.svg").arg(sessionPrefix.toLower());
        QFile iconFile(strCurSession);
        if (!iconFile.exists()) {
            strCurSession = IMAGE_DIR + QString("badges/unknown_badge.svg");
        }
        QPixmap ssIcon(strCurSession);
        ssIcon = scaledPixmap(ssIcon);
        ssIcon = ssIcon.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ssIcon = PixmapToRound(ssIcon, 24);
        m_sessionButton->setIcon(ssIcon);
        m_sessionButton->setToolTip(tr("SwitchSession"));
        connect(m_sessionButton, &StatusButton::clicked, this, [this]() { onShowSessionListWidget(); });
    }

    if (m_modelLockDialog->getIsBattery()) {
        m_batteryonButton = new StatusButton(this, BOTBUTTON);
        QListWidgetItem *buttonItem = new QListWidgetItem();
        buttonItem->setSizeHint(QSize(48, 48));
        buttonListWidget->insertItem(buttonListWidget->count(), buttonItem);
        buttonListWidget->setItemWidget(buttonItem, m_batteryonButton);
        QPixmap batteryIcon = drawSymbolicColoredPixmap(
            QIcon::fromTheme(m_modelLockDialog->getBatteryIconName()).pixmap(QSize(24, 24)), "white");
        m_batteryonButton->setIcon(batteryIcon);
        connect(m_modelLockDialog, &LockDialogModel::batteryStatusChanged, this, [=](const QString &iconName) {
            QPixmap batteryIcon = drawSymbolicColoredPixmap(QIcon::fromTheme(iconName).pixmap(QSize(24, 24)), "white");
            m_batteryonButton->setIcon(batteryIcon);
        });
        buttonListWidget->addItem(buttonItem);
        m_batteryonButton->setToolTip(tr("Power Information"));
        connect(m_batteryonButton, &StatusButton::clicked, this, [this]() { onShowBatteryWidget(); });
    }

    m_userButton = new StatusButton(this, BOTBUTTON);
    m_btnItemUser = new QListWidgetItem();
    m_btnItemUser->setSizeHint(QSize(48, 48));
    buttonListWidget->insertItem(buttonListWidget->count(), m_btnItemUser);
    buttonListWidget->setItemWidget(m_btnItemUser, m_userButton);
    m_userButton->setIcon(drawSymbolicColoredPixmap(
        QIcon::fromTheme("ukui-stock-people-symbolic", QIcon(":/image/assets/switchuser.svg")).pixmap(24, 24),
        "white"));
    buttonListWidget->addItem(m_btnItemUser);
    m_userButton->setToolTip(tr("SwitchUser"));
    connect(m_userButton, &StatusButton::clicked, this, [this]() { onShowUserListWidget(); });
    if (m_modelLockDialog->normalUserCount() > 1 || m_modelLockDialog->findUserByName("*login")) {
        buttonListWidget->setItemHidden(m_btnItemUser, false);
    } else {
        buttonListWidget->setItemHidden(m_btnItemUser, true);
    }

#ifdef USDFIXED
    if (m_rfkillState != -1) {
        m_pRfkillStatusButton = new StatusButton(this, BOTBUTTON);
        QListWidgetItem *rekillButtonItem = new QListWidgetItem();
        rekillButtonItem->setSizeHint(QSize(48, 48));
        buttonListWidget->insertItem(buttonListWidget->count(), rekillButtonItem);
        buttonListWidget->setItemWidget(rekillButtonItem, m_pRfkillStatusButton);
        setRfkillBtnIcon();
        buttonListWidget->addItem(rekillButtonItem);
        connect(m_pRfkillStatusButton, &StatusButton::clicked, this, [this]() {
            setRfkillState();
            //            setRfkillBtnIcon();
        });
    }
#endif
    syslog(LOG_INFO, "[ukui-screensaver-dialog] init net icon begin!!");
    m_networkButton = new NetWorkButton(this);
    QListWidgetItem *buttonItem = new QListWidgetItem();
    buttonItem->setSizeHint(QSize(48, 48));
    buttonListWidget->insertItem(buttonListWidget->count(), buttonItem);
    buttonListWidget->setItemWidget(buttonItem, m_networkButton);
    buttonListWidget->addItem(buttonItem);
    m_networkButton->setToolTip(tr("Network"));
    connect(m_networkButton, &NetWorkButton::clicked, this, [this]() { onShowNetworkWidget(); });
    syslog(LOG_INFO, "[ukui-screensaver-dialog] init net icon end!!");

    /*
    connect(buttonWidget, &KyNetworkIcon::updateIconHidden
            ,this, [buttonWidget,this](bool status){
        if (status) {
            buttonWidget->hide();
        } else {
            buttonWidget->show();
        }
    });
    if (buttonWidget->checkIconIsHidden()) {
        buttonWidget->hide();
    }
    */

    m_virKbButton = new StatusButton(this, BOTBUTTON);
    QListWidgetItem *kyeboardItem = new QListWidgetItem();
    kyeboardItem->setSizeHint(QSize(48, 48));
    buttonListWidget->insertItem(buttonListWidget->count(), kyeboardItem);
    buttonListWidget->setItemWidget(kyeboardItem, m_virKbButton);
    m_virKbButton->setIcon(QIcon(":/image/assets/keyboard.svg"));
    buttonListWidget->addItem(kyeboardItem);
    m_virKbButton->setToolTip(tr("VirtualKeyboard"));
    connect(m_virKbButton, &StatusButton::clicked, this, [this]() { onShowVirtualKeyboard(); });
    onShowVirtualKeyboard(m_modelLockDialog->getTabletMode());

    //    if (m_powerListWidget->count() > 1 ) {
    m_powerManagerButton = new StatusButton(this, BOTBUTTON);
    QListWidgetItem *powerButtonItem = new QListWidgetItem();
    powerButtonItem->setSizeHint(QSize(48, 48));
    buttonListWidget->insertItem(buttonListWidget->count(), powerButtonItem);
    buttonListWidget->setItemWidget(powerButtonItem, m_powerManagerButton);
    m_powerManagerButton->setIcon(drawSymbolicColoredPixmap(
        QIcon::fromTheme("system-shutdown-symbolic", QIcon(":/image/assets/shutdown.svg")).pixmap(24, 24), "white"));
    buttonListWidget->addItem(powerButtonItem);
    m_powerManagerButton->setToolTip(tr("Power"));
    connect(m_powerManagerButton, &StatusButton::clicked, this, [this]() { onShowPowerListWidget(); });
    //    }

    if (m_curUserInfo->isLoggedIn() && m_sessionButton) {
        buttonListWidget->setFixedSize(QSize(
            48 * (buttonListWidget->count() - 1) + buttonListWidget->spacing() * 2 * (buttonListWidget->count() - 1),
            64));
    } else {
        buttonListWidget->setFixedSize(
            QSize(48 * buttonListWidget->count() + buttonListWidget->spacing() * 2 * buttonListWidget->count(), 64));
    }
    bottomWidgetLayout->addStretch();
    bottomWidgetLayout->addWidget(buttonListWidget);
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (m_pBottomWidget)
            m_pBottomWidget->setLayoutDirection(Qt::RightToLeft);
    } else {
        if (m_pBottomWidget)
            m_pBottomWidget->setLayoutDirection(Qt::LeftToRight);
    }
    connect(buttonListWidget, &MyListWidget::currentItemChanged, this, &LockWidget::onButtonItemChanged);
    connect(buttonListWidget, &MyListWidget::itemClicked, this, &LockWidget::onButtonItemClicked);
    buttonListWidget->clearFocus();
}

void LockWidget::initBlockWidget()
{
    m_blockWidget = new BlockWidget(m_modelLockDialog, this);
    m_blockWidget->setGeometry(this->geometry());
    m_blockWidget->hide();
    connect(m_blockWidget, SIGNAL(cancelButtonclicked()), this, SLOT(onCancelBtnClicked()));
    connect(m_blockWidget, SIGNAL(confirmButtonclicked()), this, SLOT(onConfirmBtnClicked()));
}

void LockWidget::initSystemMonitorBtn()
{
    m_systemMonitorBtn = new StatusButton(this, NORMALBTN);
    QFont sysBtnFont;
    sysBtnFont.setFamily(m_curFont);
    sysBtnFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_systemMonitorBtn->setFont(sysBtnFont);
    m_systemMonitorBtn->hide();
    m_systemMonitorBtn->setFixedHeight(48);
    m_systemMonitorBtn->setText(tr("system-monitor"));
    m_systemMonitorBtn->setContentsMargins(5, 0, 5, 0);
    m_systemMonitorBtn->adjustSize();
    connect(m_systemMonitorBtn, &StatusButton::clicked, this, &LockWidget::doSystemMonitor);
}

void LockWidget::initUsdMediaKeys()
{
    m_areaScreenShot = m_modelLockDialog->getUsdAreaScreenShotKey();
    m_areaScreenShot2 = m_modelLockDialog->getUsdAreaScreenShot2Key();
    m_screenShot = m_modelLockDialog->getUsdScreenShotKey();
    m_screenShot2 = m_modelLockDialog->getUsdScreenShot2Key();
    m_windowScreenshot = m_modelLockDialog->getUsdwindowScreenshotKey();
    connect(m_modelLockDialog, &LockDialogModel::usdMediaKeysChanged, this, &LockWidget::onUsdMediaKeysChanged);
}

void LockWidget::initUsdMediaStateKeys()
{
    m_rfkillState = m_modelLockDialog->getUsdMediaRfkillState();
    connect(
        m_modelLockDialog, &LockDialogModel::usdMediaStateKeysChanged, this, &LockWidget::onUsdMediaStateKeysChanged);
}

const QKeySequence LockWidget::listFromString(QString keys)
{
    keys.replace("<", "");
    keys.replace(">", "+");
    keys.replace("Win", "Meta", Qt::CaseInsensitive);
    keys.replace("Start", "Meta", Qt::CaseInsensitive);
    keys.replace("PrtSc", "Print", Qt::CaseInsensitive);
    return QKeySequence(keys);
}

void LockWidget::updateBottomButton()
{
    QList<StatusButton *> statusButton = this->findChildren<StatusButton *>();
    for (auto button : statusButton) {
        if (button->getButtonType() == POWERBUTTON) {
            button->setFixedSize(130, 130);
            button->setIconSize(QSize(130, 130));
        } else if (button->getButtonType() == PWDTIPBTN) {
            button->setFixedSize(40, 40);
            button->setIconSize(QSize(24, 24));
        } else if (button->getButtonType() != NORMALBTN) {
            button->setFixedSize(48, 48);
            button->setIconSize(QSize(24, 24));
        }
    }
    QList<NetWorkButton *> networkButton = this->findChildren<NetWorkButton *>();
    for (auto button : networkButton) {
        button->setFixedSize(48, 48);
        button->setIconSize(QSize(24, 24));
    }
}

void LockWidget::onPowerItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QWidget *currentWidget = m_powerListWidget->itemWidget(current);
    PowerItemWidget *currentItem = dynamic_cast<PowerItemWidget *>(currentWidget);
    if (currentItem) {
        currentItem->setSelected(SELECT);
    }
    if (previous) {
        QWidget *previousWidget = m_powerListWidget->itemWidget(previous);
        PowerItemWidget *previousItem = dynamic_cast<PowerItemWidget *>(previousWidget);
        if (previousItem) {
            previousItem->setSelected(NORMAL);
        }
    }
}

void LockWidget::onButtonItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QWidget *currentWidget = buttonListWidget->itemWidget(current);
    StatusButton *currentItem = dynamic_cast<StatusButton *>(currentWidget);
    NetWorkButton *netItem = dynamic_cast<NetWorkButton *>(currentWidget);
    if (currentItem)
        currentItem->setClickedStatus(SELECT);
    if (netItem) {
        netItem->setClickedStatus(SELECT);
    }
    if (previous) {
        QWidget *previousWidget = buttonListWidget->itemWidget(previous);
        StatusButton *previousItem = dynamic_cast<StatusButton *>(previousWidget);
        NetWorkButton *previousNetItem = dynamic_cast<NetWorkButton *>(previousWidget);
        if (previousItem)
            previousItem->setClickedStatus(NORMAL);
        if (previousNetItem) {
            previousNetItem->setClickedStatus(NORMAL);
        }
    }
}

void LockWidget::onButtonItemClicked(QListWidgetItem *item)
{
    QWidget *widget = buttonListWidget->itemWidget(item);
    StatusButton *currentItem = dynamic_cast<StatusButton *>(widget);
    if (currentItem) {
        currentItem->setClickedStatus(CLICKED);
        currentItem->clicked();
    }
    if (currentItem == m_powerManagerButton) {
        for (int i = 0; i < buttonListWidget->count(); i++) {
            if (!buttonListWidget->isRowHidden(i)) {
                buttonListWidget->setCurrentRow(i, QItemSelectionModel::SelectionFlag::Clear);
                buttonListWidget->currentItemChanged(buttonListWidget->item(i), buttonListWidget->item(i));
                break;
            }
        }
    }
    NetWorkButton *netItem = dynamic_cast<NetWorkButton *>(widget);
    if (netItem) {
        netItem->setClickedStatus(CLICKED);
        netItem->clicked();
    }
}

void LockWidget::onPowerItemClicked(QListWidgetItem *item)
{
    QWidget *widget = m_powerListWidget->itemWidget(item);
    PowerItemWidget *currentItem = dynamic_cast<PowerItemWidget *>(widget);
    if (currentItem) {
        currentItem->setSelected(CLICKED);
        currentItem->clicked();
    }
    if (m_isSessionTools && m_blockWidget->isHidden()) {
        exitSubWidget();
        Q_EMIT sessionToolsExit();
        m_isSessionTools = false;
    } /* else if (m_blockWidget->isHidden()) {
         exitSubWidget();
     }*/
}

void LockWidget::onUsersInfoChanged()
{
    if (buttonListWidget && m_btnItemUser) {
        if (m_modelLockDialog->normalUserCount() > 1 || m_modelLockDialog->findUserByName("*login")) {
            buttonListWidget->setItemHidden(m_btnItemUser, false);
        } else {
            buttonListWidget->setItemHidden(m_btnItemUser, true);
            if (m_userListWidget && !m_userListWidget->isHidden())
                m_userListWidget->hide();
        }
        if (m_userListWidget) {
            setUserListWidgetLocate();
        }
    }
    UserInfoPtr ptrUser = m_modelLockDialog->findUserByName(m_curUserInfo->name());
    if (ptrUser) {
        m_curUserInfo = ptrUser;
        if (m_btnItemSession && m_curUserInfo->isLoggedIn()) {
            buttonListWidget->setItemHidden(m_btnItemSession, true);
            buttonListWidget->setFixedSize(QSize(
                48 * (buttonListWidget->count() - 1)
                    + buttonListWidget->spacing() * 2 * (buttonListWidget->count() - 1),
                64));
        } else {
            buttonListWidget->setItemHidden(m_btnItemSession, false);
            buttonListWidget->setFixedSize(QSize(
                48 * buttonListWidget->count() + buttonListWidget->spacing() * 2 * buttonListWidget->count(), 64));
        }
        if (authDialog) {
            authDialog->onCurUserInfoChanged(m_curUserInfo);
        }
    }
}

void LockWidget::onCurUserChanged(const QString &strUserName)
{
    UserInfoPtr ptrUser = m_modelLockDialog->findUserByName(strUserName);
    if (ptrUser) {
        m_curUserInfo = ptrUser;
        qDebug() << "CurUserInfo:" << *m_curUserInfo;
        if (m_curUserInfo->name() != "*login" && m_curUserInfo->name() != "*guest") {
            LanguageSetting::instance()->onLanguageChanged(m_curUserInfo->lang());
        }
        if (authDialog) {
            authDialog->onCurUserChanged(m_curUserInfo);
            if (authDialog->getUserAuthFailedTimes() >= 1) {
                onShowResetPasswdBtn();
            } else if (m_pResetPwdBtn) {
                m_pResetPwdBtn->hide();
                m_isShowResetPasswdBtn = false;
            }
            if (m_pResetPwdWidget && m_pResetPwdWidget->isVisible()) {
                m_pResetPwdWidget->switchToConfidentialDig();
                m_pResetPwdWidget->hide();
            }
        }
        m_curFontSize = getUserFontSize(strUserName);
        updateFontSize();
        QVariant curUserThemeColor = getUserThemeColor(strUserName);
        Q_EMIT m_modelLockDialog->setUserThemeColor("themeColor", curUserThemeColor);
        // 登录后绘制桌面背景而不是登录背景
        QString backgroundPath = m_curUserInfo->backGround();
        QFile backgroundPathFile(backgroundPath);
        if (backgroundPath.isEmpty() || !backgroundPathFile.exists()) {
            backgroundPath = Configuration::instance(this)->getDefaultBackgroundName();
        }
        setRootWindowBgOptions(Configuration::instance(this)->getRootBackgroundOption(strUserName));
        setrootWindowBackground(rootWinPicture, 0, backgroundPath);
        QString drawBackgroundColor = m_curUserInfo->backGroundColor();
        if (drawBackgroundColor.length() == 7 && drawBackgroundColor.startsWith("#")) {
            drawBackgroundColor = drawBackgroundColor.remove(0, 1);
            bool ok;
            int color = drawBackgroundColor.toUInt(&ok, 16);
            setrootWindowBackground(rootWinColor, color, NULL);
        }
        reloadRootBackground();
    }
    if (m_btnItemSession && m_curUserInfo->isLoggedIn()) {
        buttonListWidget->setItemHidden(m_btnItemSession, true);
        buttonListWidget->setFixedSize(QSize(
            48 * (buttonListWidget->count() - 1) + buttonListWidget->spacing() * 2 * (buttonListWidget->count() - 1),
            64));
    } else {
        buttonListWidget->setItemHidden(m_btnItemSession, false);
        buttonListWidget->setFixedSize(
            QSize(48 * buttonListWidget->count() + buttonListWidget->spacing() * 2 * buttonListWidget->count(), 64));
    }
}

void LockWidget::onUserChangedByManual(const QString &userName)
{
    if (m_modelLockDialog && m_modelLockDialog->findUserByName(userName)) {
        if (isCurUserSelf(userName)) {
            Q_EMIT m_modelLockDialog->setCurrentUser(userName);
        } else {
            SwitchToUser(userName);
        }
    } else {
    }
}

void LockWidget::onSessionChanged(const QString &strSession)
{
    if (buttonListWidget) {
        for (int n = 0; n < buttonListWidget->count(); n++) {
            QListWidgetItem *item = buttonListWidget->item(n);
            QWidget *widget = buttonListWidget->itemWidget(item);
            if (widget->objectName() == "BtnSession") {
                StatusButton *sButton = qobject_cast<StatusButton *>(widget);
                if (sButton) {
                    QString strCurSession = strSession;
                    QString sessionPrefix = strCurSession.left(strCurSession.indexOf('-'));
                    if (strCurSession == "ukui-wayland")
                        sessionPrefix = "ukui_wayland";
                    if (strCurSession == "kylin-wlcom")
                        sessionPrefix = "kylin_wlcom";
                    strCurSession = IMAGE_DIR + QString("badges/%1_badge.svg").arg(sessionPrefix.toLower());
                    QFile iconFile(strCurSession);
                    if (!iconFile.exists()) {
                        strCurSession = IMAGE_DIR + QString("badges/unknown_badge.svg");
                    }
                    QPixmap ssIcon(strCurSession);
                    ssIcon = scaledPixmap(ssIcon);
                    ssIcon = ssIcon.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    ssIcon = PixmapToRound(ssIcon, 24);
                    sButton->setIcon(ssIcon);
                }
            }
        }
    }
}

void LockWidget::onUsdMediaKeysChanged(const QString &keys, const QString &value)
{
    if (keys == KEY_AREA_SCREENSHOT) {
        m_areaScreenShot = value;
    } else if (keys == KEY_AREA_SCREENSHOT2) {
        m_areaScreenShot2 = value;
    } else if (keys == KEY_SCREEN_SHOT) {
        m_screenShot = value;
    } else if (keys == KEY_SCREEN_SHOT2) {
        m_screenShot2 = value;
    } else if (keys == KEY_WINDOW_SCREENSHOT) {
        m_windowScreenshot = value;
    }
}

void LockWidget::onUsdMediaStateKeysChanged(const QString &keys, const int &value)
{
    if (keys == KEY_RFKILL_STATE) {
        m_rfkillState = value;
        setRfkillBtnIcon();
    }
}

void LockWidget::onShowSessionListWidget()
{
    initSessionWidget();
    if (m_sessionListWidget && m_sessionListWidget->isHidden()) {
        m_sessionListWidget->show();
        m_sessionListWidget->setFocus();
        m_sessionButton->setClickedStatus(CLICKED);
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
            m_networkButton->setClickedStatus(NORMAL);
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
    } else {
        m_sessionListWidget->hide();
        m_sessionButton->setClickedStatus(NORMAL);
    }
}

void LockWidget::onShowBatteryWidget()
{
    if (!batteryWidget) {
        batteryWidget = new BatteryWidget(m_modelLockDialog, this);
        batteryWidget->setFixedSize(420, 112);
        setBatteryWidgetLocate();
    }

    if (batteryWidget && batteryWidget->isHidden()) {
        batteryWidget->show();
        m_batteryonButton->setClickedStatus(CLICKED);
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
            m_networkButton->setClickedStatus(NORMAL);
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
    } else {
        batteryWidget->hide();
        m_batteryonButton->setClickedStatus(NORMAL);
    }
}

void LockWidget::onShowUserListWidget(bool show)
{
    initUserWidget();
    if (m_userListWidget && m_userListWidget->isHidden()) {
        m_userListWidget->show();
        m_userButton->setClickedStatus(CLICKED);
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
            m_networkButton->setClickedStatus(NORMAL);
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
        m_userListWidget->onUpdateListInfo();
        m_userListWidget->setFocus();
    } else if (show && m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->onUpdateListInfo();
        m_userListWidget->setFocus();
    } else {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
    }
}

void LockWidget::onShowNetworkWidget()
{
    if (!m_networkWidget) {
        m_networkWidget = new MyNetworkWidget(this);
        m_networkWidget->installEventFilter(this);
        connect(m_networkWidget, &MyNetworkWidget::showVirtualKeyboard, this, [=] {
            onShowVirtualKeyboard(m_isTabletMode);
        });
    }
    m_networkWidget->loadNetPlugin();
    setNetWorkWidgetLocate();
    if (m_networkWidget && m_networkWidget->isVisible()) {
        m_networkWidget->hide();
        m_networkButton->setClickedStatus(NORMAL);
    } else {
        m_networkWidget->show();
        m_networkButton->setClickedStatus(CLICKED);
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
    }
}

void LockWidget::onShowResetPasswdBtn()
{
    if (m_pSecurityQuestionAnswer->IsUserHasSecurityAnswers(authDialog->getCurAuthUserUid())) {
        if (!m_pResetPwdBtn) {
            m_pResetPwdBtn = new QPushButton(this);
            m_pResetPwdBtn->setFocusPolicy(Qt::NoFocus);
            m_pResetPwdBtn->installEventFilter(this);
            m_pResetPwdBtn->setFixedHeight(48);
            m_pResetPwdBtn->setMinimumWidth(120);
            m_pResetPwdBtn->setText(tr("Reset Pwd"));
            m_pResetPwdBtn->adjustSize();
            m_pResetPwdBtn->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0);border-radius: 24px;color: "
                                          "white;padding-left: 8px; padding-right: 8px;}"
                                          "QPushButton:hover{background: rgba(255, 255, 255, 0.15);border-radius: "
                                          "24px;padding-left: 8px; padding-right: 8px;}"
                                          "QPushButton:pressed {background: rgba(255, 255, 255, 0.4);border-radius: "
                                          "24px;padding-left: 8px; padding-right: 8px;}");
            connect(m_pResetPwdBtn, &QPushButton::clicked, this, &LockWidget::onShowResetPwdWidget);
            m_pResetPwdBtn->setGeometry(
                (this->width() - m_pResetPwdBtn->width()) / 2,
                height() - m_pResetPwdBtn->height() - BOTTOM_MARGIN * scale - 8,
                m_pResetPwdBtn->width(),
                m_pResetPwdBtn->height());
        }
        m_pResetPwdBtn->show();
        m_isShowResetPasswdBtn = true;
    }
}

void LockWidget::onShowResetPwdWidget()
{
    if (!m_pResetPwdWidget) {
        m_pResetPwdWidget = new ResetPwdWidget(
            m_modelLockDialog,
            authDialog->getCurAuthRealName(),
            authDialog->getCurAuthUserFace(),
            authDialog->getCurAuthUserUid(),
            m_pSecurityQuestionAnswer,
            this);
        Q_EMIT startFcitx();
        connect(m_pResetPwdWidget, &ResetPwdWidget::backToAuthDialog, this, &LockWidget::onBackToAuthDialog);
        connect(m_pResetPwdWidget, &ResetPwdWidget::resetEditClicked, this, [=]() {
            if (batteryWidget && batteryWidget->isVisible()) {
                batteryWidget->hide();
                m_batteryonButton->setClickedStatus(NORMAL);
            }
            if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
                m_sessionListWidget->hide();
                m_sessionButton->setClickedStatus(NORMAL);
            }
            if (m_userListWidget && m_userListWidget->isVisible()) {
                m_userListWidget->hide();
                m_userButton->setClickedStatus(NORMAL);
            }
            if (m_networkWidget && m_networkWidget->isVisible()) {
                m_networkWidget->hide();
                m_networkButton->setClickedStatus(NORMAL);
            }
        });
        m_pResetPwdWidget->setFixedWidth(width());
        m_pResetPwdWidget->updateResetPwdSize();
        m_pResetPwdWidget->setGeometry(
            0, (59 * scale + m_timeWidget->height()), width(), height() - (59 * scale + m_timeWidget->height() + 70));
        m_pResetPwdWidget->hide();
    }
    m_pResetPwdWidget->updateFontSize(m_curFontSize);
    m_pResetPwdWidget->show();
    m_pResetPwdWidget->setFocusToEdit();
    m_pResetPwdWidget->raise();
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->hide();
    }
    authDialog->hide();
}

void LockWidget::onBackToAuthDialog(bool isSuccess)
{
    authDialog->inhibitShow(false);
    authDialog->setFocus();
    if (m_pResetPwdWidget)
        m_pResetPwdWidget->hide();
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->show();
    }
    if (isSuccess)
        authDialog->clearMessage();
}

void LockWidget::onLineEditClicked()
{
    if (!m_virtualKeyboardWidget) {
        m_virtualKeyboardWidget = new VirtualKeyboardWidget(this);
        m_virtualKeyboardWidget->installEventFilter(this);
        m_virtualKeyboardWidget->hide();

        connect(m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToClose, this, &LockWidget::onHideVirkeyboard);
        connect(
            m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToFloat, this, &LockWidget::onNetWorkResetLocation);
    }
    if (m_isTabletMode) {
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
        m_virtualKeyboardWidget->show();
        authDialog->setFocus();
    }
}

void LockWidget::onShowVirtualKeyboard()
{
    if (!m_virtualKeyboardWidget) {
        m_virtualKeyboardWidget = new VirtualKeyboardWidget(this);
        m_virtualKeyboardWidget->installEventFilter(this);
        m_virtualKeyboardWidget->hide();

        connect(m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToClose, this, &LockWidget::onHideVirkeyboard);
        connect(
            m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToFloat, this, &LockWidget::onNetWorkResetLocation);
    }
    if (m_virtualKeyboardWidget->isHidden()) {
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
        m_virtualKeyboardWidget->show();
        authDialog->setFocus();
    } else {
        m_virtualKeyboardWidget->hide();
    }

    if (!m_virtualKeyboardWidget->isHidden()) {
        setNetWorkWidgetLocate();
        m_virtualKeyboardWidget->raise();
    }
    m_virKbButton->setClickedStatus(NORMAL);
}

void LockWidget::onShowVirtualKeyboard(bool tabletMode)
{
    m_isTabletMode = tabletMode;
    if (!m_virtualKeyboardWidget) {
        m_virtualKeyboardWidget = new VirtualKeyboardWidget(this);
        m_virtualKeyboardWidget->installEventFilter(this);
        m_virtualKeyboardWidget->hide();

        connect(m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToClose, this, &LockWidget::onHideVirkeyboard);
        connect(
            m_virtualKeyboardWidget, &VirtualKeyboardWidget::aboutToFloat, this, &LockWidget::onNetWorkResetLocation);
    }
    if (tabletMode) {
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            m_sessionButton->setClickedStatus(NORMAL);
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            m_userButton->setClickedStatus(NORMAL);
        }
        m_virtualKeyboardWidget->show();
        authDialog->setFocus();
    } else {
        m_virtualKeyboardWidget->hide();
    }

    if (!m_virtualKeyboardWidget->isHidden()) {
        setNetWorkWidgetLocate();
        m_virtualKeyboardWidget->raise();
    }
    m_virKbButton->setClickedStatus(NORMAL);
}

void LockWidget::onShowPowerListWidget(bool issessionTools)
{
    if (issessionTools) {
        m_powerListWidget->setPowerType(SESSION);
        m_isSessionTools = issessionTools;
    } else {
        m_powerListWidget->setPowerType(SAVER);
    }
    if (m_powerListWidget->isHidden()) {
        m_powerListWidget->show();
        if (m_isShowResetPasswdBtn) {
            m_pResetPwdBtn->hide();
        }
        if (m_pResetPwdWidget && m_pResetPwdWidget->isVisible()) {
            m_pResetPwdWidget->switchToConfidentialDig();
            m_pResetPwdWidget->hide();
        }
        authDialog->inhibitShow(true);
        if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
            m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
        } else { // 居中显示
            m_powerListWidget->move(
                (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
        }
        if (m_isSessionTools) {
            m_systemMonitorBtn->show();
            buttonListWidget->hide();
        }
        buttonListWidget->clearFocus();
        if (m_widgetMediaControl) {
            m_widgetMediaControl->hide();
        }
    } else {
        m_powerListWidget->hide();
        authDialog->inhibitShow(false);
        // bug#292889 头像加载缓慢问题，需要调用一次update或repaint来刷新
        authDialog->update();
        authDialog->setFocus();
        if (m_isShowResetPasswdBtn) {
            m_pResetPwdBtn->show();
        }
        m_systemMonitorBtn->hide();
        buttonListWidget->show();
        if (m_widgetMediaControl) {
            m_widgetMediaControl->show();
        }
    }

    //    if (m_isInhibitStatus) {
    //        authDialog->hide();
    //        m_powerListWidget->hide();
    //        buttonListWidget->hide();
    //        m_timeWidget->hide();
    //    }

    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
    }
    if (m_virtualKeyboardWidget && m_virtualKeyboardWidget->isVisible()) {
        m_virtualKeyboardWidget->hide();
        m_virKbButton->setClickedStatus(NORMAL);
    }
    if (batteryWidget && batteryWidget->isVisible()) {
        batteryWidget->hide();
        m_batteryonButton->setClickedStatus(NORMAL);
    }
    if (m_networkWidget && m_networkWidget->isVisible()) {
        m_networkWidget->hide();
        m_networkButton->setClickedStatus(NORMAL);
    }
    if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
        m_sessionListWidget->hide();
        m_sessionButton->setClickedStatus(NORMAL);
    }
    m_powerManagerButton->setClickedStatus(NORMAL);
}

void LockWidget::doSystemMonitor()
{
    qDebug() << "doSystemMonitor....";
    QProcess::startDetached("ukui-system-monitor", QStringList());
}

void LockWidget::onSetVirkeyboardPos()
{
    if (m_virtualKeyboardWidget) {
        m_virtualKeyboardWidget->adjustGeometry();
    }
}

void LockWidget::onNetWorkResetLocation()
{
    if (m_networkWidget && m_networkWidget->isVisible()) {
        setNetWorkWidgetLocate();
    }
    if (m_virtualKeyboardWidget && m_virtualKeyboardWidget->isVisible()) {
        m_virtualKeyboardWidget->raise();
    }
}

void LockWidget::onHideVirkeyboard()
{
    if (m_virtualKeyboardWidget && m_virtualKeyboardWidget->isVisible()) {
        m_virtualKeyboardWidget->hide();
    }
    setNetWorkWidgetLocate();
}

void LockWidget::onShowInhibitWarning(QStringList list, int type, bool iscommand)
{
    authDialog->inhibitShow(true);
    if (m_widgetMediaControl) {
        m_widgetMediaControl->hide();
    }
    buttonListWidget->hide();
    m_timeWidget->hide();
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
    }
    m_powerListWidget->hide();
    m_blockWidget->setGeometry(this->geometry());
    m_blockWidget->setWarning(list, type);
    m_blockWidget->show();
    m_isInhibitStatus = true;
    if (!m_isSessionTools)
        m_isSessionTools = iscommand;
    if (m_isSessionTools)
        m_systemMonitorBtn->show();
}

void LockWidget::onSureShutDown(int inhibitType, bool iscommand)
{
    authDialog->inhibitShow(true);
    buttonListWidget->hide();
    m_timeWidget->hide();
    m_powerListWidget->hide();
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
    }
    m_blockWidget->setGeometry(this->geometry());
    if (inhibitType == 0) {
        m_inhibitType = "Reboot";
        m_blockWidget->setTips(
            tr("%1 may cause users who have logged in to this computer to lose content that has not yet "
               "been stored,To still perform please click \"Confirm\".")
                .arg(tr("Restart")));
    } else if (inhibitType == 1) {
        m_inhibitType = "PowerOff";
        m_blockWidget->setTips(
            tr("%1 may cause users who have logged in to this computer to lose content that has not yet "
               "been stored,To still perform please click \"Confirm\".")
                .arg(tr("Shut Down")));
    }
    m_blockWidget->setMsgTips(inhibitType);
    m_blockWidget->show();
    m_isInhibitStatus = true;
    if (!m_isSessionTools)
        m_isSessionTools = iscommand;
    if (m_isSessionTools)
        m_systemMonitorBtn->show();
}

void LockWidget::onMulUsersLogined(int inhibitType, bool iscommand)
{
    authDialog->inhibitShow(true);
    if (m_widgetMediaControl) {
        m_widgetMediaControl->hide();
    }
    buttonListWidget->hide();
    m_timeWidget->hide();
    m_powerListWidget->hide();
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        m_userButton->setClickedStatus(NORMAL);
    }
    m_blockWidget->setGeometry(this->geometry());
    if (inhibitType == 0) {
        m_inhibitType = "Reboot";
        m_blockWidget->setTips(
            tr("Multiple users are logged in at the same time.Are you sure you want to %1 this system?")
                .arg(tr("Restart")));
    } else if (inhibitType == 1) {
        m_inhibitType = "PowerOff";
        m_blockWidget->setTips(
            tr("Multiple users are logged in at the same time.Are you sure you want to %1 this system?")
                .arg(tr("Shut Down")));
    }
    m_blockWidget->show();
    m_isInhibitStatus = true;
    if (!m_isSessionTools)
        m_isSessionTools = iscommand;
    if (m_isSessionTools)
        m_systemMonitorBtn->show();
}

void LockWidget::onConfirmBtnClicked()
{
    m_blockWidget->hide();
    m_powerListWidget->hide();
    authDialog->inhibitShow(false);
    if (m_widgetMediaControl) {
        m_widgetMediaControl->show();
    }
    m_timeWidget->show();
    buttonListWidget->show();
    m_isInhibitStatus = false;
    Q_EMIT m_modelLockDialog->setPowerManager(m_inhibitType);
    if (m_isSessionTools) {
        //        m_isSessionTools = false;
        exitSubWidget();
        Q_EMIT sessionToolsExit();
    }
}

void LockWidget::onCancelBtnClicked()
{
    m_isInhibitStatus = false;
    m_blockWidget->hide();
    m_powerListWidget->show();
    buttonListWidget->show();
    m_timeWidget->show();
    if (m_isSessionTools) {
        //        m_isSessionTools = false;
        exitSubWidget();
        Q_EMIT sessionToolsExit();
    }
}

void LockWidget::onLockScreenClicked()
{
    m_isSessionTools = false;
    exitSubWidget();
}

void LockWidget::onSuspendClicked()
{
    if (!isGreeterMode() && m_isSessionTools && m_modelLockDialog->getSleepActivationEnabled()) {
        m_isSessionTools = false;
        Q_EMIT showBlankScreensaver(0, true);
    } else {
        exitSubWidget();
    }
}

void LockWidget::onSwitchuserClicked()
{
    onLockScreenClicked();
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    onShowUserListWidget();
}

void LockWidget::onCustomRequestAccount(QString account)
{
    if (m_modelLockDialog) {
        SwitchToUser(account);
    }
}

void LockWidget::onGetCustomPluginMsg(QString strMsg)
{
    QJsonParseError jsonParseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strMsg.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        qWarning() << "Parse message json failed!!";
        return;
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
            qWarning() << "Message Json is null!!";
            return;
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

void LockWidget::stopAuth()
{
    if (authDialog) {
        authDialog->stopAuth();
    }
}

void LockWidget::startAuth()
{
    if (authDialog) {
        authDialog->startAuth();
    }
}

void LockWidget::onSessionActiveChanged(bool isActive)
{
    if (isActive) {
        if (m_timerChkActive && m_timerChkActive->isActive()) {
            m_timerChkActive->stop();
        }
        if (m_modelLockDialog && m_modelLockDialog->isUseWayland()) {
            QTimer::singleShot(500, this, [&, this]() {
                if (this->isHidden()) {
                    this->show();
                    update();
                }
                startAuth();
            });
        } else {
            if (this->isHidden()) {
                this->show();
                update();
            }
            startAuth();
        }
    }
}

void LockWidget::onTabletModeChanged(bool tabletMode)
{
    onShowVirtualKeyboard(tabletMode);
}

void LockWidget::onFontChanged(QString font)
{
    m_curFont = font;
}

void LockWidget::onFontSizeChanged(double fontSize)
{
    m_curFontSize = fontSize;
    updateFontSize();
}

void LockWidget::onAuthDialogBottomPosChanged(int nPosY)
{
    if (m_widgetMediaControl) {
        if (nPosY + m_widgetMediaControl->height() + 64 <= height()) {
            m_widgetMediaControl->move(
                (width() - m_widgetMediaControl->width()) / 2,
                (nPosY + height() - 64 - m_widgetMediaControl->height()) / 2);
        } else {
            m_widgetMediaControl->move(24, 27);
        }
        qDebug() << "m_widgetMediaControl geometry:" << m_widgetMediaControl->geometry();
    }
}

void LockWidget::onServiceUserChanged(WillLoginUserInfo willSwitchUser)
{
    if (!isGreeterMode() || willSwitchUser.strUserName.isEmpty()) {
        if (isCurUserSelf(willSwitchUser.strUserName)) { // 锁屏且是当前用户则清空不处理
            UniAuthService::instance()->SwitchToGreeterUser("");
        }
        return;
    }
    static QString strLastUserName = "";
    static quint64 llLastChangeTick = 0;

    // 清空 willSwitchUser
    UniAuthService::instance()->SwitchToGreeterUser("");

    quint64 curChangeTick = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if (willSwitchUser.strUserName == strLastUserName && curChangeTick - llLastChangeTick <= 2000) {
        qDebug() << "onServiceUserChanged same name and time short:" << willSwitchUser.strUserName << ","
                 << curChangeTick - llLastChangeTick;
        return;
    }
    strLastUserName = willSwitchUser.strUserName;
    llLastChangeTick = curChangeTick;

    if (willSwitchUser.strUserName.isEmpty()
        || !UniAuthService::instance()->isUserNameValid(willSwitchUser.strUserName)) {
        qDebug() << "WillSwitchUser is NULL or invalid:" << willSwitchUser.strUserName;
        return;
    }
    qDebug() << "willSwitchUser:" << willSwitchUser;
    SwitchToUser(willSwitchUser.strUserName, true);
}

void LockWidget::SwitchToUser(QString strUserName, bool isSwitchInCurPage /* = false*/)
{
    if (m_modelLockDialog) {
        if (isSwitchInCurPage) {
            Q_EMIT m_modelLockDialog->setCurrentUser(strUserName);
        } else {
            UniAuthService::instance()->SwitchToGreeterUser(strUserName);
            int nSwitchRet = Q_EMIT m_modelLockDialog->switchToUser(strUserName);
            if (nSwitchRet != 0) {
                UniAuthService::instance()->SwitchToGreeterUser("");
                Q_EMIT m_modelLockDialog->setCurrentUser(strUserName);
            } else {
                //            if(authDialog) {
                //                authDialog->stopAuth();
                //            }
                if (!isCurUserSelf(strUserName) && !isHidden()) {
                    this->hide();
                    update();
                    if (!m_timerChkActive) {
                        m_timerChkActive = new QTimer(this);
                        m_timerChkActive->setInterval(10 * 1000);
                        connect(m_timerChkActive, &QTimer::timeout, this, [&, this]() {
                            if (this->isHidden() && m_modelLockDialog->sessionActive()) {
                                this->show();
                            }
                            m_timerChkActive->stop();
                        });
                    } else {
                        if (m_timerChkActive->isActive()) {
                            m_timerChkActive->stop();
                        }
                    }
                    m_timerChkActive->start();
                    stopAuth();
                } else {
                    UniAuthService::instance()->SwitchToGreeterUser("");
                    Q_EMIT m_modelLockDialog->setCurrentUser(strUserName);
                }
            }
        }
    }
}

void LockWidget::setRfkillState()
{
    if (m_rfkillState == 0) {
        Q_EMIT m_modelLockDialog->setRfkillState(KEY_RFKILL_STATE, 1);
    } else if (m_rfkillState == 1) {
        Q_EMIT m_modelLockDialog->setRfkillState(KEY_RFKILL_STATE, 0);
    }
    m_pRfkillStatusButton->setClickedStatus(NORMAL);
}

void LockWidget::setRfkillBtnIcon()
{
    if (m_rfkillState == 0) {
        QPixmap rfkillOffIcon = drawSymbolicColoredPixmap(
            QIcon::fromTheme("ukui-airplane-mode-on-symbolic").pixmap(QSize(24, 24)), "gray");
        m_pRfkillStatusButton->setIcon(rfkillOffIcon);
    } else if (m_rfkillState == 1) {
        QPixmap rfkillOnIcon = drawSymbolicColoredPixmap(
            QIcon::fromTheme("ukui-airplane-mode-on-symbolic").pixmap(QSize(24, 24)), "white");
        m_pRfkillStatusButton->setIcon(QIcon(rfkillOnIcon));
    }
}

void LockWidget::refreshTranslate()
{
    QLocale local;
    systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (m_pBottomWidget)
            m_pBottomWidget->setLayoutDirection(Qt::RightToLeft);
        if (m_userListWidget)
            m_userListWidget->setLayoutDirection(Qt::RightToLeft);
        if (m_powerListWidget)
            m_powerListWidget->setLayoutDirection(Qt::RightToLeft);
    } else {
        if (m_pBottomWidget)
            m_pBottomWidget->setLayoutDirection(Qt::LeftToRight);
        if (m_userListWidget)
            m_userListWidget->setLayoutDirection(Qt::LeftToRight);
        if (m_powerListWidget)
            m_powerListWidget->setLayoutDirection(Qt::LeftToRight);
    }
    setUserListWidgetLocate();
}

void LockWidget::setUserListWidgetLocate()
{
    if (!m_userListWidget || !buttonListWidget)
        return;
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_userListWidget->move(
            RIGHT_MARGIN,
            height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale
                - m_userListWidget->height());
    } else {
        m_userListWidget->move(
            width() - m_userListWidget->width() - RIGHT_MARGIN,
            height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale
                - m_userListWidget->height());
    }
}

void LockWidget::setNetWorkWidgetLocate()
{
    if (!m_networkWidget || !buttonListWidget)
        return;
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (m_networkWidget->isVisible() && m_virtualKeyboardWidget && !m_virtualKeyboardWidget->getFloatStatus()
            && m_virtualKeyboardWidget->isVisible()) {
            m_networkWidget->move(
                RIGHT_MARGIN * scale, this->height() - m_networkWidget->height() - m_virtualKeyboardWidget->height());
            m_networkWidget->raise();
        } else {
            m_networkWidget->move(
                RIGHT_MARGIN * scale,
                this->height() - m_networkWidget->height() - BOTTOM_MARGIN * scale - buttonListWidget->spacing() * scale
                    - buttonListWidget->height());
        }
    } else {
        if (m_networkWidget->isVisible() && m_virtualKeyboardWidget && !m_virtualKeyboardWidget->getFloatStatus()
            && m_virtualKeyboardWidget->isVisible()) {
            m_networkWidget->move(
                this->width() - m_networkWidget->width() - RIGHT_MARGIN * scale,
                this->height() - m_networkWidget->height() - m_virtualKeyboardWidget->height());
            m_networkWidget->raise();
        } else {
            m_networkWidget->move(
                this->width() - m_networkWidget->width() - RIGHT_MARGIN * scale,
                this->height() - m_networkWidget->height() - BOTTOM_MARGIN * scale - buttonListWidget->spacing() * scale
                    - buttonListWidget->height());
        }
    }
}

void LockWidget::setSessionListWidgetLocate()
{
    if (!m_sessionListWidget)
        return;
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (buttonListWidget) {
            m_sessionListWidget->move(
                RIGHT_MARGIN * scale,
                height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale * 2
                    - m_sessionListWidget->height());
        }
    } else {
        if (buttonListWidget) {
            m_sessionListWidget->move(
                this->width() - m_sessionListWidget->width() - RIGHT_MARGIN * scale,
                height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale * 2
                    - m_sessionListWidget->height());
        }
    }
}

void LockWidget::setBatteryWidgetLocate()
{
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (batteryWidget && buttonListWidget) {
            batteryWidget->move(
                RIGHT_MARGIN * scale,
                height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale
                    - batteryWidget->height());
        }
    } else {
        if (batteryWidget && buttonListWidget) {
            batteryWidget->move(
                this->width() - batteryWidget->width() - RIGHT_MARGIN * scale,
                height() - BOTTOM_MARGIN * scale - buttonListWidget->height() - buttonListWidget->spacing() * scale
                    - batteryWidget->height());
        }
    }
}

void LockWidget::onLanguageChanged(bool isCompleted)
{
    if (isCompleted) {
        if (m_networkWidget) {
            m_networkWidget->unloadNetPlugin();
        }
    }
}

void LockWidget::setrootWindowBackground(int type, unsigned int color, QString filename)
{
    drawBackgroundType = type;
    if (drawBackgroundType == rootWinPicture)
        m_rootWindowBackground = filename;
    else if (drawBackgroundType == rootWinColor)
        drawBackgroundColor = color;
}

void LockWidget::setRootWindowBgOptions(int nOption)
{
    m_nPicOptions = nOption;
}

bool LockWidget::getdrawBackgroundIsStarted()
{
    return drawBackgroundIsStarted;
}

void LockWidget::setrootWindow(QList<QPair<QRect, QRect>> screenRectList)
{
    drawBackgroundIsStarted = true;
    QString m_defaultBackgroundPath = DEFAULT_BACKGROUND_PATH;
    if (m_rootWindowBackground.isEmpty())
        m_rootWindowBackground = m_defaultBackgroundPath;
    if (m_rootWindowBackground.isEmpty())
        return;

    char *path;
    QByteArray ba = m_rootWindowBackground.toLatin1(); // must
    path = ba.data();

    if (drawBackgroundType == 0)
        setRootWindowBackground(0, 0, path, screenRectList, m_nPicOptions);
    else
        setRootWindowBackground(1, drawBackgroundColor, NULL, screenRectList, m_nPicOptions);
    drawBackgroundIsStarted = false;
}

void LockWidget::reloadRootBackground()
{
    if (!isGreeterMode() || m_modelLockDialog->isUseWayland()) {
        return;
    }
    // 记录当前的屏幕信息，写入QList保存，防止线程中在遍历屏幕信息时，因
    // 屏幕插拔而崩溃
    m_screenRectList.clear();
    for (QScreen *screen : QApplication::screens()) {
        QRect rect1 = screen->geometry();
        QRect rect2 = screen->virtualGeometry();
        m_screenRectList.append(qMakePair(rect1, rect2));
    }
    if (getdrawBackgroundIsStarted()) {
        if (m_futureDrawBg.isStarted() && m_futureDrawBg.isRunning()) {
            m_futureDrawBg.waitForFinished();
            m_futureDrawBg = QtConcurrent::run([=]() { setrootWindow(m_screenRectList); });
        } else {
            m_futureDrawBg = QtConcurrent::run([=]() { setrootWindow(m_screenRectList); });
        }
    } else {
        m_futureDrawBg = QtConcurrent::run([=]() { setrootWindow(m_screenRectList); });
    }
}

void LockWidget::drawRootBackground()
{
    if (m_modelLockDialog->isUseWayland()) {
        return;
    }
    drawBackground(true);
}

void LockWidget::updateFont()
{
    if (authDialog) {
        authDialog->updateAuthFont(m_curFont);
    }
    if (m_timeWidget) {
        m_timeWidget->updateTimeFont(m_curFont);
    }
    if (m_powerListWidget) {
        QList<PowerItemWidget *> itemList = this->findChildren<PowerItemWidget *>();
        for (int i = 0; i < itemList.count(); i++) {
            itemList.at(i)->setFontFamily(m_curFont);
        }
    }
    if (m_userListWidget) {
        QList<UserItemWidget *> itemList = this->findChildren<UserItemWidget *>();
        for (int i = 0; i < itemList.count(); i++) {
            itemList.at(i)->setFontFamily(m_curFont);
        }
    }
    if (m_blockWidget) {
        m_blockWidget->updateFontFamily(m_curFont);
    }
    if (m_systemMonitorBtn) {
        QFont sysBtnFont;
        sysBtnFont.setFamily(m_curFont);
        sysBtnFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
        m_systemMonitorBtn->setFont(sysBtnFont);
        m_systemMonitorBtn->setText(tr("system-monitor"));
        m_systemMonitorBtn->adjustSize();
    }
    if (m_mediaControlWidget) {
        m_mediaControlWidget->updateFont(m_curFont, m_curFontSize);
    }
}

void LockWidget::updateFontSize()
{
    if (authDialog) {
        authDialog->updateAuthFontSize(m_curFontSize);
    }
    if (m_timeWidget) {
        m_timeWidget->updateTimeFontSize(m_curFontSize);
    }
    if (m_powerListWidget) {
        QList<PowerItemWidget *> itemList = this->findChildren<PowerItemWidget *>();
        for (int i = 0; i < itemList.count(); i++) {
            itemList.at(i)->setFontSize((16 + m_curFontSize) * m_ptToPx);
        }
    }
    if (m_userListWidget) {
        QList<UserItemWidget *> itemList = this->findChildren<UserItemWidget *>();
        for (int i = 0; i < itemList.count(); i++) {
            itemList.at(i)->setFontSize((14 + m_curFontSize) * m_ptToPx);
        }
    }
    if (m_mediaControlWidget) {
        m_mediaControlWidget->updateFont(m_curFont, m_curFontSize);
    }
}

void LockWidget::initMediaControlWidget()
{
#ifdef SUPPORT_MEDIA_CONTROL
    if (isGreeterMode()) {
        return;
    }
    if (!m_widgetMediaControl) {
        m_widgetMediaControl = new QWidget(this);
        m_widgetMediaControl->setFixedSize(320, 120);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        if (!m_mediaControlWidget) {
            m_mediaControlWidget = new MediaControlWidget(m_widgetMediaControl);
            m_mediaControlWidget->updateFont(m_curFont, m_curFontSize);
            m_mediaControlWidget->initMultimediaControl();
            m_mediaControlWidget->setVisible(false);
            layout->addWidget(m_mediaControlWidget);
        }
        m_widgetMediaControl->setLayout(layout);
    }
#endif
}
