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
#include "fullbackgroundwidget.h"
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
#include <QPushButton>
#include <QApplication>
#include <QVariant>
#include <QScreen>
#include <QHBoxLayout>
#include <QAbstractAnimation>
#include <QtConcurrent/QtConcurrentRun>
#include <QShortcut>
#include <QAction>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include "authdialogexiteffect.h"
#include "lockmainuianim.h"
#include "poweritemwidget.h"
#include "useritemwidget.h"
#include "mylistwidget.h"
#include "userlistwidget.h"
#include "sessionlistwidget.h"
#include "statusbutton.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "iconedit.h"
#include "commonfunc.h"
#include "global_utils.h"
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
#include "tiplabel.h"
#include <syslog.h>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

namespace {

void drainParallelAnimationGroup(QParallelAnimationGroup *group)
{
    if (!group) {
        return;
    }
    while (group->animationCount() > 0) {
        QAbstractAnimation *anim = group->takeAnimation(0);
        if (anim) {
            anim->deleteLater();
        }
    }
}

} // namespace

#define BLUR_RADIUS 300
#define RIGHT_MARGIN 24
#define BOTTOM_MARGIN 24

#define PREFIX QString(RESOURCE_PATH)
#define IMAGE_DIR PREFIX + "/images/"
float scale;
LockWidget::LockWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    scale = 1.0;
    m_curUserInfo = m_modelLockDialog->findUserByName(
        m_modelLockDialog->defaultUserName().isEmpty() ? getenv("USER") : m_modelLockDialog->defaultUserName());
    initUI();
    initConnections();
    initUsdMediaKeys();
    initPowerManagerKey();
    installEventFilter(this);
}

bool LockWidget::event(QEvent *event)
{
    bool bReturn = QWidget::event(event);
    if (event && event->type() == QEvent::LanguageChange) {
        QLocale local;
        systemLang = local.name();
        // 加载网络插件时会安装语言文件，会触发语言改变信号，qt内部会根据语言名称设置默认布局方向，
        // 因此收到语言变化事件后需重新设置布局方向
        if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
            buttonListWidget->setLayoutDirection(Qt::LeftToRight);
        } else {
            buttonListWidget->setLayoutDirection(Qt::RightToLeft);
        }
    }
    return bReturn;
}

bool LockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            MyListWidget *listWidget = qobject_cast<MyListWidget *>(obj);
            if (listWidget && !listWidget->isHidden()) {
                if (listWidget && listWidget->hasFocus()) { // Tab键切入时，设置焦点状态
                    listWidget->setCurrentRow(
                        listWidget->currentRow(), QItemSelectionModel::SelectionFlag::SelectCurrent);
                    listWidget->currentItemChanged(listWidget->currentItem(), nullptr);
                } else { // Tab键切出时，清空焦点状态
                    listWidget->setCurrentRow(listWidget->currentRow(), QItemSelectionModel::SelectionFlag::Clear);
                }
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        /// TODO
        if ((m_networkWidget && m_networkWidget == obj) || (m_userListWidget && m_userListWidget == obj)
            || (m_sessionListWidget && m_sessionListWidget == obj)
            || (m_virtualKeyboardWidget && m_virtualKeyboardWidget == obj) || m_isInhibitStatus) {
            return true;
        } else if (buttonListWidget && obj != buttonListWidget) {
            exitSubWidget(false, false, false);
        }
    } else if (event->type() == QEvent::Show) {
        if (obj == this) {
            doDelayInit();
            if (authDialog && m_powerListWidget && m_powerListWidget->isHidden()) {
                authDialog->inhibitShow(false);
            }
        }
    } else if (event->type() == QEvent::Hide) {
        if (obj == this) {
            if (authDialog && m_powerListWidget && !m_powerListWidget->isHidden()) {
                authDialog->inhibitShow(true);
            }
        }
	    if (m_blockWidget && obj == m_blockWidget && (!m_blockWidget->isVisible() || !m_blockWidget->isHidden()) && !m_blockSetClosePhase) {
            if (m_inhibitType.contains(QStringLiteral("Reboot")) || m_inhibitType.contains(QStringLiteral("PowerOff")) || m_inhibitType.contains(QStringLiteral("Logout"))) {
                m_alreadyShowMulUsers = false;
                Q_EMIT m_modelLockDialog->setClosePhase(false);
            }
        }
    }

    return false;
}

void LockWidget::releaseResources()
{
#if SUPPORT_NET_ICON
    if (m_networkButton) {
        if (buttonListWidget && m_btnItemNetwork) {
            buttonListWidget->removeItemWidget(m_btnItemNetwork);
        }
        m_networkButton->hide();
        m_networkButton->deleteLater();
        m_networkButton = nullptr;
    }
#endif // SUPPORT_NET_ICON
}

bool LockWidget::exitSubWidget(bool isForScreensaver, bool hideVirkeyboard, bool hideCenterWidget)
{
    m_isManualPower = false;
    qDebug() << "exitSubWidget:" << isForScreensaver << hideVirkeyboard << m_modelLockDialog->isSessionTools();
    if (m_modelLockDialog->isSessionTools()) {
        authDialog->setFocus();
        authDialog->inhibitShow(false);
        buttonListWidget->show();
        m_timeWidget->setVisible(m_isShowDateTime);
        if (m_powerListWidget) {
            m_powerListWidget->hide();
        }
        if (m_systemMonitorBtn && !m_systemMonitorBtn->isHidden()) {
            m_systemMonitorBtn->hide();
        }
        if (m_blockWidget) {
            m_blockWidget->hide();
        }
        if (m_powerManagerButton) {
            m_powerManagerButton->setClickedStatus(NORMAL);
        }
        m_modelLockDialog->setIsSessionTools(false);
        if (!isForScreensaver) {
            Q_EMIT sessionToolsExit();
        }
        return false;
    }
    bool allExited = true;
    if (m_userListWidget && !m_userListWidget->isHidden()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
        allExited = false;
    }
    if (hideCenterWidget && m_blockWidget && !m_blockWidget->isHidden()) {
        m_blockWidget->hide();
        m_timeWidget->show();
        if (m_powerListWidget && m_powerListWidget->isHidden()) {
            m_powerListWidget->show();
        }
        buttonListWidget->show();
        allExited = false;
    }
    if (m_powerListWidget && !m_powerListWidget->isHidden()) {
        if (m_isShowResetPasswdBtn && m_pResetPwdBtn) {
            m_pResetPwdBtn->show();
        }
        m_powerListWidget->hide();
        buttonListWidget->show();
        if (m_systemMonitorBtn && !m_systemMonitorBtn->isHidden()) {
            m_systemMonitorBtn->hide();
        }
        if (m_powerManagerButton) {
            m_powerManagerButton->setClickedStatus(NORMAL);
        }
        authDialog->inhibitShow(false);
        m_timeWidget->setVisible(m_isShowDateTime);
        allExited = false;
    }
    if (m_networkWidget && !m_networkWidget->isHidden()) {
        m_networkWidget->hide();
#if SUPPORT_NET_ICON
        if (m_networkButton) {
            m_networkButton->setClickedStatus(NORMAL);
        }
#endif // SUPPORT_NET_ICON
        allExited = false;
    }
    if (hideVirkeyboard && m_virtualKeyboardWidget && !m_virtualKeyboardWidget->isHidden()) {
        m_virtualKeyboardWidget->hide();
        if (m_virKbButton) {
            m_virKbButton->setClickedStatus(NORMAL);
        }
        allExited = false;
    }
    if (batteryWidget && !batteryWidget->isHidden()) {
        batteryWidget->hide();
        if (m_batteryonButton) {
            m_batteryonButton->setClickedStatus(NORMAL);
        }
        allExited = false;
    }
    if (m_sessionListWidget && !m_sessionListWidget->isHidden()) {
        m_sessionListWidget->hide();
        if (m_sessionButton) {
            m_sessionButton->setClickedStatus(NORMAL);
        }
        allExited = false;
    }
    if (hideCenterWidget && m_pResetPwdWidget && !m_pResetPwdWidget->isHidden()) {
        m_pResetPwdWidget->hide();
        m_pResetPwdWidget->switchToConfidentialDig();
        m_timeWidget->setVisible(m_isShowDateTime);
        buttonListWidget->show();
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
    QWidget::paintEvent(event);
}

void LockWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    if (m_widgetSize == size) {
        return;
    }
    m_widgetSize = size;
    // 重新计算缩放比例
    scale = QString::number(size.width() / 1920.0, 'f', 1).toFloat();

    m_timeWidget->updateUI();
    if (scale <= 0.55) {
        m_timeWidget->move((width() - m_timeWidget->width()) / 2, 12 * scale);
        authDialog->setGeometry(
            (width() - this->geometry().width()) / 2,
            160 * scale,
            this->width(),
            (height() - 160 * scale - (24 + 48 + 16) * scale));
        authDialog->updateAuthSize();
    } else {
        m_timeWidget->move((width() - m_timeWidget->width()) / 2, 48 * scale);
        authDialog->setGeometry(
            (width() - this->geometry().width()) / 2,
            100 * scale,
            this->width(),
            (height() - 150 * scale - (24 + 48 + 16) * scale));
        authDialog->updateAuthSize();
    }
    if (m_userListWidget) {
        m_userListWidget->setMaxHeight(height() / 2);
        m_userListWidget->updateWidgetSize();
        qDebug() << "m_userListWidget:" << m_userListWidget->geometry();
    }
    if (m_powerManagerButton) {
        m_powerManagerButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("system-shutdown-symbolic", QIcon(":/image/assets/shutdown.svg")).pixmap(24, 24),
            "white"));
    }
    if (m_virKbButton) {
        m_virKbButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("input-keyboard-symbolic", QIcon(":/image/assets/keyboard.svg")).pixmap(24, 24), "white"));
    }
    if (m_userButton) {
        m_userButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("system-users-symbolic", QIcon(":/image/assets/switchuser.svg")).pixmap(24, 24), "white"));
    }
    if (m_powerListWidget) {
        if (m_systemMonitorBtn) {
            m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height() + m_systemMonitorBtn->height()));
        } else {
            m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height()));
        }
        m_powerListWidget->updateWidgetSize(rect());

        if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
            m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
        } else { // 居中显示
            m_powerListWidget->move(
                (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
        }
    }

    updateBottomButton();
    buttonListWidget->setFixedSize(QSize((48 + buttonListWidget->spacing() * 2) * buttonListWidget->count(), 64));
    m_pBottomWidget->setGeometry(0, height() - buttonListWidget->height() - BOTTOM_MARGIN * scale, this->width(), 64);
    if (m_blockWidget) {
        m_blockWidget->setGeometry(0, 0, width(), height());
    }
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->setGeometry(
            (this->width() - m_pResetPwdBtn->width()) / 2,
            height() - m_pResetPwdBtn->height() - BOTTOM_MARGIN * scale + 8,
            m_pResetPwdBtn->width(),
            m_pResetPwdBtn->height());
    }

    if (m_pKylinIDCodeTip) {
        m_pKylinIDCodeTip->move((this->width() - m_pKylinIDCodeTip->width()) / 2, height() - 72);
    }

    if (m_pResetPwdWidget) {
        m_pResetPwdWidget->updateResetPwdSize();
        m_pResetPwdWidget->setFixedWidth(width());
        m_pResetPwdWidget->setGeometry(this->geometry());
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
    cacheMainUiTargetGeometry();
}

void LockWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_K) {
        if (e->modifiers() & Qt::ControlModifier)
            onShowVirtualKeyboard();
    }

    if (e->key() == Qt::Key_Return && (m_systemMonitorBtn && m_systemMonitorBtn->hasFocus())) {
        doSystemMonitor();
        exitSubWidget();
    }

    // 监听按键事件实现快捷键功能
    switch (e->key()) {
        case Qt::Key_VolumeMute:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(MUTE_KEY);
            break;
        case Qt::Key_TouchpadOn:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(TOUCHPAD_ON_KEY);
            break;
        case Qt::Key_TouchpadOff:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(TOUCHPAD_OFF_KEY);
            break;
        case Qt::Key_TouchpadToggle:
            Q_EMIT m_modelLockDialog->usdExternalDoAction(TOUCHPAD_KEY);
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

    if (e->key() == Qt::Key_PowerOff) {
        if (m_strPowerButtonKey == "interactive") {
            if (!m_modelLockDialog->isSessionTools()) {
                m_isManualPower = true;
                onShowPowerListWidget();
            }
        } else if (m_strPowerButtonKey == "shutdown") {
            if (!m_modelLockDialog->isSessionTools() && m_modelLockDialog->getLoggedInUsersCount() >= 1
                && m_modelLockDialog->getShutdownLockcheck().isEmpty()) {
                onSureShutDown(SHUTDOWN);
            } else {
                if (!m_modelLockDialog->getIsLegacy()) {
                    onDoPowerAction("PowerOff", true);
                } else {
                    m_modelLockDialog->setPowerManager(false, "PowerOff");
                }
            }
        } else if (m_strPowerButtonKey == "hibernate") {
            m_modelLockDialog->setPowerManager(false, "Hibernate");
        } else if (m_strPowerButtonKey == "suspend") {
            onSuspendClicked("Suspend");
        }
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
    m_isShowDateTime = m_modelLockDialog->getLockDatetimeEnable();
    QLocale local;
    systemLang = local.name();
    initTimeWidget();
    initButtonWidget();
    initSystemMonitorBtn();
    cacheMainUiTargetGeometry();
    //    authDialog->setFocus();
    //    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
}

void LockWidget::cacheMainUiTargetGeometry()
{
    if (m_timeWidget) {
        m_timeTargetGeometry = m_timeWidget->geometry();
    }
    if (authDialog) {
        m_authTargetGeometry = authDialog->geometry();
    }
    if (m_pBottomWidget) {
        m_bottomTargetGeometry = m_pBottomWidget->geometry();
    }
}

/** 清理认证子控件上的透明度/退场效果。进场时由 AuthDialog::beginLockEnterRowAnimation 临时移出行再 setGeometry，结束后 endLockEnterRowAnimation 恢复布局。 */
static void clearAuthSubWidgetEffects(AuthDialog *dlg)
{
    if (!dlg) {
        return;
    }
    if (QWidget *w = dlg->headerWidget()) {
        w->setGraphicsEffect(nullptr);
    }
    if (QWidget *w = dlg->nameWidget()) {
        w->setGraphicsEffect(nullptr);
    }
    if (QWidget *w = dlg->editWidget()) {
        w->setGraphicsEffect(nullptr);
    }
    if (QWidget *w = dlg->loginOptionsWidget()) {
        w->setGraphicsEffect(nullptr);
    }
    if (QWidget *w = dlg->tipWidget()) {
        w->setGraphicsEffect(nullptr);
    }
    if (QWidget *w = dlg->ukeyPasswordWidget()) {
        w->setGraphicsEffect(nullptr);
    }
}

QGraphicsOpacityEffect *LockWidget::ensureOpacityEffect(QWidget *widget)
{
    if (!widget) {
        return nullptr;
    }
    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    return effect;
}

void LockWidget::stopMainUiAnimations()
{
    if (m_mainUiEnterAnimation) {
        disconnect(m_mainUiEnterAnimation, &QParallelAnimationGroup::finished, this, nullptr);
        m_mainUiEnterAnimation->stop();
        drainParallelAnimationGroup(m_mainUiEnterAnimation);
    }
    if (m_mainUiExitAnimation) {
        disconnect(m_mainUiExitAnimation, &QParallelAnimationGroup::finished, this, nullptr);
        m_mainUiExitAnimation->stop();
        drainParallelAnimationGroup(m_mainUiExitAnimation);
    }
    if (authDialog) {
        authDialog->endLockEnterRowAnimation();
        authDialog->endLockLoginOptsEnterAnimation();
    }
    if (auto *bg = qobject_cast<FullBackgroundWidget *>(parentWidget())) {
        bg->endAuthUiWallpaperSnapshot();
    }
}

void LockWidget::hookAnimationForCoordinatedRepaint(QPropertyAnimation *animation)
{
    if (!animation) {
        return;
    }
    connect(animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &) {
        auto *bg = qobject_cast<FullBackgroundWidget *>(parentWidget());
        if (!bg) {
            update();
            return;
        }
        if (bg->isWallpaperAnimActive()) {
            bg->scheduleWallpaperRepaint();
            update();
            return;
        }
        if (bg->isAuthUiWallpaperSnapshotActive()) {
            update();
            return;
        }
        bg->update();
    });
}

void LockWidget::beginAuthUiWallpaperSnapshotIfNeeded()
{
    auto *bg = qobject_cast<FullBackgroundWidget *>(parentWidget());
    if (!bg || bg->isWallpaperAnimActive() || bg->isAuthUiWallpaperSnapshotActive()) {
        return;
    }
    bg->beginAuthUiWallpaperSnapshot();
}

void LockWidget::applyMainUiVisualMode(MainUiVisualMode mode)
{
    stopMainUiAnimations();

    if (!m_timeWidget || !authDialog || !m_pBottomWidget) {
        return;
    }

    switch (mode) {
        case MainUiVisualMode::StableWithGeometryReset:
            m_timeWidget->setGeometry(m_timeTargetGeometry);
            authDialog->setGeometry(m_authTargetGeometry);
            m_pBottomWidget->setGeometry(m_bottomTargetGeometry);
            break;
        case MainUiVisualMode::StableKeepGeometry:
            break;
        case MainUiVisualMode::EnterFromScreensaverStart: {
            const int topOffset = qRound(LockMainUiAnim::kEnterTopOffsetBase * scale);
            const int bottomOffset = qRound(LockMainUiAnim::kEnterBottomOffsetBase * scale);

            auto *timeOpacity = ensureOpacityEffect(m_timeWidget);
            auto *authOpacity = ensureOpacityEffect(authDialog);
            auto *bottomOpacity = ensureOpacityEffect(m_pBottomWidget);
            if (!timeOpacity || !authOpacity || !bottomOpacity) {
                return;
            }

            QRect timeStart = m_timeTargetGeometry;
            timeStart.moveTop(m_timeTargetGeometry.y() - topOffset);
            m_timeWidget->setGeometry(timeStart);
            timeOpacity->setOpacity(0.0);

            /* 认证区垂直位移由头像/名称/输入行单独动画承担，整块保持目标 geometry，避免与行 y 动画叠加 */
            authDialog->setGeometry(m_authTargetGeometry);
            authOpacity->setOpacity(0.0);

            QRect bottomStart = m_bottomTargetGeometry;
            bottomStart.moveTop(m_bottomTargetGeometry.y() + bottomOffset);
            m_pBottomWidget->setGeometry(bottomStart);
            bottomOpacity->setOpacity(0.0);
            break;
        }
    }

    if (mode != MainUiVisualMode::EnterFromScreensaverStart) {
        m_timeWidget->setGraphicsEffect(nullptr);
        authDialog->setGraphicsEffect(nullptr);
        m_pBottomWidget->setGraphicsEffect(nullptr);
    }
    clearAuthSubWidgetEffects(authDialog);
}

void LockWidget::resetMainUiVisualState(bool resetGeometry)
{
    applyMainUiVisualMode(
        resetGeometry ? MainUiVisualMode::StableWithGeometryReset : MainUiVisualMode::StableKeepGeometry);
}

void LockWidget::prepareMainUiEnterStartState()
{
    if (!m_timeWidget || !authDialog || !m_pBottomWidget || !buttonListWidget) {
        return;
    }
    applyMainUiVisualMode(MainUiVisualMode::EnterFromScreensaverStart);
}

void LockWidget::startMainUiEnterAnimation()
{
    if (!m_timeWidget || !authDialog || !m_pBottomWidget || !buttonListWidget) {
        return;
    }

    if (isGreeterMode()) {
        return;
    }

    clearAuthSubWidgetEffects(authDialog);

    stopMainUiAnimations();
    if (!m_mainUiEnterAnimation) {
        m_mainUiEnterAnimation = new QParallelAnimationGroup(this);
    }

    const int duration = LockMainUiAnim::kDurationMs;
    const QEasingCurve curve = LockMainUiAnim::exitEasingCurve();
    const int topOffset = qRound(LockMainUiAnim::kEnterTopOffsetBase * scale);
    const int authRowOffset = qRound(LockMainUiAnim::kEnterAuthOffsetBase * scale);
    const int bottomOffset = qRound(LockMainUiAnim::kEnterBottomOffsetBase * scale);

    buttonListWidget->show();
    m_pBottomWidget->show();
    authDialog->show();
    if (m_isShowDateTime) {
        m_timeWidget->show();
    }

    auto *timeOpacity = ensureOpacityEffect(m_timeWidget);
    auto *bottomOpacity = ensureOpacityEffect(m_pBottomWidget);
    if (!timeOpacity || !bottomOpacity) {
        return;
    }

    QRect timeStart = m_timeWidget->geometry();
    if (timeStart == m_timeTargetGeometry) {
        timeStart.moveTop(m_timeTargetGeometry.y() - topOffset);
        m_timeWidget->setGeometry(timeStart);
        timeOpacity->setOpacity(0.0);
    }

    authDialog->setGraphicsEffect(nullptr);
    authDialog->setGeometry(m_authTargetGeometry);

    QWidget *wHeader = authDialog->headerWidget();
    QWidget *wName = authDialog->nameWidget();
    QWidget *wEdit = authDialog->editWidget();
    QWidget *wUkey = authDialog->ukeyPasswordWidget();
    QWidget *wTip = authDialog->tipWidget();
    QWidget *wOpts = authDialog->loginOptionsWidget();
    const bool ukeyInEnter = wUkey && wUkey->isVisible() && wUkey->height() > 0;
    const bool loginOptsInEnter = wOpts && wOpts->isVisible() && wOpts->height() > 0;
    auto *opHeader = wHeader ? ensureOpacityEffect(wHeader) : nullptr;
    auto *opName = wName ? ensureOpacityEffect(wName) : nullptr;
    auto *opEdit = wEdit ? ensureOpacityEffect(wEdit) : nullptr;
    auto *opUkey = ukeyInEnter ? ensureOpacityEffect(wUkey) : nullptr;
    auto *opTip = wTip ? ensureOpacityEffect(wTip) : nullptr;
    auto *opOpts = wOpts ? ensureOpacityEffect(wOpts) : nullptr;
    if (!opHeader || !opName || !opEdit || !opTip || !opOpts || (ukeyInEnter && !opUkey)) {
        return;
    }
    opHeader->setOpacity(0.0);
    opName->setOpacity(0.0);
    opEdit->setOpacity(0.0);
    if (ukeyInEnter) {
        opUkey->setOpacity(0.0);
    }
    opTip->setOpacity(0.0);
    opOpts->setOpacity(0.0);

    authDialog->beginLockEnterRowAnimation(authRowOffset);
    if (loginOptsInEnter) {
        authDialog->beginLockLoginOptsEnterAnimation(authRowOffset);
    }

    QRect bottomStart = m_pBottomWidget->geometry();
    if (bottomStart == m_bottomTargetGeometry) {
        bottomStart.moveTop(m_bottomTargetGeometry.y() + bottomOffset);
        m_pBottomWidget->setGeometry(bottomStart);
        bottomOpacity->setOpacity(0.0);
    }

    auto *timeMove = new QPropertyAnimation(m_timeWidget, "geometry");
    timeMove->setDuration(duration);
    timeMove->setEasingCurve(curve);
    timeMove->setStartValue(m_timeWidget->geometry());
    timeMove->setEndValue(m_timeTargetGeometry);
    m_mainUiEnterAnimation->addAnimation(timeMove);
    hookAnimationForCoordinatedRepaint(timeMove);

    auto *timeFade = new QPropertyAnimation(timeOpacity, "opacity");
    timeFade->setDuration(duration);
    timeFade->setEasingCurve(curve);
    timeFade->setStartValue(timeOpacity->opacity());
    timeFade->setEndValue(m_isShowDateTime ? 1.0 : 0.0);
    m_mainUiEnterAnimation->addAnimation(timeFade);
    hookAnimationForCoordinatedRepaint(timeFade);

    auto addAuthRowEnter = [this, duration, curve, authRowOffset](QWidget *w, QGraphicsOpacityEffect *op, int delayMs) {
        if (!w || !op) {
            return;
        }
        const QRect g0 = w->geometry();
        const QRect g1 = g0.translated(0, authRowOffset);

        auto *seq = new QSequentialAnimationGroup(m_mainUiEnterAnimation);
        if (delayMs > 0) {
            seq->addPause(delayMs);
        }
        auto *inner = new QParallelAnimationGroup(seq);

        auto *move = new QPropertyAnimation(w, "geometry", inner);
        move->setDuration(duration);
        move->setEasingCurve(curve);
        move->setStartValue(g0);
        move->setEndValue(g1);
        inner->addAnimation(move);

        auto *fade = new QPropertyAnimation(op, "opacity", inner);
        fade->setDuration(duration);
        fade->setEasingCurve(curve);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        inner->addAnimation(fade);
        hookAnimationForCoordinatedRepaint(move);
        hookAnimationForCoordinatedRepaint(fade);

        seq->addAnimation(inner);
        m_mainUiEnterAnimation->addAnimation(seq);
    };
    addAuthRowEnter(wHeader, opHeader, 0);
    addAuthRowEnter(wName, opName, LockMainUiAnim::kAuthEnterNameDelayMs);
    addAuthRowEnter(wEdit, opEdit, LockMainUiAnim::kAuthEnterEditDelayMs);
    if (ukeyInEnter) {
        addAuthRowEnter(wUkey, opUkey, LockMainUiAnim::kAuthEnterEditDelayMs);
    }
    if (loginOptsInEnter) {
        addAuthRowEnter(wOpts, opOpts, LockMainUiAnim::kAuthEnterEditDelayMs);
    }

    auto addAuthAuxFadeEnter = [this, duration, curve](QGraphicsOpacityEffect *op) {
        if (!op) {
            return;
        }
        auto *fade = new QPropertyAnimation(op, "opacity");
        fade->setDuration(duration);
        fade->setEasingCurve(curve);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        m_mainUiEnterAnimation->addAnimation(fade);
        hookAnimationForCoordinatedRepaint(fade);
    };
    addAuthAuxFadeEnter(opTip);
    if (!loginOptsInEnter) {
        addAuthAuxFadeEnter(opOpts);
    }

    auto *bottomMove = new QPropertyAnimation(m_pBottomWidget, "geometry");
    bottomMove->setDuration(duration);
    bottomMove->setEasingCurve(curve);
    bottomMove->setStartValue(m_pBottomWidget->geometry());
    bottomMove->setEndValue(m_bottomTargetGeometry);
    m_mainUiEnterAnimation->addAnimation(bottomMove);
    hookAnimationForCoordinatedRepaint(bottomMove);

    auto *bottomFade = new QPropertyAnimation(bottomOpacity, "opacity");
    bottomFade->setDuration(duration);
    bottomFade->setEasingCurve(curve);
    bottomFade->setStartValue(bottomOpacity->opacity());
    bottomFade->setEndValue(1.0);
    m_mainUiEnterAnimation->addAnimation(bottomFade);
    hookAnimationForCoordinatedRepaint(bottomFade);

    connect(m_mainUiEnterAnimation, &QParallelAnimationGroup::finished, this, [this]() {
        if (m_timeWidget) {
            m_timeWidget->setGeometry(m_timeTargetGeometry);
        }
        if (authDialog) {
            authDialog->endLockEnterRowAnimation();
            authDialog->endLockLoginOptsEnterAnimation();
            authDialog->setGeometry(m_authTargetGeometry);
        }
        if (m_pBottomWidget) {
            m_pBottomWidget->setGeometry(m_bottomTargetGeometry);
        }
        if (auto *timeEffect = qobject_cast<QGraphicsOpacityEffect *>(m_timeWidget ? m_timeWidget->graphicsEffect() : nullptr)) {
            timeEffect->setOpacity(m_isShowDateTime ? 1.0 : 0.0);
        }
        if (authDialog) {
            authDialog->setGraphicsEffect(nullptr);
        }
        if (auto *bottomEffect = qobject_cast<QGraphicsOpacityEffect *>(m_pBottomWidget ? m_pBottomWidget->graphicsEffect() : nullptr)) {
            bottomEffect->setOpacity(1.0);
        }
        clearAuthSubWidgetEffects(authDialog);
        if (auto *bg = qobject_cast<FullBackgroundWidget *>(parentWidget())) {
            bg->endAuthUiWallpaperSnapshot();
        }
    });

    beginAuthUiWallpaperSnapshotIfNeeded();
    m_mainUiEnterAnimation->start();
}

void LockWidget::applyMainUiExitFinishedState()
{
    if (m_timeWidget) {
        m_timeWidget->setGeometry(m_timeTargetGeometry);
    }
    if (authDialog) {
        authDialog->setGeometry(m_authTargetGeometry);
    }
    if (m_pBottomWidget) {
        m_pBottomWidget->setGeometry(m_bottomTargetGeometry);
    }
    if (auto *timeEffect = qobject_cast<QGraphicsOpacityEffect *>(m_timeWidget ? m_timeWidget->graphicsEffect() : nullptr)) {
        timeEffect->setOpacity(m_isShowDateTime ? 1.0 : 0.0);
    }
    if (authDialog) {
        authDialog->setGraphicsEffect(nullptr);
        if (auto *authOpacity = ensureOpacityEffect(authDialog)) {
            authOpacity->setOpacity(1.0);
        }
    }
    if (auto *bottomEffect = qobject_cast<QGraphicsOpacityEffect *>(m_pBottomWidget ? m_pBottomWidget->graphicsEffect() : nullptr)) {
        bottomEffect->setOpacity(1.0);
    }
    clearAuthSubWidgetEffects(authDialog);
}

void LockWidget::startMainUiExitAnimation()
{
    if (!m_timeWidget || !authDialog || !m_pBottomWidget || !buttonListWidget) {
        Q_EMIT mainUiExitAnimationFinished();
        return;
    }

    if (isGreeterMode()) {
        applyMainUiExitFinishedState();
        Q_EMIT mainUiExitAnimationFinished();
        return;
    }

    clearAuthSubWidgetEffects(authDialog);

    stopMainUiAnimations();
    if (!m_mainUiExitAnimation) {
        m_mainUiExitAnimation = new QParallelAnimationGroup(this);
    }

    const int duration = LockMainUiAnim::kDurationMs;
    QEasingCurve curve = LockMainUiAnim::exitEasingCurve();
    const int topOffset = qRound(LockMainUiAnim::kExitTopOffsetBase * scale);
    const int bottomOffset = qRound(LockMainUiAnim::kExitBottomOffsetBase * scale);

    auto *timeOpacity = ensureOpacityEffect(m_timeWidget);
    auto *bottomOpacity = ensureOpacityEffect(m_pBottomWidget);
    if (!timeOpacity || !bottomOpacity) {
        Q_EMIT mainUiExitAnimationFinished();
        return;
    }

    QRect timeEnd = m_timeTargetGeometry;
    timeEnd.moveTop(m_timeTargetGeometry.y() - topOffset);

    QRect bottomEnd = m_bottomTargetGeometry;
    bottomEnd.moveTop(m_bottomTargetGeometry.y() + bottomOffset);

    auto *timeMove = new QPropertyAnimation(m_timeWidget, "geometry");
    timeMove->setDuration(duration);
    timeMove->setEasingCurve(curve);
    timeMove->setStartValue(m_timeWidget->geometry());
    timeMove->setEndValue(timeEnd);
    m_mainUiExitAnimation->addAnimation(timeMove);
    hookAnimationForCoordinatedRepaint(timeMove);

    auto *timeFade = new QPropertyAnimation(timeOpacity, "opacity");
    timeFade->setDuration(duration);
    timeFade->setEasingCurve(curve);
    timeFade->setStartValue(timeOpacity->opacity());
    timeFade->setEndValue(0.0);
    m_mainUiExitAnimation->addAnimation(timeFade);
    hookAnimationForCoordinatedRepaint(timeFade);

    const int authRowOffset = qRound(LockMainUiAnim::kEnterAuthOffsetBase * scale);
    QWidget *wHeader = authDialog->headerWidget();
    QWidget *wName = authDialog->nameWidget();
    QWidget *wEdit = authDialog->editWidget();
    QWidget *wUkey = authDialog->ukeyPasswordWidget();
    QWidget *wTip = authDialog->tipWidget();
    QWidget *wLoginOpts = authDialog->loginOptionsWidget();
    const bool ukeyInExit = wUkey && wUkey->isVisible() && wUkey->height() > 0;
    const bool loginOptsInExit = wLoginOpts && wLoginOpts->isVisible() && wLoginOpts->height() > 0;

    authDialog->beginLockEnterRowAnimation(authRowOffset);
    if (loginOptsInExit) {
        authDialog->beginLockLoginOptsEnterAnimation(authRowOffset);
    }
    auto snapDetachedRowToTarget = [authRowOffset](QWidget *w) {
        if (w) {
            w->setGeometry(w->geometry().translated(0, authRowOffset));
        }
    };
    snapDetachedRowToTarget(wHeader);
    snapDetachedRowToTarget(wName);
    snapDetachedRowToTarget(wEdit);
    if (ukeyInExit) {
        snapDetachedRowToTarget(wUkey);
    }
    if (loginOptsInExit) {
        snapDetachedRowToTarget(wLoginOpts);
    }

    /* 头像/名称/输入：下移 + 1.2 放大 + 淡出并行（无错峰）；登录选项仅下移 + 淡出 */
    auto addAuthRowExitScaled = [this, duration, curve, authRowOffset](
        QWidget *w, AuthDialogExitEffect::ScaleAnchor anchor) {
        if (!w || !w->isVisible() || w->height() <= 0) {
            return;
        }
        auto *eff = new AuthDialogExitEffect(w);
        eff->setScaleAnchor(anchor);
        eff->setExitOpacity(1.0);
        eff->setExitScale(1.0);
        w->setGraphicsEffect(eff);

        const QRect g0 = w->geometry();
        const QRect g1 = g0.translated(0, authRowOffset);

        auto *inner = new QParallelAnimationGroup(m_mainUiExitAnimation);

        auto *move = new QPropertyAnimation(w, "geometry", inner);
        move->setDuration(duration);
        move->setEasingCurve(curve);
        move->setStartValue(g0);
        move->setEndValue(g1);
        inner->addAnimation(move);

        auto *scaleAnim = new QPropertyAnimation(eff, "exitScale", inner);
        scaleAnim->setDuration(LockMainUiAnim::kAuthExitScaleDurationMs);
        scaleAnim->setEasingCurve(curve);
        scaleAnim->setStartValue(1.0);
        scaleAnim->setEndValue(LockMainUiAnim::kAuthExitScaleEnd);
        inner->addAnimation(scaleAnim);

        auto *fadeAnim = new QPropertyAnimation(eff, "exitOpacity", inner);
        fadeAnim->setDuration(duration);
        fadeAnim->setEasingCurve(curve);
        fadeAnim->setStartValue(1.0);
        fadeAnim->setEndValue(0.0);
        inner->addAnimation(fadeAnim);

        hookAnimationForCoordinatedRepaint(move);
        hookAnimationForCoordinatedRepaint(scaleAnim);
        hookAnimationForCoordinatedRepaint(fadeAnim);

        m_mainUiExitAnimation->addAnimation(inner);
    };

    auto addAuthRowExitFadeMove = [this, duration, curve, authRowOffset](QWidget *w) {
        if (!w || !w->isVisible() || w->height() <= 0) {
            return;
        }
        auto *op = ensureOpacityEffect(w);
        if (!op) {
            return;
        }
        const QRect g0 = w->geometry();
        const QRect g1 = g0.translated(0, authRowOffset);

        auto *inner = new QParallelAnimationGroup(m_mainUiExitAnimation);

        auto *move = new QPropertyAnimation(w, "geometry", inner);
        move->setDuration(duration);
        move->setEasingCurve(curve);
        move->setStartValue(g0);
        move->setEndValue(g1);
        inner->addAnimation(move);

        auto *fade = new QPropertyAnimation(op, "opacity", inner);
        fade->setDuration(duration);
        fade->setEasingCurve(curve);
        fade->setStartValue(op->opacity());
        fade->setEndValue(0.0);
        inner->addAnimation(fade);

        hookAnimationForCoordinatedRepaint(move);
        hookAnimationForCoordinatedRepaint(fade);

        m_mainUiExitAnimation->addAnimation(inner);
    };

    addAuthRowExitScaled(wHeader, AuthDialogExitEffect::ScaleAnchor::BottomCenter);
    addAuthRowExitScaled(wName, AuthDialogExitEffect::ScaleAnchor::Center);
    addAuthRowExitScaled(wEdit, AuthDialogExitEffect::ScaleAnchor::TopCenter);
    if (ukeyInExit) {
        addAuthRowExitScaled(wUkey, AuthDialogExitEffect::ScaleAnchor::TopCenter);
    }
    if (loginOptsInExit) {
        addAuthRowExitFadeMove(wLoginOpts);
    }

    if (wTip) {
        if (wTip->isVisible() && wTip->height() > 0) {
            if (auto *tipOp = ensureOpacityEffect(wTip)) {
            auto *tipFade = new QPropertyAnimation(tipOp, "opacity");
            tipFade->setDuration(duration);
            tipFade->setEasingCurve(curve);
            tipFade->setStartValue(tipOp->opacity());
            tipFade->setEndValue(0.0);
            m_mainUiExitAnimation->addAnimation(tipFade);
            hookAnimationForCoordinatedRepaint(tipFade);
            }
        }
    }
    if (wLoginOpts && (!wLoginOpts->isVisible() || wLoginOpts->height() <= 0)) {
        if (auto *optsOp = ensureOpacityEffect(wLoginOpts)) {
            auto *optsFade = new QPropertyAnimation(optsOp, "opacity");
            optsFade->setDuration(duration);
            optsFade->setEasingCurve(curve);
            optsFade->setStartValue(optsOp->opacity());
            optsFade->setEndValue(0.0);
            m_mainUiExitAnimation->addAnimation(optsFade);
            hookAnimationForCoordinatedRepaint(optsFade);
        }
    }

    auto *bottomMove = new QPropertyAnimation(m_pBottomWidget, "geometry");
    bottomMove->setDuration(duration);
    bottomMove->setEasingCurve(curve);
    bottomMove->setStartValue(m_pBottomWidget->geometry());
    bottomMove->setEndValue(bottomEnd);
    m_mainUiExitAnimation->addAnimation(bottomMove);
    hookAnimationForCoordinatedRepaint(bottomMove);

    auto *bottomFade = new QPropertyAnimation(bottomOpacity, "opacity");
    bottomFade->setDuration(duration);
    bottomFade->setEasingCurve(curve);
    bottomFade->setStartValue(bottomOpacity->opacity());
    bottomFade->setEndValue(0.0);
    m_mainUiExitAnimation->addAnimation(bottomFade);
    hookAnimationForCoordinatedRepaint(bottomFade);

    connect(m_mainUiExitAnimation, &QParallelAnimationGroup::finished, this, [this]() {
        if (authDialog) {
            authDialog->endLockEnterRowAnimation();
            authDialog->endLockLoginOptsEnterAnimation();
        }
        applyMainUiExitFinishedState();
        if (auto *bg = qobject_cast<FullBackgroundWidget *>(parentWidget())) {
            bg->endAuthUiWallpaperSnapshot();
        }
        Q_EMIT mainUiExitAnimationFinished();
    });

    beginAuthUiWallpaperSnapshotIfNeeded();
    m_mainUiExitAnimation->start();
}

void LockWidget::onFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(now);

    if (!old) {
        return;
    }

    MyListWidget *oldListWidget = qobject_cast<MyListWidget *>(old);
    if (!oldListWidget) {
        return;
    }
    oldListWidget->resetSelectionState();
}

void LockWidget::initConnections()
{
    connect(qApp, &QApplication::focusChanged, this, &LockWidget::onFocusChanged);
    connect(UniAuthService::instance(), &UniAuthService::userChanged, this, &LockWidget::onServiceUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::usersInfoChanged, this, &LockWidget::onUsersInfoChanged);
    connect(m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &LockWidget::onCurUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::currentSessionChanged, this, &LockWidget::onSessionChanged);
    connect(m_modelLockDialog, &LockDialogModel::sessionActiveChanged, this, &LockWidget::onSessionActiveChanged);
    connect(m_modelLockDialog, &LockDialogModel::tabletModeChanged, this, &LockWidget::onTabletModeChanged);
    connect(m_modelLockDialog, &LockDialogModel::fontChanged, this, &LockWidget::onFontChanged);
    connect(m_modelLockDialog, &LockDialogModel::fontSizeChanged, this, &LockWidget::onFontSizeChanged);
    connect(m_modelLockDialog, &LockDialogModel::upowerServiceReady, this, &LockWidget::onUPowerServiceReady);
    connect(m_modelLockDialog, &LockDialogModel::windowChanged, this, &LockWidget::onWindowChanged);
    connect(
        m_modelLockDialog, &LockDialogModel::lockDatetimeEnableChanged, this, &LockWidget::onLockDatetimeEnableChanged);
    if (authDialog) {
        connect(authDialog, SIGNAL(authSucceed(QString)), this, SIGNAL(authSucceed(QString)));
        connect(authDialog, &AuthDialog::authSucceed, this, [=] {
            if (m_isShowResetPasswdBtn && m_pResetPwdBtn) {
                m_pResetPwdBtn->hide();
                m_isShowResetPasswdBtn = false;
            }
        });
        connect(authDialog, &AuthDialog::userChangedByManual, this, &LockWidget::onUserChangedByManual);
        connect(authDialog, &AuthDialog::customRequestAccount, this, &LockWidget::onCustomRequestAccount);
        connect(authDialog, &AuthDialog::lineEditClicked, this, [=] { onLineEditClicked(); });
        connect(authDialog, &AuthDialog::lineEditMousePressed, this, [=] { onLineEditMousePressed(); });
        connect(authDialog, &AuthDialog::showResetPasswdBtn, this, &LockWidget::onShowResetPasswdBtn);
        connect(authDialog, &AuthDialog::loginOptsCreated, this, [this]() {
            // 设置Tab焦点导航顺序：m_loginOpts->m_buttonListWidget -> buttonListWidget
            if (authDialog && authDialog->getLoginOpts() && buttonListWidget) {
                QWidget::setTabOrder(authDialog->getLoginOpts()->getButtonListWidget(), buttonListWidget);
            }
        });
    }
    connect(LanguageSetting::instance(this), &LanguageSetting::languageChanged, this, &LockWidget::onLanguageChanged);
}

void LockWidget::initTimeWidget()
{
    m_timeWidget = new TimeWidget(m_modelLockDialog, this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_timeWidget, "TimeWidget", "initialWidget", "add time widget of tools");
    m_timeWidget->move((width() - m_timeWidget->width()) / 2, 0);
    m_timeWidget->setVisible(m_isShowDateTime);
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
    if (m_powerListWidget) {
        if (m_systemMonitorBtn) {
            m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height() + m_systemMonitorBtn->height()));
        } else {
            m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height()));
        }
        m_powerListWidget->updateWidgetSize(rect());
        if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
            m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
        } else { // 居中显示
            m_powerListWidget->move(
                (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
        }
        return;
    }
    m_powerListWidget = new PowerListWidget(m_modelLockDialog, this);
    if (m_systemMonitorBtn) {
        m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height() + m_systemMonitorBtn->height()));
    } else {
        m_powerListWidget->setMaxHeight(height() - (80 + m_timeWidget->height()));
    }
    m_powerListWidget->hide();
    connect(m_powerListWidget, &MyListWidget::currentItemChanged, this, &LockWidget::onPowerItemChanged);
    connect(m_powerListWidget, &PowerListWidget::mulUsersLogined, this, &LockWidget::onMulUsersLogined);
    connect(m_powerListWidget, &PowerListWidget::sureShutDown, this, &LockWidget::onSureShutDown);
    connect(m_powerListWidget, &PowerListWidget::lockScreenClicked, this, &LockWidget::onLockScreenClicked);
    connect(m_powerListWidget, &PowerListWidget::suspendClicked, this, &LockWidget::onSuspendClicked);
    connect(m_powerListWidget, &PowerListWidget::itemClicked, this, &LockWidget::onPowerItemClicked);
    connect(m_powerListWidget, &PowerListWidget::switchuserClicked, this, &LockWidget::onSwitchuserClicked);
    connect(m_powerListWidget, &PowerListWidget::powerWidgetClicked, this, [=] {
        if (m_modelLockDialog->isSessionTools()) {
            Q_EMIT sessionToolsExit();
        } else {
            onShowPowerListWidget();
        }
    });
    connect(m_powerListWidget, &PowerListWidget::doPowerAction, this, &LockWidget::onDoPowerAction);
    m_powerListWidget->updateWidgetSize(rect());
    if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
        m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
    } else { // 居中显示
        m_powerListWidget->move(
            (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
    }
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
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        buttonListWidget->setLayoutDirection(Qt::LeftToRight);
    } else {
        buttonListWidget->setLayoutDirection(Qt::RightToLeft);
    }
    buttonListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    buttonListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString buttonListStyle = "QListWidget{ background:rgba(255, 255, 255, 0%); border: 0px;}"
                              "QListWidget::item{border-radius: 4px; background:rgba(255, 255, 255, 0%);}";
    buttonListWidget->setStyleSheet(buttonListStyle);
    buttonListWidget->setSpacing(8);
    // 电源管理
    m_btnItemPowerManager = new QListWidgetItem();
    m_btnItemPowerManager->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemPowerManager);
    // 虚拟键盘
    m_btnItemVirKb = new QListWidgetItem();
    m_btnItemVirKb->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemVirKb);
#if SUPPORT_NET_ICON
    // 网络设置
    syslog(LOG_INFO, "[ukui-screensaver-dialog] init net icon begin!!");
    m_btnItemNetwork = new QListWidgetItem();
    m_btnItemNetwork->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemNetwork);
    syslog(LOG_INFO, "[ukui-screensaver-dialog] init net icon end!!");
#endif

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
    // 用户选择
    m_btnItemUser = new QListWidgetItem();
    m_btnItemUser->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemUser);
    // 电池信息
    m_btnItemBattery = new QListWidgetItem();
    m_btnItemBattery->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemBattery);
    // 会话选择
    m_btnItemSession = new QListWidgetItem();
    m_btnItemSession->setSizeHint(QSize(48, 48));
    buttonListWidget->addItem(m_btnItemSession);

    buttonListWidget->setFixedSize(QSize((48 + buttonListWidget->spacing() * 2) * buttonListWidget->count(), 64));
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
}

void LockWidget::initBlockWidget()
{
    if (!m_blockWidget) {
        m_blockWidget = new BlockWidget(m_modelLockDialog, this);
        m_blockWidget->installEventFilter(this);
        m_blockWidget->setGeometry(0, 0, width(), height());
        m_blockWidget->hide();
        connect(m_blockWidget, SIGNAL(cancelButtonclicked()), this, SLOT(onCancelBtnClicked()));
        connect(m_blockWidget, SIGNAL(confirmButtonclicked()), this, SLOT(onConfirmBtnClicked()));
    }
}

void LockWidget::initSystemMonitorBtn()
{
    if (!m_systemMonitorBtn) {
        m_systemMonitorBtn = new StatusButton(this, NORMALBTN);
        KDK_EXTEND_ALL_INFO_FORMAT(
            m_systemMonitorBtn, "StatusButton", "initialBtn", "add system-monitor button of tools");
        QFont sysBtnFont;
        sysBtnFont.setFamily(m_curFont);
        sysBtnFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
        m_systemMonitorBtn->setFont(sysBtnFont);
        m_systemMonitorBtn->hide();
        m_systemMonitorBtn->setFixedHeight(48);
        m_systemMonitorBtn->setText(tr("system-monitor"));
        m_systemMonitorBtn->setContentsMargins(5, 0, 5, 0);
        m_systemMonitorBtn->adjustSize();
        m_systemMonitorBtn->setMinimumWidth(m_systemMonitorBtn->sizeHint().width() + 70);
        connect(m_systemMonitorBtn, &StatusButton::clicked, this, &LockWidget::doSystemMonitor);
    }
    if (m_systemMonitorBtn) {
        m_systemMonitorBtn->move(
            (width() - m_systemMonitorBtn->width()) / 2, height() - m_systemMonitorBtn->height() - 40);
    }
}

void LockWidget::initUsdMediaKeys()
{
    m_areaScreenShot = m_modelLockDialog->getUsdAreaScreenShotKey();
    m_areaScreenShot2 = m_modelLockDialog->getUsdAreaScreenShot2Key();
    m_screenShot = m_modelLockDialog->getUsdScreenShotKey();
    m_screenShot2 = m_modelLockDialog->getUsdScreenShot2Key();
    m_windowScreenshot = m_modelLockDialog->getUsdwindowScreenshotKey();
    connect(m_modelLockDialog, &LockDialogModel::usdMediaKeysChanged, this, &LockWidget::onUsdMediaKeysChanged);

    QAction *volumeUpAction = new QAction(this);
    volumeUpAction->setShortcut(QKeySequence(Qt::Key_VolumeUp));
    connect(volumeUpAction, &QAction::triggered, this, [this]() {
       Q_EMIT m_modelLockDialog->usdExternalDoAction(VOLUME_UP_KEY);
    });

    QAction *volumeDownAction = new QAction(this);
    volumeDownAction->setShortcut(QKeySequence(Qt::Key_VolumeDown));
    connect(volumeDownAction, &QAction::triggered, this, [this]() {
       Q_EMIT m_modelLockDialog->usdExternalDoAction(VOLUME_DOWN_KEY);
    });

    QAction *brightnessUpAction = new QAction(this);
    brightnessUpAction->setShortcut(QKeySequence(Qt::Key_MonBrightnessUp));
    connect(brightnessUpAction, &QAction::triggered, this, [this]() {
       Q_EMIT m_modelLockDialog->usdExternalDoAction(BRIGHT_UP_KEY);
    });

    QAction *brightnessDownAction = new QAction(this);
    brightnessDownAction->setShortcut(QKeySequence(Qt::Key_MonBrightnessDown));
    connect(brightnessDownAction, &QAction::triggered, this, [this]() {
       Q_EMIT m_modelLockDialog->usdExternalDoAction(BRIGHT_DOWN_KEY);
    });

    this->addAction(volumeUpAction);
    this->addAction(volumeDownAction);
    this->addAction(brightnessUpAction);
    this->addAction(brightnessDownAction);
}

void LockWidget::initUsdMediaStateKeys()
{
    m_rfkillState = m_modelLockDialog->getUsdMediaRfkillState();
    connect(
        m_modelLockDialog, &LockDialogModel::usdMediaStateKeysChanged, this, &LockWidget::onUsdMediaStateKeysChanged);
}

void LockWidget::initPowerManagerKey()
{
    m_strPowerButtonKey = m_modelLockDialog->getPowerButtonKey();
    connect(m_modelLockDialog, &LockDialogModel::powerButoonKeyChanged, this, [=](const QString &value) {
        m_strPowerButtonKey = value;
    });
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
#if SUPPORT_NET_ICON
    QList<NetWorkButton *> networkButton = this->findChildren<NetWorkButton *>();
    for (auto button : networkButton) {
        button->setFixedSize(48, 48);
        button->setIconSize(QSize(24, 24));
    }
#endif
    if (m_batteryonButton && m_modelLockDialog->getIsBattery()) {
        QPixmap pixmap = QIcon::fromTheme(m_batteryIconName).pixmap(QSize(24, 24));
        QStyleOption opt;
        QPixmap batteryIcon = filledSymbolicColoredPix(pixmap, opt.palette.highlightedText().color());
        m_batteryonButton->setIcon(batteryIcon);
    }
}

void LockWidget::onPowerItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!m_powerListWidget) {
        return;
    }
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
    StatusButton *currentItem = qobject_cast<StatusButton *>(currentWidget);
#if SUPPORT_NET_ICON
    NetWorkButton *netItem = qobject_cast<NetWorkButton *>(currentWidget);
#endif // SUPPORT_NET_ICON

    if (currentItem) {
        currentItem->setClickedStatus(SELECT);
    }
#if SUPPORT_NET_ICON
    if (netItem) {
        netItem->setClickedStatus(SELECT);
    }
#endif // SUPPORT_NET_ICON
    if (previous) {
        QWidget *previousWidget = buttonListWidget->itemWidget(previous);
        StatusButton *previousItem = qobject_cast<StatusButton *>(previousWidget);
#if SUPPORT_NET_ICON
        NetWorkButton *previousNetItem = qobject_cast<NetWorkButton *>(previousWidget);
#endif // SUPPORT_NET_ICON
        if (previousItem) {
            previousItem->setClickedStatus(NORMAL);
        }
#if SUPPORT_NET_ICON
        if (previousNetItem) {
            previousNetItem->setClickedStatus(NORMAL);
        }
#endif // SUPPORT_NET_ICON
    }
}

void LockWidget::onButtonItemClicked(QListWidgetItem *item)
{
    if (buttonListWidget->isHidden()) {
        return;
    }
    QWidget *widget = buttonListWidget->itemWidget(item);
    StatusButton *currentItem = dynamic_cast<StatusButton *>(widget);
    if (currentItem && currentItem->isVisible()) {
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
#if SUPPORT_NET_ICON
    NetWorkButton *netItem = dynamic_cast<NetWorkButton *>(widget);
    if (netItem) {
        netItem->setClickedStatus(CLICKED);
        netItem->clicked();
    }
#endif // SUPPORT_NET_ICON
}

void LockWidget::onPowerItemClicked(QListWidgetItem *item)
{
    if (!m_powerListWidget) {
        return;
    }
    QWidget *widget = m_powerListWidget->itemWidget(item);
    PowerItemWidget *currentItem = dynamic_cast<PowerItemWidget *>(widget);
    if (currentItem) {
        currentItem->setSelected(CLICKED);
        currentItem->clicked();
    }
    if (m_modelLockDialog->isSessionTools()) {
        if (m_systemMonitorBtn && m_systemMonitorBtn->isVisible()) {
            m_systemMonitorBtn->hide();
        }
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
        if (buttonListWidget && m_btnItemSession) {
            if (m_curUserInfo->isLoggedIn() || m_modelLockDialog->sessionsInfo().count() <= 1) {
                buttonListWidget->setItemHidden(m_btnItemSession, true);
                if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
                    m_sessionListWidget->hide();
                }
            } else {
                buttonListWidget->setItemHidden(m_btnItemSession, false);
            }
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
        bool isUserChanged = (m_curUserInfo->name() != ptrUser->name() || m_curUserInfo->uid() != ptrUser->uid());
        m_curUserInfo = ptrUser;
        qDebug() << "CurUserInfo:" << *m_curUserInfo;
        if (m_curUserInfo->name() != "*login" && m_curUserInfo->name() != "*guest") {
            LanguageSetting::instance()->onLanguageChanged(m_curUserInfo->lang());
        }
        if (authDialog && !m_modelLockDialog->isSessionTools()) {
            authDialog->onCurUserChanged(m_curUserInfo);
            if (isUserChanged) {
                if (authDialog->getUserAuthFailedTimes() >= 1) {
                    onShowResetPasswdBtn();
                } else if (m_pResetPwdBtn) {
                    m_pResetPwdBtn->hide();
                    m_isShowResetPasswdBtn = false;
                }
                if (m_pResetPwdWidget && m_pResetPwdWidget->isVisible()) {
                    m_pResetPwdWidget->switchToConfidentialDig();
                    m_pResetPwdWidget->hide();
                    if (m_isShowDateTime) {
                        m_timeWidget->show();
                    }
                    buttonListWidget->show();
                    authDialog->inhibitShow(false);
                }
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
    if (buttonListWidget && m_btnItemSession) {
        if (m_curUserInfo->isLoggedIn() || m_modelLockDialog->sessionsInfo().count() <= 1) {
            buttonListWidget->setItemHidden(m_btnItemSession, true);
            if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
                m_sessionListWidget->hide();
            }
        } else {
            buttonListWidget->setItemHidden(m_btnItemSession, false);
        }
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
            if (widget && widget->objectName() == "BtnSession") {
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
        if (m_sessionButton) {
            m_sessionButton->setClickedStatus(CLICKED);
        }
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
#if SUPPORT_NET_ICON
            if (m_networkButton) {
                m_networkButton->setClickedStatus(NORMAL);
            }
#endif // SUPPORT_NET_ICON
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
    } else {
        m_sessionListWidget->hide();
        if (m_sessionButton) {
            m_sessionButton->setClickedStatus(NORMAL);
        }
    }
}

void LockWidget::onShowBatteryWidget()
{
    if (!batteryWidget) {
        batteryWidget = new BatteryWidget(m_modelLockDialog, this);
        batteryWidget->setFixedSize(420, 112);
        setBatteryWidgetLocate();
        setPowerWindowProperties();
        batteryWidget->setPowerStateLabelForAcOnlineChanged();
    }

    if (batteryWidget && batteryWidget->isHidden()) {
        batteryWidget->show();
        if (m_batteryonButton) {
            m_batteryonButton->setClickedStatus(CLICKED);
        }
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
#if SUPPORT_NET_ICON
            if (m_networkButton) {
                m_networkButton->setClickedStatus(NORMAL);
            }
#endif // SUPPORT_NET_ICON
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
    } else {
        batteryWidget->hide();
        if (m_batteryonButton) {
            m_batteryonButton->setClickedStatus(NORMAL);
        }
    }
}

void LockWidget::onShowUserListWidget(bool show)
{
    initUserWidget();
    if (m_userListWidget && m_userListWidget->isHidden() && m_userButton && m_userButton->isVisible()) {
        m_userListWidget->show();
        if (m_userButton) {
            m_userButton->setClickedStatus(CLICKED);
        }
        if (m_networkWidget && m_networkWidget->isVisible()) {
            m_networkWidget->hide();
#if SUPPORT_NET_ICON
            if (m_networkButton) {
                m_networkButton->setClickedStatus(NORMAL);
            }
#endif // SUPPORT_NET_ICON
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
        m_userListWidget->onUpdateListInfo();
        m_userListWidget->setFocus();
    } else if (show && m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->onUpdateListInfo();
        m_userListWidget->setFocus();
    } else {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
}

void LockWidget::onShowNetworkWidget()
{
    if (!m_networkWidget) {
        m_networkWidget = new MyNetworkWidget(this);
        m_networkWidget->installEventFilter(this);
        connect(m_networkWidget, &MyNetworkWidget::showVirtualKeyboard, this, [=] {
            if (m_isTabletMode) {
                onShowVirtualKeyboard(m_isTabletMode);
            }
        });
    }
    m_networkWidget->loadNetPlugin();
#if SUPPORT_NET_ICON
    auto updateDefaultNetworkPage = [this]() {
        if (!m_networkWidget) {
            return;
        }
        int defaultPageIndex = 0;
        if (m_networkButton) {
            const QVariantMap pageInfo = m_networkButton->executeFunction(static_cast<KyExecuteFuncType>(0));
            bool ok = false;
            const int pageValue = pageInfo.value("getDefaultNetworkPage").toInt(&ok);
            if (ok) {
                defaultPageIndex = pageValue;
            }
        }
        if (defaultPageIndex != 0 && defaultPageIndex != 1) {
            defaultPageIndex = 0;
        }
        m_networkWidget->setDefaultPage(defaultPageIndex);
    };
    updateDefaultNetworkPage();
#endif
    setNetWorkWidgetLocate();
    if (m_networkWidget && m_networkWidget->isVisible()) {
        m_networkWidget->hide();
#if SUPPORT_NET_ICON
        if (m_networkButton) {
            m_networkButton->setClickedStatus(NORMAL);
        }
#endif // SUPPORT_NET_ICON
    } else {
        m_networkWidget->show();
#if SUPPORT_NET_ICON
        if (m_networkButton) {
            m_networkButton->setClickedStatus(CLICKED);
        }
#endif // SUPPORT_NET_ICON
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
        if (batteryWidget && batteryWidget->isVisible()) {
            batteryWidget->hide();
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
    }
}

void LockWidget::onShowResetPasswdBtn()
{
    if (!m_pSecurityQuestionAnswer) {
        m_pSecurityQuestionAnswer = new SecurityQuestionAnswer(this);
    }
    bool hasSecurityQustion = m_pSecurityQuestionAnswer->IsUserHasSecurityAnswers(authDialog->getCurAuthUserUid());
    bool hasBindKylinID = isUserBindKylinID();
    qDebug() << " hasBindKylinID = " << hasBindKylinID << authDialog->getCurAuthUserUid();
    if (hasSecurityQustion && hasBindKylinID) {
        m_nUserHasBindIdentity = ALLDEFINED;
    } else if (hasSecurityQustion) {
        m_nUserHasBindIdentity = SECURITYQUESTION;
    } else if (hasBindKylinID) {
        m_nUserHasBindIdentity = KYLINID;
    } else {
        m_nUserHasBindIdentity = NOTHINGBIND;
    }

    qDebug() << " IsUserHasSecurityAnswers = " << m_nUserHasBindIdentity;
    if (m_nUserHasBindIdentity > 0) {
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

bool LockWidget::isUserBindKylinID()
{
    QDBusInterface interface(
        "org.kylinID.service", "/org/kylinID/path", "org.kylinID.interface", QDBusConnection::systemBus());
    QDBusMessage result = interface.call("IsUserBindKyidAccounts", authDialog->getCurAuthUserUid());
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

void LockWidget::onShowResetPwdWidget()
{
    if (!m_pResetPwdWidget) {
        if (!m_pSecurityQuestionAnswer) {
            m_pSecurityQuestionAnswer = new SecurityQuestionAnswer(this);
        }
        m_pResetPwdWidget = new ResetPwdWidget(
            m_modelLockDialog,
            authDialog->getCurAuthRealName(),
            authDialog->getCurAuthUserUid(),
            m_pSecurityQuestionAnswer,
            this);
        Q_EMIT startFcitx();
        connect(m_pResetPwdWidget, &ResetPwdWidget::backToAuthDialog, this, &LockWidget::onBackToAuthDialog);
        connect(m_pResetPwdWidget, &ResetPwdWidget::resetEditClicked, this, [=]() {
            if (batteryWidget && batteryWidget->isVisible()) {
                batteryWidget->hide();
                if (m_batteryonButton) {
                    m_batteryonButton->setClickedStatus(NORMAL);
                }
            }
            if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
                m_sessionListWidget->hide();
                if (m_sessionButton) {
                    m_sessionButton->setClickedStatus(NORMAL);
                }
            }
            if (m_userListWidget && m_userListWidget->isVisible()) {
                m_userListWidget->hide();
                if (m_userButton) {
                    m_userButton->setClickedStatus(NORMAL);
                }
            }
            if (m_networkWidget && m_networkWidget->isVisible()) {
                m_networkWidget->hide();
#if SUPPORT_NET_ICON
                if (m_networkButton) {
                    m_networkButton->setClickedStatus(NORMAL);
                }
#endif // SUPPORT_NET_ICON
            }
        });
        connect(m_pResetPwdWidget, &ResetPwdWidget::getCodeByKyidAccountsed, this, &LockWidget::onShowKylinIDCodesTips);
        connect(m_pResetPwdWidget, &ResetPwdWidget::hideKylinIDCodesTips, this, [=]() {
            if (authDialog) {
                authDialog->hidePwdTipBtn();
            }
        });
        m_pResetPwdWidget->setFixedWidth(width());
        m_pResetPwdWidget->updateResetPwdSize();
        m_pResetPwdWidget->setGeometry(this->geometry());
        m_pResetPwdWidget->hide();
    }
    m_pResetPwdWidget->setBindIdentity(m_nUserHasBindIdentity);
    m_pResetPwdWidget->updateFontSize(m_curFontSize);
    m_pResetPwdWidget->show();
    m_pResetPwdWidget->setFocusToEdit();
    m_pResetPwdWidget->raise();
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->hide();
    }
    buttonListWidget->hide();
    m_timeWidget->hide();
    authDialog->inhibitShow(true);
}

void LockWidget::onBackToAuthDialog(bool isSuccess)
{
    if (m_isShowDateTime) {
        m_timeWidget->show();
    }
    buttonListWidget->show();
    authDialog->inhibitShow(false);
    authDialog->setFocus();
    if (m_pResetPwdWidget) {
        m_pResetPwdWidget->hide();
    }
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->show();
    }
    if (m_pKylinIDCodeTip) {
        m_pKylinIDCodeTip->hide();
    }
    if (isSuccess)
        authDialog->clearMessage();
}

void LockWidget::onShowKylinIDCodesTips(QString message)
{
    if (!m_pKylinIDCodeTip) {
        m_pKylinIDCodeTip = new TipLabel(this);
    }
    m_pKylinIDCodeTip->setText(message);
    QTimer::singleShot(0, this, [&, this]() { //  不加延时计算的m_pKylinIDCodeTip长度会有问题
        m_pKylinIDCodeTip->move((this->width() - m_pKylinIDCodeTip->width()) / 2, height() - 72);
    });
    m_pKylinIDCodeTip->show();
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
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
        m_virtualKeyboardWidget->show();
    }
}

void LockWidget::onLineEditMousePressed()
{
    exitSubWidget(false, false);
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
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
        m_virtualKeyboardWidget->show();
    } else {
        m_virtualKeyboardWidget->hide();
    }

    if (!m_virtualKeyboardWidget->isHidden()) {
        setNetWorkWidgetLocate();
        m_virtualKeyboardWidget->raise();
    }
    if (m_virKbButton) {
        m_virKbButton->setClickedStatus(NORMAL);
    }
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
            if (m_batteryonButton) {
                m_batteryonButton->setClickedStatus(NORMAL);
            }
        }
        if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
            m_sessionListWidget->hide();
            if (m_sessionButton) {
                m_sessionButton->setClickedStatus(NORMAL);
            }
        }
        if (m_userListWidget && m_userListWidget->isVisible()) {
            m_userListWidget->hide();
            if (m_userButton) {
                m_userButton->setClickedStatus(NORMAL);
            }
        }
        m_virtualKeyboardWidget->show();
    } else {
        m_virtualKeyboardWidget->hide();
    }

    if (!m_virtualKeyboardWidget->isHidden()) {
        setNetWorkWidgetLocate();
        m_virtualKeyboardWidget->raise();
    }
    if (m_virKbButton) {
        m_virKbButton->setClickedStatus(NORMAL);
    }
}

void LockWidget::onShowPowerListWidget()
{
    if (!m_powerListWidget) {
        initPowerWidget();
    }
    if (m_modelLockDialog->isSessionTools()) {
        Q_EMIT moveToCurrentScreen();
        m_powerListWidget->setPowerType(SESSION);
    } else {
        m_powerListWidget->setPowerType(SAVER);
    }
    if (m_powerListWidget->isHidden()) {
        if (m_isShowResetPasswdBtn) {
            m_pResetPwdBtn->hide();
        }
        if (m_pResetPwdWidget && m_pResetPwdWidget->isVisible()) {
            m_pResetPwdWidget->switchToConfidentialDig();
            m_pResetPwdWidget->hide();
        }
        if (m_modelLockDialog->isSessionTools()) {
            m_timeWidget->show();
        } else {
            m_timeWidget->setVisible(m_isShowDateTime);
        }

        buttonListWidget->show();
        authDialog->inhibitShow(true);
        if (m_powerListWidget->getScrollShowStatus()) { // 如果显示Scroll，直接从时间窗口底部开始显示
            m_powerListWidget->move((width() - m_powerListWidget->width()) / 2, m_timeWidget->height() + 30);
        } else { // 居中显示
            m_powerListWidget->move(
                (width() - m_powerListWidget->width()) / 2, (height() - m_powerListWidget->height()) / 2);
        }
        if (m_modelLockDialog->isSessionTools()) {
            initSystemMonitorBtn();
            if (m_systemMonitorBtn) {
                m_systemMonitorBtn->show();
            }
            buttonListWidget->hide();
        }
        m_powerListWidget->show();
        m_powerListWidget->setFocus();
    } else {
        if (m_powerListWidget) {
            m_powerListWidget->hide();
        }
        authDialog->inhibitShow(false);
        // bug#292889 头像加载缓慢问题，需要调用一次update或repaint来刷新
        authDialog->update();
        authDialog->setFocus();
        if (m_isShowResetPasswdBtn) {
            m_pResetPwdBtn->show();
        }
        if (m_systemMonitorBtn) {
            m_systemMonitorBtn->hide();
        }
        buttonListWidget->show();
    }

    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
    if (m_virtualKeyboardWidget && m_virtualKeyboardWidget->isVisible()) {
        m_virtualKeyboardWidget->hide();
        if (m_virKbButton) {
            m_virKbButton->setClickedStatus(NORMAL);
        }
    }
    if (batteryWidget && batteryWidget->isVisible()) {
        batteryWidget->hide();
        if (m_batteryonButton) {
            m_batteryonButton->setClickedStatus(NORMAL);
        }
    }
    if (m_networkWidget && m_networkWidget->isVisible()) {
        m_networkWidget->hide();
#if SUPPORT_NET_ICON
        if (m_networkButton) {
            m_networkButton->setClickedStatus(NORMAL);
        }
#endif // SUPPORT_NET_ICON
    }
    if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
        m_sessionListWidget->hide();
        if (m_sessionButton) {
            m_sessionButton->setClickedStatus(NORMAL);
        }
    }
    if (m_powerManagerButton) {
        m_powerManagerButton->setClickedStatus(NORMAL);
    }
}

void LockWidget::doSystemMonitor()
{
    qDebug() << "doSystemMonitor";
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

void LockWidget::onShowInhibitWarning(
    QVector<InhibitorInfo> list, QVector<WindowInfo::WindowInfo> &windows, int type, bool isSleepLock /* = false*/)
{
    if (!m_isManualPower) {
        m_modelLockDialog->setIsSessionTools(true);
    }
    authDialog->inhibitShow(true);
    buttonListWidget->hide();
    m_timeWidget->hide();
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
    if (m_powerListWidget) {
        m_powerListWidget->hide();
    }
    initBlockWidget();
    if (m_blockWidget) {
        m_blockWidget->setGeometry(0, 0, width(), height());
        m_blockWidget->setWarning(list, windows, type);
        if (isSleepLock) {
            // 睡眠锁的情况先不修改，默认都为隐藏确认按钮
            m_blockWidget->setConfirmButton(true);
        }
        m_blockWidget->show();
        m_blockWidget->setFocus();
    }
    m_isInhibitStatus = true;
}

void LockWidget::onShowInhibitWarning(QVector<InhibitorInfo> list, int type)
{
    if (type == 0 || type == 1 || type == 4) {
        m_isHardInhibit = true;
    } else if (type == 5 || type == 6 || type == 7) {
        m_isHardInhibit = false;
    }

    if (type == 5) {
        m_inhibitType = "RebootSoftBlock";
    } else if (type == 6) {
        m_inhibitType = "PoweroffSoftBlock";
    } else if (type == 7) {
        m_inhibitType = "Logout";
    }
    if (!m_isManualPower) {
        m_modelLockDialog->setIsSessionTools(true);
    }
    authDialog->inhibitShow(true);
    buttonListWidget->hide();
    m_timeWidget->hide();
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
    if (m_powerListWidget) {
        m_powerListWidget->hide();
    }
    initBlockWidget();
    if (m_blockWidget) {
        m_blockWidget->setGeometry(0, 0, width(), height());
        m_blockWidget->setConfirmButton(m_isHardInhibit);
        m_blockWidget->setWarning(list, type);
        m_blockWidget->show();
        m_blockWidget->setFocus();
    }
    m_isInhibitStatus = true;
}

void LockWidget::onSureShutDown(int inhibitType)
{
    //
    if (!m_modelLockDialog->getIsLegacy()) {
        m_alreadyShowMulUsers = true;
    }
    if (!m_isManualPower) {
        m_modelLockDialog->setIsSessionTools(true);
    }
    authDialog->inhibitShow(true);
    buttonListWidget->hide();
    m_timeWidget->hide();
    if (m_powerListWidget) {
        m_powerListWidget->hide();
    }
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
    initBlockWidget();
    if (m_blockWidget) {
        m_blockWidget->setGeometry(0, 0, width(), height());
        if (inhibitType == 0) {
            m_inhibitType = "Reboot";
            m_blockWidget->setTips(
                tr("%1 may lead to unsaved data loss for other logged-in users.")
                    .arg(tr("Restart")), inhibitType);
        } else if (inhibitType == 1) {
            m_inhibitType = "PowerOff";
            m_blockWidget->setTips(
                tr("%1 may lead to unsaved data loss for other logged-in users.")
                    .arg(tr("Shut Down")), inhibitType);
        }
        m_blockWidget->show();
        m_blockWidget->setFocus();
    }
    m_isInhibitStatus = true;
}

void LockWidget::onMulUsersLogined(int inhibitType)
{
    if (!m_modelLockDialog->getIsLegacy()) {
        m_alreadyShowMulUsers = true;
        QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
        if (shutdownLockcheck.isEmpty()) {
            m_runCloseWindow = false;
        } else {
            for (int i = 0; i < shutdownLockcheck.size(); ++i) {
                qDebug() << shutdownLockcheck[i].uid;
                if (shutdownLockcheck[i].uid == 0) {
                    m_runCloseWindow = true;
                    break;
                } else {
                    m_runCloseWindow = false;
                }
            }
        }
        qDebug() << "m_runCloseWindow" << m_runCloseWindow;
    }
    if (!m_isManualPower) {
        m_modelLockDialog->setIsSessionTools(true);
    }
    authDialog->inhibitShow(true);
    buttonListWidget->hide();
    m_timeWidget->hide();
    if (m_powerListWidget) {
        m_powerListWidget->hide();
    }
    if (m_userListWidget && m_userListWidget->isVisible()) {
        m_userListWidget->hide();
        if (m_userButton) {
            m_userButton->setClickedStatus(NORMAL);
        }
    }
    initBlockWidget();
    if (m_blockWidget) {
        m_blockWidget->setGeometry(0, 0, width(), height());
        if (inhibitType == 0) {
            m_inhibitType = "Reboot";
            m_blockWidget->setTips(
                tr("Restart may lead to unsaved data loss for other logged-in users.").arg(tr("Restart")), inhibitType);
        } else if (inhibitType == 1) {
            m_inhibitType = "PowerOff";
            m_blockWidget->setTips(
                tr("Shut Down may lead to unsaved data loss for other logged-in users.").arg(tr("Shut Down")),
                inhibitType);
        }
        m_blockWidget->show();
        m_blockWidget->setFocus();
    }
    m_isInhibitStatus = true;
}

void LockWidget::onConfirmBtnClicked()
{
    m_isInhibitStatus = false;
    m_blockSetClosePhase = true;
    if (m_blockWidget) {
        m_blockWidget->hide();
    }
    if (m_powerListWidget) {
        m_powerListWidget->hide();
    }
    authDialog->inhibitShow(false);
    if (m_isShowDateTime) {
        m_timeWidget->show();
    }
    buttonListWidget->show();
    bool isLegacy = m_modelLockDialog->getIsLegacy();
    bool isSessionTools = m_modelLockDialog->isSessionTools();
    qInfo() << "onConfirmBtnClicked" << isSessionTools << m_inhibitType << m_runCloseWindow << m_alreadyShowMulUsers
            << isLegacy;
    if (!isLegacy && isSessionTools) {
        m_inPowerPhase = false;
        Q_EMIT m_modelLockDialog->setPowerManager(false, m_inhibitType, m_runCloseWindow, m_alreadyShowMulUsers, true);
        exitSubWidget();
        Q_EMIT sessionToolsExit();
    } else if (!isLegacy && !isSessionTools) {
        m_inPowerPhase = false;
        Q_EMIT m_modelLockDialog->setPowerManager(true, m_inhibitType, false, m_alreadyShowMulUsers);
    } else if (isLegacy && isSessionTools) {
        Q_EMIT m_modelLockDialog->setPowerManager(false, m_inhibitType, false, false, true);
        exitSubWidget();
        Q_EMIT sessionToolsExit();
    } else if (isLegacy && !isSessionTools) {
        Q_EMIT m_modelLockDialog->setPowerManager(true, m_inhibitType);
    }
    m_blockSetClosePhase = false;
}

void LockWidget::onCancelBtnClicked()
{
    m_isInhibitStatus = false;
    if (m_blockWidget) {
        m_blockWidget->hide();
    }
    if (m_powerListWidget) {
        m_powerListWidget->show();
    }
    buttonListWidget->show();
    m_timeWidget->show();
    if (m_systemMonitorBtn) {
        m_systemMonitorBtn->hide();
    }
    if (m_modelLockDialog->isSessionTools()) {
        Q_EMIT sessionToolsExit();
    }
    if (m_inhibitType.contains(QStringLiteral("Reboot")) || m_inhibitType.contains(QStringLiteral("PowerOff"))
        || m_inhibitType.contains(QStringLiteral("Logout"))) {
        m_alreadyShowMulUsers = false;
        m_inPowerPhase = false;
        Q_EMIT m_modelLockDialog->setClosePhase(false);
    }
    exitSubWidget();
}

void LockWidget::onLockScreenClicked()
{
    doDelayInit();
    Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    m_modelLockDialog->setIsSessionTools(false);
    Q_EMIT moveToPrimaryScreen();
    Q_EMIT setCurrentBackground();
    onCurUserChanged(m_curUserInfo->name());
    exitSubWidget();
}

void LockWidget::onSuspendClicked(const QString &strType)
{
    QVector<InhibitorInfo> lockcheck = m_modelLockDialog->getSleepLockcheck();
    bool canContinue = true;
    if (!isGreeterMode() && m_modelLockDialog->isSessionTools() && m_modelLockDialog->getSleepActivationEnabled()) {
        Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
        if (lockcheck.isEmpty()) {
            Q_EMIT showBlankScreensaver(0, true);
        } else {
            if (strType == "Hibernate") {
                Q_EMIT m_modelLockDialog->showAppBlockWindow(3);
            } else {
                Q_EMIT m_modelLockDialog->showAppBlockWindow(2);
            }
            canContinue = false;
        }
    } else {
        if (lockcheck.isEmpty()) {
            if (m_modelLockDialog->isSessionTools()) {
                exitSubWidget();
            } else {
                Q_EMIT showBlankScreensaver(0, true);
            }
        } else {
            if (strType == "Hibernate") {
                Q_EMIT m_modelLockDialog->showAppBlockWindow(3);
            } else {
                Q_EMIT m_modelLockDialog->showAppBlockWindow(2);
            }
            canContinue = false;
        }
    }
    if (canContinue) {
        QTimer::singleShot(100, this, [&, strType]() { Q_EMIT m_modelLockDialog->setPowerManager(false, strType); });
    }
}

void LockWidget::onDoPowerAction(const QString &strType, bool isCloseWindowPhase /* = false*/)
{
    QString strRealType = strType;
    if (!strType.isEmpty()) {
        m_command = strType;
    }
    qInfo() << "doPowerAction:" << strRealType << isCloseWindowPhase << m_modelLockDialog->isSessionTools()
            << m_modelLockDialog->getIsLegacy();
    if (!strRealType.isEmpty()) {
        if (!m_modelLockDialog->getIsLegacy()) {
            m_runCloseWindow = isCloseWindowPhase;
            m_inhibitType = strType;
            if (strRealType != "UpgradeThenRestart" && strRealType != "UpgradeThenShutdown") {
                if (strRealType == "Hibernate" || strRealType == "Suspend") {
                    QVector<InhibitorInfo> sleepLockcheck = m_modelLockDialog->getSleepLockcheck();
                    if (!sleepLockcheck.isEmpty()) {
                        if (strRealType == "Hibernate") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(3);
                        } else {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(2);
                        }
                        return;
                    }
                }

                if (m_modelLockDialog->getLoggedInUsersCount() > 1 && strRealType != "Logout"
                    && !m_alreadyShowMulUsers) {
                    if (strRealType == "Reboot") {
                        Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(0);
                    } else if (strRealType == "PowerOff") {
                        Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(1);
                    }
                    return;
                }

                if (strRealType == "Reboot" || strRealType == "PowerOff" || strRealType == "Logout") {
                    // closewindow阶段，只通知session给所有客户端发送退出信号
                    if (isCloseWindowPhase) {
                        m_inPowerPhase = true;
                        Q_EMIT m_modelLockDialog->setPowerManager(false, strRealType, true, false, true);
                        if (m_modelLockDialog->isSessionTools()) {
                            qDebug() << "exit tools";
                            Q_EMIT sessionToolsExit();
                        }
                        return;
                    }
                    QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
                    QVector<InhibitorInfo> logoutLockcheck = m_modelLockDialog->getLogoutLockCheck();
                    QVector<WindowInfo::WindowInfo> windows = m_modelLockDialog->getWindowList();
                    int count = shutdownLockcheck.count() + logoutLockcheck.count() + windows.count();
                    qDebug() << "inhibit" << count << shutdownLockcheck.count() << logoutLockcheck.count()
                             << windows.count();
                    if (count > 0) {
                        if (strRealType == "Reboot") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(0);
                        } else if (strRealType == "PowerOff") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(1);
                        } else {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(4);
                        }
                        return;
                    }
                }
                qDebug() << "isCloseWindowPhase" << isCloseWindowPhase;
                qDebug() << "doPowerAction:" << strRealType << m_modelLockDialog->isSessionTools();
                Q_EMIT m_modelLockDialog->setPowerManager(
                    false, strRealType, isCloseWindowPhase, m_alreadyShowMulUsers);
            } else {
                Q_EMIT m_modelLockDialog->setPowerManager(false, strRealType, false, m_alreadyShowMulUsers);
            }
        } else {
            if (strRealType != "UpgradeThenRestart" && strRealType != "UpgradeThenShutdown") {
                if (strRealType == "Hibernate" || strRealType == "Suspend") {
                    QVector<InhibitorInfo> sleepLockcheck = m_modelLockDialog->getSleepLockcheck();
                    if (!sleepLockcheck.isEmpty()) {
                        if (strRealType == "Hibernate") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(3);
                        } else {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(2);
                        }
                        return;
                    }
                }
                if (strRealType == "Reboot" || strRealType == "PowerOff" || strRealType == "Logout") {
                    QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
                    if (shutdownLockcheck.count() > 0) {
                        if (strRealType == "Reboot") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(0);
                        } else if (strRealType == "PowerOff") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(1);
                        } else {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(4);
                        }
                        return;
                    }
                    QVector<InhibitorInfo> logoutLockcheck = m_modelLockDialog->getLogoutLockCheck();
                    if (logoutLockcheck.count() > 0) {
                        qDebug() << "logoutlockcheck";
                        if (strRealType == "Reboot") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(5);
                        } else if (strRealType == "PowerOff") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(6);
                        } else if (strRealType == "Logout") {
                            Q_EMIT m_modelLockDialog->showAppBlockWindow(7);
                        }
                        return;
                    }
                    if (m_modelLockDialog->getLoggedInUsersCount() > 1 && strRealType != "Logout") {
                        qDebug() << "multiuserblockwindow";
                        if (strRealType == "Reboot") {
                            Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(0);
                        } else if (strRealType == "PowerOff") {
                            Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(1);
                        }
                        return;
                    }
                }

                // 当存在多用户且存在软阻塞的情况下，且用户选择的是关机或重启，并且用户在软阻塞场景下点击了确认按钮
                if (strRealType == "RebootSoftBlock" || strRealType == "PoweroffSoftBlock") {
                    if (strRealType == "RebootSoftBlock") {
                        strRealType = "Reboot";
                    } else if (strRealType == "PoweroffSoftBlock") {
                        strRealType = "PowerOff";
                    }
                    if (m_modelLockDialog->getLoggedInUsersCount() > 1) {
                        qDebug() << "multiuserblockwindow";
                        if (strRealType == "Reboot") {
                            Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(0);
                        } else if (strRealType == "PowerOff") {
                            Q_EMIT m_modelLockDialog->showMultiUsersBlockWindow(1);
                        }
                        return;
                    }
                }
            }
            qDebug() << "doPowerAction:" << strRealType << m_modelLockDialog->isSessionTools();
            Q_EMIT m_modelLockDialog->setPowerManager(false, strRealType);
            qDebug() << m_modelLockDialog->isSessionTools() << "exit tools before";
            if (m_modelLockDialog->isSessionTools()) {
                qDebug() << "exit tools";
                Q_EMIT sessionToolsExit();
            }
        }
    }
}

void LockWidget::onSwitchuserClicked()
{
    onLockScreenClicked();
    qDebug() << "onSwitchuserClicked";
    // fix flash headimage
    QTimer::singleShot(0, this, [=]() { onShowUserListWidget(); });
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

void LockWidget::onLockDatetimeEnableChanged(bool enable, QString username)
{
    if (m_curUserInfo->name() == username) {
        m_isShowDateTime = enable;
        m_timeWidget->setVisible(m_isShowDateTime);
    }
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
        || (willSwitchUser.strUserName != "*login"
            && !UniAuthService::instance()->isUserNameValid(willSwitchUser.strUserName))) {
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
    if (systemLang == local.name()) {
        return;
    }
    systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        if (m_pBottomWidget) {
            m_pBottomWidget->setLayoutDirection(Qt::RightToLeft);
        }
        if (m_userListWidget) {
            m_userListWidget->setLayoutDirection(Qt::RightToLeft);
        }
        if (m_powerListWidget) {
            m_powerListWidget->setLayoutDirection(Qt::RightToLeft);
        }
    } else {
        if (m_pBottomWidget) {
            m_pBottomWidget->setLayoutDirection(Qt::LeftToRight);
        }
        if (m_userListWidget) {
            m_userListWidget->setLayoutDirection(Qt::LeftToRight);
        }
        if (m_powerListWidget) {
            m_powerListWidget->setLayoutDirection(Qt::LeftToRight);
        }
    }
    setUserListWidgetLocate();
    if (m_powerManagerButton) {
        m_powerManagerButton->setToolTip(tr("Power"));
    }
    if (m_virKbButton) {
        m_virKbButton->setToolTip(tr("VirtualKeyboard"));
    }
#if SUPPORT_NET_ICON
    if (m_networkButton) {
        m_networkButton->setToolTip(tr("Network"));
    }
#endif // SUPPORT_NET_ICON
    if (m_userButton) {
        m_userButton->setToolTip(tr("SwitchUser"));
    }
    if (m_batteryonButton) {
        m_batteryonButton->setToolTip(tr("Power Information"));
    }
    if (m_sessionButton) {
        m_sessionButton->setToolTip(tr("SwitchSession"));
    }
    if (m_systemMonitorBtn) {
        m_systemMonitorBtn->setText(tr("system-monitor"));
    }
    if (m_pResetPwdBtn) {
        m_pResetPwdBtn->setText(tr("Reset Pwd"));
    }
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
#ifdef SUPPORT_X11_BACKGROUND
    if (drawBackgroundType == 0)
        setRootWindowBackground(0, 0, path, screenRectList, m_nPicOptions);
    else
        setRootWindowBackground(1, drawBackgroundColor, NULL, screenRectList, m_nPicOptions);
#endif
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
    for (QScreen *screen : effectiveApplicationScreens()) {
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
#ifdef SUPPORT_X11_BACKGROUND
    drawBackground(true);
#endif
}

void LockWidget::updateFont()
{
    if (authDialog) {
        authDialog->updateAuthFont(m_curFont);
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
}

void LockWidget::updateFontSize()
{
    if (authDialog) {
        authDialog->updateAuthFontSize(m_curFontSize);
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
}

void LockWidget::onUPowerServiceReady()
{
    qDebug() << "onUPowerServiceReady delay!";
    if (buttonListWidget && m_btnItemBattery && m_batteryonButton) {
        if (!m_modelLockDialog->getIsBattery()) {
            buttonListWidget->setItemHidden(m_btnItemBattery, true);
        } else {
            buttonListWidget->setItemHidden(m_btnItemBattery, false);
            m_batteryIconName = m_modelLockDialog->getBatteryIconName();
            QPixmap pixmap = QIcon::fromTheme(m_batteryIconName).pixmap(QSize(24, 24));
            QStyleOption opt;
            QPixmap batteryIcon = filledSymbolicColoredPix(pixmap, opt.palette.highlightedText().color());
            m_batteryonButton->setIcon(batteryIcon);
            updateBatteryInfo(true);
        }
    }
}

void LockWidget::doDelayInit()
{
    // 电源管理
    if (!m_powerManagerButton) {
        m_powerManagerButton = new StatusButton(this, BOTBUTTON);
        if (buttonListWidget && m_btnItemPowerManager) {
            buttonListWidget->setItemWidget(m_btnItemPowerManager, m_powerManagerButton);
        }
        m_powerManagerButton->setToolTip(tr("Power"));
        connect(m_powerManagerButton, &StatusButton::clicked, this, [this]() {
            m_isManualPower = true;
            onShowPowerListWidget();
        });
        m_powerManagerButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("system-shutdown-symbolic", QIcon(":/image/assets/shutdown.svg")).pixmap(24, 24),
            "white"));
    }
    // 虚拟键盘
    if (!m_virKbButton) {
        m_virKbButton = new StatusButton(this, BOTBUTTON);
        if (buttonListWidget && m_btnItemVirKb) {
            buttonListWidget->setItemWidget(m_btnItemVirKb, m_virKbButton);
        }
        m_virKbButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("input-keyboard-symbolic", QIcon(":/image/assets/keyboard.svg")).pixmap(24, 24), "white"));
        m_virKbButton->setToolTip(tr("VirtualKeyboard"));
        connect(m_virKbButton, &StatusButton::clicked, this, [this]() { onShowVirtualKeyboard(); });
        if (m_modelLockDialog->getTabletMode()) {
            onShowVirtualKeyboard(m_modelLockDialog->getTabletMode());
        }
    }
#if SUPPORT_NET_ICON
    if (!m_networkButton) {
        m_networkButton = new NetWorkButton(this);
        if (buttonListWidget && m_btnItemNetwork) {
            buttonListWidget->setItemWidget(m_btnItemNetwork, m_networkButton);
        }
        m_networkButton->setToolTip(tr("Network"));
        connect(m_networkButton, &NetWorkButton::clicked, this, [this]() { onShowNetworkWidget(); });
    }
#endif
    // 用户选择
    if (!m_userButton) {
        m_userButton = new StatusButton(this, BOTBUTTON);
        if (buttonListWidget && m_btnItemUser) {
            buttonListWidget->setItemWidget(m_btnItemUser, m_userButton);
        }
        m_userButton->setToolTip(tr("SwitchUser"));
        connect(m_userButton, &StatusButton::clicked, this, [this]() { onShowUserListWidget(); });
        if (!isGreeterMode() && !m_modelLockDialog->getLockCanSwitchUser()) {
            buttonListWidget->setItemHidden(m_btnItemUser, true);
        } else if (m_modelLockDialog->normalUserCount() > 1 || m_modelLockDialog->findUserByName("*login")) {
            buttonListWidget->setItemHidden(m_btnItemUser, false);
        } else {
            buttonListWidget->setItemHidden(m_btnItemUser, true);
        }
        m_userButton->setIcon(drawSymbolicColoredPixmap(
            QIcon::fromTheme("system-users-symbolic", QIcon(":/image/assets/switchuser.svg")).pixmap(24, 24), "white"));
    }
    // 电池信息
    if (!m_batteryonButton) {
        m_batteryonButton = new StatusButton(this, BOTBUTTON);
        if (buttonListWidget && m_btnItemBattery) {
            buttonListWidget->setItemWidget(m_btnItemBattery, m_batteryonButton);
        }
        m_batteryIconName = m_modelLockDialog->getBatteryIconName();
        QPixmap pixmap = QIcon::fromTheme(m_batteryIconName).pixmap(QSize(24, 24));
        QStyleOption opt;
        QPixmap batteryIcon = filledSymbolicColoredPix(pixmap, opt.palette.highlightedText().color());
        m_batteryonButton->setIcon(batteryIcon);
        connect(m_modelLockDialog, &LockDialogModel::batteryStatusChanged, this, [=](const QString &iconName) {
            if (m_batteryIconName != iconName) {
                m_batteryIconName = iconName;
                QPixmap pixmap = QIcon::fromTheme(m_batteryIconName).pixmap(QSize(24, 24));
                QPixmap batteryIcon = filledSymbolicColoredPix(pixmap, opt.palette.highlightedText().color());
                m_batteryonButton->setIcon(batteryIcon);
                if (batteryWidget) {
                    batteryWidget->setIconName(m_batteryIconName);
                }
            }
        });
        connect(m_modelLockDialog, &LockDialogModel::batteryChanged, this, [=](const QStringList &batteryArgs) {
            qDebug() << "battery info changed";
            updateBatteryInfo();
        });
        connect(m_batteryonButton, &StatusButton::clicked, this, [this]() { onShowBatteryWidget(); });
        if (!m_modelLockDialog->getIsBattery()) {
            buttonListWidget->setItemHidden(m_btnItemBattery, true);
        } else {
            buttonListWidget->setItemHidden(m_btnItemBattery, false);
            // 初始化电量信息
            updateBatteryInfo(true);
        }
    }
    // 会话选择
    if (!m_sessionButton) {
        m_sessionButton = new StatusButton(this, BOTBUTTON);
        if (buttonListWidget && m_btnItemSession) {
            buttonListWidget->setItemWidget(m_btnItemSession, m_sessionButton);
        }
        m_sessionButton->setObjectName("BtnSession");
        if (m_curUserInfo->isLoggedIn() || m_modelLockDialog->sessionsInfo().count() <= 1) {
            buttonListWidget->setItemHidden(m_btnItemSession, true);
            if (m_sessionListWidget && m_sessionListWidget->isVisible()) {
                m_sessionListWidget->hide();
            }
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
}

void LockWidget::updateBatteryInfo(bool isFirst /* = false*/)
{
    bool isChanged = false;
    QStringList batteryArgs = m_modelLockDialog->getBatteryArgs();
    if (batteryArgs.count() > 0) {
        double percentage = batteryArgs[0].toDouble();
        if (percentage != m_batteryPercentage) {
            m_batteryPercentage = percentage;
            isChanged = true;
        }
    }
    if (batteryArgs.count() > 1) {
        int nBatteryState = batteryArgs[1].toInt();
        if (nBatteryState != m_batteryState) {
            m_batteryState = nBatteryState;
            isChanged = true;
        }
    }
    if (batteryArgs.count() > 2) {
        qint64 timeToEmpty = batteryArgs[2].toLongLong();
        if (timeToEmpty != m_timeToEmpty) {
            m_timeToEmpty = timeToEmpty;
            // isChanged = true;
        }
    }
    if (batteryArgs.count() > 3) {
        bool hasBattery = QVariant(batteryArgs[3]).toBool();
        if (hasBattery != m_isHasBattery) {
            m_isHasBattery = hasBattery;
            isChanged = true;
        }
    }
    if (batteryArgs.count() > 4) {
        bool acOnlineState = QVariant(batteryArgs[4]).toBool();
        if (acOnlineState != m_acOnlineState) {
            m_acOnlineState = acOnlineState;
            isChanged = true;
        }
    }
    if (batteryArgs.count() > 5) {
        qint64 timeToFull = batteryArgs[5].toLongLong();
        if (timeToFull != m_timeToFull) {
            m_timeToFull = timeToFull;
            // isChanged = true;
        }
    }
    if (isChanged || isFirst) {
        setTrayIconToolTipForAcOnlineChanged(m_acOnlineState, m_batteryPercentage);
        // 更新widget内容
        setPowerWindowProperties();
        if (batteryWidget) {
            batteryWidget->setPowerStateLabelForAcOnlineChanged();
        }
    }
    qDebug() << "Battery info:" << m_batteryPercentage << m_batteryState << m_isHasBattery << m_acOnlineState;
}

void LockWidget::setTrayIconToolTipForAcOnlineChanged(bool acOnlineState, double percentage)
{
    if (!m_batteryonButton) {
        return;
    }
    // 添加无电池情况的处理
    if (!m_isHasBattery) {
        if (true == acOnlineState) {
            m_batteryonButton->setToolTip(tr("No battery detected"));
        }
        return;
    }

    if (true == acOnlineState) {
        if (100 == percentage) {
            m_batteryonButton->setToolTip(tr("Fully Charged (100%)"));
        } else {
            m_batteryonButton->setToolTip(tr("%1% available (plugged in)").arg(percentage));
        }
    } else {
        m_batteryonButton->setToolTip(tr("%1% remaining").arg(percentage));
    }
    qDebug() << "battery tooltips:" << m_batteryonButton->toolTip();
}

void LockWidget::setPowerWindowProperties()
{
    if (batteryWidget) {
        batteryWidget->setIconName(m_batteryIconName);
        batteryWidget->setHasBattery(m_isHasBattery);
        batteryWidget->setAcOnlineState(m_acOnlineState);
        batteryWidget->setBatteryState(m_batteryState);
        batteryWidget->setBatteryPercentage(m_batteryPercentage);
        batteryWidget->setBatteryLeftTime(m_timeToFull, m_timeToEmpty);
    }
}

void LockWidget::onWindowChanged(const QString &operation, const QString &winId)
{
    if (m_inPowerPhase) {
        qDebug() << "onWindowChanged" << operation;
        if (operation == QStringLiteral("remove")) {
            QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
            QVector<InhibitorInfo> logoutLockcheck = m_modelLockDialog->getLogoutLockCheck();
            QVector<WindowInfo::WindowInfo> windows = m_modelLockDialog->getWindowList();
            int count = shutdownLockcheck.count() + logoutLockcheck.count() + windows.count();
            qDebug() << "inhibit" << count << shutdownLockcheck.count() << logoutLockcheck.count() << windows.count();
            if (count == 0) {
                onDoPowerAction(m_command, false);
            }
        }
    }
}
