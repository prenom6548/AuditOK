/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "titlebar.h"
#include "common.h"
#include "gamekey/gamekeymanager.h"
#include "gamekey/joystickmanager.h"
#include "displaymanager.h"
#include "preferences.h"
#include "menu.h"
#include "action.h"
#include "kmreenv.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVariant>
#include <QMouseEvent>
#include <QGSettings>
#include <QTimer>
#include <QX11Info>
#include <X11/Xlib.h>
#include <sys/syslog.h>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QTouchEvent>

#include "wayland/ukui/ukui-decoration-manager.h"
#include "wayland/xdg/XdgManager.h"

TitleBar::TitleBar(KmreWindow *window) 
    : QWidget(window)
    , mMainWindow(window)
    , mButtonPressed(false)
    , m_moveFlag(true)
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    //this->setStyleSheet("QWidget{background: palette(base);}");
    this->setFixedHeight(DEFAULT_TITLEBAR_HEIGHT);
    this->setFocusPolicy(Qt::NoFocus);//this->setAttribute(Qt::WA_ShowWithoutActivating, true);
    this->setMouseTracking(true);

    this->initUI();
    this->initConnect();

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        installEventFilter(this);
    }
}

TitleBar::~TitleBar()
{
    QLayoutItem *child;
    if(m_lLayout!=nullptr){
        while ((child = m_lLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }
    }

    if(m_mLayout!=nullptr){
        while ((child = m_mLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }
    }
    if(m_rLayout!=nullptr){
        while ((child = m_rLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }
    }
}

int TitleBar::getMenuBtnX()
{
    //return m_menuBtn->x();
    return this->width() - m_menuBtn->width()*4 - BUTTON_SPACE*3;
}

void TitleBar::setIcon(const QString &iconName)
{
    //const QIcon &icon = QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    const QIcon &icon = QIcon(iconName);
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(m_iconLabel->size()));
    }
}

void TitleBar::initLeftContent()
{
    QWidget *w = new QWidget;
    m_lLayout = new QHBoxLayout(w);
    m_lLayout->setContentsMargins(8, 0, 0, 0);
    m_lLayout->setSpacing(8);
    m_layout->addWidget(w, 1, Qt::AlignLeft);

    m_iconLabel = new QLabel;
    m_lLayout->addWidget(m_iconLabel);
    m_iconLabel->setFixedSize(24, 24);

    m_titleLabel = new QLabel;
    m_lLayout->addWidget(m_titleLabel);

    m_lLayout->addSpacing(8);
    m_backBtn = new QPushButton;
    m_backBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_backBtn->setFlat(true);
    m_backBtn->setToolTip(tr("Back"));
    m_backBtn->setProperty("useIconHighlightEffect", 0x2);
    m_backBtn->setProperty("isWindowButton", 0x01);
    m_backBtn->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    m_backBtn->setFocusPolicy(Qt::NoFocus);
    m_backBtn->setVisible(false);

    m_lLayout->addWidget(m_backBtn);
}
//QLabel *tmpLabel= nullptr;
void TitleBar::initMiddleContent()
{
    QWidget *w = new QWidget;
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mLayout = new QHBoxLayout(w);
    m_mLayout->setContentsMargins(0, 4, 0, 0);
    m_mLayout->setSpacing(5);
    m_layout->addWidget(w);
}

void TitleBar::initRightContent()
{
    QWidget *w = new QWidget;
    m_rLayout = new QHBoxLayout(w);
    m_rLayout->setContentsMargins(0, 0, 4, 0);
    m_rLayout->setSpacing(BUTTON_SPACE);
    m_layout->addWidget(w, 1, Qt::AlignRight);

    m_menuBtn = new QPushButton;
    m_menuBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
//    if (m_useUkuiTheme) {
//        m_menuBtn->setProperty("isOptionButton", true);
//        m_menuBtn->setProperty("isWindowButton", 0x1);
//        m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
//        m_menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
//        //m_menuBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
//    }
//    else {
//        m_menuBtn->setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url(':/res/menu1.png');}QPushButton:hover{border:0px;background-image:url(':/res/menu2.png');}QPushButton:pressed{border:0px;background-image:url(':/res/menu3.png');}");
//    }
    m_menuBtn->setFlat(true);
    m_menuBtn->setToolTip(tr("Options"));

    m_fullscreenBtn = new QPushButton;
    m_fullscreenBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_fullscreenBtn->setFlat(true);
    m_fullscreenBtn->setToolTip(tr("Fullscreen"));

    m_minBtn = new QPushButton;
    m_minBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
//    if (m_useUkuiTheme) {
//        m_minBtn->setProperty("isWindowButton", 0x1);
//        m_minBtn->setProperty("useIconHighlightEffect", 0x2);
//        m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
//        //m_minBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
//    }
//    else {
//        m_minBtn->setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url(':/res/mini1.png');}QPushButton:hover{border:0px;background-image:url(':/res/mini2.png');}QPushButton:pressed{border:0px;background-image:url(':/res/mini3.png');}");
//    }
    m_minBtn->setFlat(true);
    m_minBtn->setToolTip(tr("Minimize"));

    m_closeBtn = new QPushButton;
    m_closeBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
//    if (m_useUkuiTheme) {
//        m_closeBtn->setProperty("isWindowButton", 0x2);
//        m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
//        m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
//        //m_closeBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
//    }
//    else {
//        m_closeBtn->setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url(':/res/close1.png')}QPushButton:hover{border:0px;background-image:url(':/res/close2.png')}QPushButton:pressed{border:0px;background-image:url(':/res/close3.png')}");
//    }
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));

    m_topBtn = new QPushButton;
    m_topBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_topBtn->setFlat(true);
    m_topBtn->setToolTip(tr("Top"));

    m_cancletopBtn = new QPushButton;
    m_cancletopBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_cancletopBtn->setFlat(true);
    m_cancletopBtn->setToolTip(tr("Cancel Top"));
    m_cancletopBtn->setVisible(false);

    m_screenshotBtn = new QToolButton(this);
    m_screenshotBtn->setFixedSize(36,36);
    m_screenshotBtn->setAutoRaise(true);
    m_screenshotBtn->setToolTip(tr("Screenshot"));
    checkScreenshotFeature();
    if (m_hideWhileScreenShotSupported) {
        m_screenshotBtn->setPopupMode(QToolButton::MenuButtonPopup);
        m_screenshotBtn->setFixedSize(60,36);
        m_screenshotBtn->setMenu(HideMenu);
    }

    m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuBtn->setProperty("isWindowButton", 0x01);
    m_fullscreenBtn->setProperty("useIconHighlightEffect", 0x2);
    m_fullscreenBtn->setProperty("isWindowButton", 0x01);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setProperty("isWindowButton", 0x01);
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_topBtn->setProperty("useIconHighlightEffect", 0x2);
    m_topBtn->setProperty("isWindowButton", 0x01);
    m_cancletopBtn->setProperty("useIconHighlightEffect", 0x2);
    m_cancletopBtn->setProperty("isWindowButton", 0x01);
    m_screenshotBtn->setProperty("useIconHighlightEffect", 0x2);
    m_screenshotBtn->setProperty("isWindowButton", 0x01);

    // /usr/share/icons/ukui-icon-theme-default/scalable/actions/
    m_menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_fullscreenBtn->setIcon(QIcon::fromTheme("view-fullscreen-symbolic"));
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_topBtn->setIcon(QIcon::fromTheme("ukui-fixed-symbolic"));
    m_cancletopBtn->setIcon(QIcon::fromTheme("ukui-unfixed-symbolic"));
    m_screenshotBtn->setIcon(QIcon::fromTheme("screenshot-app-symbolic"));

    m_menuBtn->setFocusPolicy(Qt::NoFocus);
    m_fullscreenBtn->setFocusPolicy(Qt::NoFocus);
    m_minBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_topBtn->setFocusPolicy(Qt::NoFocus);
    m_cancletopBtn->setFocusPolicy(Qt::NoFocus);
    m_screenshotBtn->setFocusPolicy(Qt::NoFocus);

    m_rLayout->addWidget(m_topBtn);
    m_rLayout->addWidget(m_cancletopBtn);
    m_rLayout->addWidget(m_screenshotBtn);
    m_rLayout->addWidget(m_menuBtn);
    m_rLayout->addWidget(m_fullscreenBtn);
    m_rLayout->addWidget(m_minBtn);
    m_rLayout->addWidget(m_closeBtn);
}

void TitleBar::initShotMenu()
{
    HideMenu = new Menu;

    KmreConfig::Preferences *preferences = KmreConfig::Preferences::getInstance();
    m_setHideAction = new Action(tr("Hide Current Window"), preferences->m_toScreenHide);
    if (preferences->m_toScreenHide) {
        QTimer::singleShot(500, [&]() {emit sigToHide(m_setHideAction->isChecked());});
    }
    connect(m_setHideAction, &Action::triggered, [=] {
        m_setHideAction->update(!m_setHideAction->isChecked());
        preferences->m_toScreenHide = m_setHideAction->isChecked();
        preferences->updateWindowHidestatus();
        emit sigToHide(m_setHideAction->isChecked());
    });
    HideMenu->addAction(m_setHideAction);
}

void TitleBar::initUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);//m_layout->setMargin(0);
    m_layout->setSpacing(0);
    this->setLayout(m_layout);

    this->initShotMenu();
    this->initLeftContent();
    this->initMiddleContent();
    this->initRightContent();  
    this->updateUi();
}

void TitleBar::updateLayout(int width)
{
    //syslog(LOG_DEBUG, "[%s] Update Titlebar layout, width = %d", __func__, width);
    int mLayoutDeltaSize = 0;

    if (width >= 450) {
	    mLayoutDeltaSize = 0;
    }
    else if (width >= 400) {
	    mLayoutDeltaSize = 1;
    }
    else if (width >= 350) {
	    mLayoutDeltaSize = 2;
    }
    else {
	    mLayoutDeltaSize = 4;
    }

    m_rLayout->setContentsMargins(0, 0, 4 - mLayoutDeltaSize, 0);
    m_rLayout->setSpacing(4 - mLayoutDeltaSize);

    m_topBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
    m_cancletopBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
    //int tmpWidth = m_hideWhileScreenShotSupported ? (60 - mLayoutDeltaSize) : (36 - mLayoutDeltaSize);
    //m_screenshotBtn->setFixedSize(tmpWidth, 36 - mLayoutDeltaSize);
    m_menuBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
    m_fullscreenBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
    m_minBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
    m_closeBtn->setFixedSize(BUTTON_SIZE - mLayoutDeltaSize, BUTTON_SIZE - mLayoutDeltaSize);
}

void TitleBar::updateTitleName()
{
    bool needAppend = false;
    QString titleName = mTitleName;
    int titleMaxLength = parentWidget()->width() - m_iconLabel->width() - m_backBtn->width() - 
            m_topBtn->width() - m_screenshotBtn->width() - m_menuBtn->width() -
            m_fullscreenBtn->width() - m_minBtn->width() - m_closeBtn->width() - 
	        ((10 - mLayoutDeltaSize) * 8);

    // syslog(LOG_DEBUG, "[%s] window width = %d, titleMaxLength = %d", 
    //         __func__, parentWidget()->width(), titleMaxLength);
    
    while (titleName.length() > 0) {
        int titleLength = QFontMetrics(font()).width(titleName);
        if (titleLength > titleMaxLength) {
            titleName.remove(titleName.length() - 1, 1);
            needAppend = true;
        }
        else {
            if (needAppend) {
                titleName.append("...");
            }
            break;
        }
    }

    //syslog(LOG_DEBUG, "[%s] titleName = %s", __func__, titleName.toStdString().c_str());
    m_titleLabel->setText(titleName);
}

void TitleBar::updateTitle(int width)
{
    updateLayout(width);
    updateTitleName();
}

void TitleBar::updateTitleName(const QString &titleName, bool force)
{
    if (force || mTitleName.isEmpty()) {
        setTitleName(titleName);
        m_titleLabel->setText(titleName);
    }
}

void TitleBar::updateUi()
{
    QString pkgName = mMainWindow->getPackageName();
    KmreConfig::Feature *feature = KmreConfig::Feature::getInstance();

    pkgName = kmre::utils::getRealPkgName(pkgName);
    m_backBtn->setVisible(feature->isEnabled(pkgName, "back"));
    m_topBtn->setVisible(feature->isEnabled(pkgName, "to_top"));
    if (SessionSettings::getInstance().windowUsePlatformWayland() &&
            !SessionSettings::getInstance().hasWaylandPlasmaWindowManagementSupport()) {
        m_topBtn->setVisible(false);
    }
    m_menuBtn->setVisible(
        feature->isEnabled(pkgName, "settings") || 
        //feature->isEnabled(pkgName, "game_key") || 
        feature->isEnabled(pkgName, "joy_stick") || 
        feature->isEnabled(pkgName, "shack_screen") || 
        //feature->isEnabled(pkgName, "orientation_switch") || 
        feature->isEnabled(pkgName, "screen_record_sharing") || 
        feature->isEnabled(pkgName, "virtual_keyboard") || 
        feature->isEnabled(pkgName, "mobile_data_folder") || 
        //feature->isEnabled(pkgName, "mobile_photo_folder") || 
        //feature->isEnabled(pkgName, "wechat_download_folder") || 
        //feature->isEnabled(pkgName, "qq_download_folder") || 
        //feature->isEnabled(pkgName, "lock_screen") || 
        //feature->isEnabled(pkgName, "scroll_sensitivity") ||
        //feature->isEnabled(pkgName, "close_all_apps") || 
        feature->isEnabled(pkgName, "help") || 
        feature->isEnabled(pkgName, "quit"));
    m_fullscreenBtn->setVisible(feature->isEnabled(pkgName, "full"));
    m_minBtn->setVisible(feature->isEnabled(pkgName, "minimize"));
    m_closeBtn->setVisible(feature->isEnabled(pkgName, "close"));
    m_screenshotBtn->setVisible(feature->isEnabled(pkgName, "screen_shot"));
}

void TitleBar::initConnect()
{
    connect(m_backBtn, &QPushButton::clicked, [this](){emit sigBack();});
    connect(m_fullscreenBtn, &QPushButton::clicked, [this](){emit sigFullscreen();});
    connect(m_closeBtn, &QPushButton::clicked, [this](){emit sigClose();});
    connect(m_minBtn, &QPushButton::clicked, [this]() {emit sigMiniSize();});
    connect(m_menuBtn, &QPushButton::clicked, [this]() {emit sigShowMenu();});
    connect(m_topBtn, &QPushButton::clicked, [this]() {
        m_topBtn->setVisible(false);
        m_cancletopBtn->setVisible(true);
        emit sigToTop(true);
    });
    connect(m_cancletopBtn, &QPushButton::clicked, [this](){
        m_cancletopBtn->setVisible(false);
        m_topBtn->setVisible(true);
        emit sigToTop(false);
    });
    connect(m_screenshotBtn, &QPushButton::clicked, [this]() {emit sigScreenShot();});
//    connect(m_screenHideBtn, &QPushButton::clicked, [this]() {HideMenu->exec(QPoint(width() - 170 + mapToGlobal(QPoint(0,0)).x(),
//                                                                        mapToGlobal(QPoint(0,0)).y() + height() - 5));});

}

void TitleBar::checkScreenshotFeature()
{
    m_hideWhileScreenShotSupported = false;
    if (QFile::exists("/usr/bin/kylin-screenshot")) {
        QDBusInterface interface("org.dharkael.kylinscreenshot", "/", "org.freedesktop.DBus.Introspectable", QDBusConnection::sessionBus());
        QDBusReply<QString> reply = interface.call("Introspect");
        if (reply.isValid()) {
            QString xml_data = reply.value();
            if (xml_data.contains("captureCopy")) {
                m_hideWhileScreenShotSupported = true;
            }
        }
    }
}

// handle window move event
bool TitleBar::event(QEvent *e)
{
    //syslog(LOG_DEBUG, "[TitleBar] event: type = %d", e->type());
    if (e->type() == QEvent::MouseButtonDblClick) {
        emit sigFullscreen();
        return QWidget::event(e);
    }
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        if ((e->type() == QEvent::MouseButtonPress) && (!mMainWindow->getDisplayManager()->isWindowStretchReady())) {
            QMouseEvent* event = static_cast<QMouseEvent*>(e);
            if (event->button() == Qt::LeftButton) {
                mButtonPressed = true;
                mLastPos = event->pos();
            }
        }
        else if ((e->type() == QEvent::MouseMove) && mButtonPressed) {
            QMouseEvent* event = static_cast<QMouseEvent*>(e);
            // add below codes for window can be dragged outside of screen. 
            Display *display = QX11Info::display();
            XEvent xEvent;
            qreal dpiRatio = qApp->devicePixelRatio();
            QPoint pos = this->mapToGlobal(event->pos());

            memset(&xEvent, 0, sizeof(XEvent));
            xEvent.xclient.type = ClientMessage;
            xEvent.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
            xEvent.xclient.display = display;
            xEvent.xclient.window = this->parentWidget()->winId();
            xEvent.xclient.format = 32;
            xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
            xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
            xEvent.xclient.data.l[2] = 8;
            xEvent.xclient.data.l[3] = Button1;
            xEvent.xclient.data.l[4] = 0;

            XUngrabPointer(display, CurrentTime);
            XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
                        False, SubstructureNotifyMask | SubstructureRedirectMask,
                        &xEvent);
            XFlush(display);
        
            if (event->source() == Qt::MouseEventSynthesizedByQt) {// touch move
                if (!this->mouseGrabber()) {
                    this->grabMouse();
                    this->releaseMouse();
                }
            }
            // restore titlebar to get focus
            this->hide();
            this->show();

            //balance mouse release event
            QMouseEvent me(QMouseEvent::MouseButtonRelease, 
                event->pos(), event->windowPos(), event->screenPos(), event->button(), 
                event->buttons(), event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            qApp->sendEvent(this, &me);

            mMainWindow->getDisplayManager()->hideGameKeysWhileMoving();
            mButtonPressed = false;// the event 'MouseButtonRelease' will never be reached! so reset 'mButtonPressed' here.
            return true;
        }
        else if ((e->type() == QEvent::MouseButtonRelease)) {
            mButtonPressed = false;
            mMainWindow->getDisplayManager()->showGameKeysAfterMoving();
        }
    } /* if (SessionSettings::getInstance().windowUsePlatformX11()) { */
    return QWidget::event(e);
}

void TitleBar::waylandMove()
{
    if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
        return;
    }

    m_moveFlag = false;
    if (this->parentWidget()) {
        if (SessionSettings::getInstance().hasWaylandUKUIDecorationSupport()) {
            UKUIDecorationManager::getInstance()->moveWindow(this->parentWidget()->windowHandle());
        } else if (SessionSettings::getInstance().supportsWaylandXdgShell()) {
            XdgManager::getInstance()->moveWindow(this->parentWidget()->windowHandle());
        }
    }
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
        return false;
    }

    if (watched == this) {
        if (event->type() == QEvent::MouseMove){
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                if (m_moveFlag) {
                    waylandMove();
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonPress) {
            // 移动之后不要再移动
            m_moveFlag = true;
        }
    }
    return false;
}
