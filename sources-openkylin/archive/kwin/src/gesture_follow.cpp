/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QPoint>
#include <QRect>
#include <QtConcurrentRun>

#include <libkydatacollect.h>

#include "abstract_client.h"
#include "gesture_follow.h"
#include "input_event.h"
#include "screens.h"
#include "tablet_manager.h"
#include "toplevel.h"
#include "unmanaged.h"
#include "virtualdesktops.h"
#include "workspace.h"
#include "x11client.h"

namespace KWin
{

KWIN_SINGLETON_FACTORY(GestureFollowInputFilter)

GestureFollowInputFilter::GestureFollowInputFilter(QObject *parent)
    : QObject(parent)
{
    getTabletTaskbarWininfo();
    monitorModeChanged();

    connect(TabletManager::self(), &TabletManager::tabletModeChanged, this, &GestureFollowInputFilter::monitorModeChanged);
    connect(workspace(), &Workspace::clientActivated, this, &GestureFollowInputFilter::handleActiveClient);
    connect(workspace(), &Workspace::clientAdded, this, &GestureFollowInputFilter::handleActiveClient);
    connect(workspace(), &Workspace::clientRemoved, this, [&](auto client) {
        if (client->caption() == "tablet-taskbar") {
            m_gestureFollowWindow = nullptr;
            m_panelOriginYCoordinate = 0;
        }
    });

    connect(screens(), &Screens::geometryChanged, this, [&]() {
        float scaleFactor = getScreenWidth() >= getScreenHeight() ? getScreenHeight() / 1080.0 : getScreenWidth() / 1080.0;
        m_panelOriginYCoordinate = getScreenHeight() - (36 + 96) * scaleFactor;
    });
}

void GestureFollowInputFilter::getTabletTaskbarWininfo()
{
    auto list = Workspace::self()->stackingOrder();
    for (auto window : list) {
        if (!window->isDeleted()) {
            auto win = qobject_cast<AbstractClient *>(window);
            if (win->caption() == "tablet-taskbar") {
                m_gestureFollowWindow = win;
                m_panelOriginXCoordinate = m_gestureFollowWindow->x();
                m_panelOriginYCoordinate = m_gestureFollowWindow->y();
                m_panelHeight = m_gestureFollowWindow->height();
                qDebug() << __func__ << "当前窗口信息: "
                         << "X: " << m_panelOriginXCoordinate
                         << "Y: " << m_panelOriginYCoordinate
                         << "Height: " << m_panelHeight;
            }
        }
    }

    float scaleFactor = getScreenWidth() >= getScreenHeight() ? getScreenHeight() / 1080.0 : getScreenWidth() / 1080.0;
    if (m_panelOriginYCoordinate == getScreenHeight() - (36 + 96) * scaleFactor) {
        return;
    }

    m_panelOriginYCoordinate = getScreenHeight() - (36 + 96) * scaleFactor;
}

void GestureFollowInputFilter::monitorModeChanged()
{
    if (m_gestureFollowWindow == nullptr) {
        qDebug() << "m_gestureFollowWindow is nullptr" << __FILE__ << __func__ << __LINE__;
        return;
    }

    m_gestureFollowWindow->hideClient();
}

void GestureFollowInputFilter::handleActiveClient(AbstractClient *client)
{
    if (!client) {
        return;
    }

    if (client->caption() == "tablet-taskbar") {
        m_gestureFollowWindow = client;
        m_panelOriginXCoordinate = m_gestureFollowWindow->x();
        if (m_panelOriginYCoordinate == 0) {
            m_panelOriginYCoordinate = m_gestureFollowWindow->y();
        }

        m_panelHeight = m_gestureFollowWindow->height();
        qDebug() << __func__ << __LINE__
                 << "当前窗口信息： "
                 << "x : " << m_panelOriginXCoordinate
                 << "Y; " << m_panelOriginYCoordinate << "height : " << m_panelHeight;

        if (!m_clickPositionInPanel && !m_startGestureFollow) {
            monitorModeChanged();
        }
    } else {
        if (!TabletManager::self()->tabletMode() || !m_gestureFollowWindow) {
            return;
        }

        if (!m_gestureFollowWindow->isHiddenInternal()) {
            m_gestureFollowWindow->hideClient();
        }
    }
}

bool GestureFollowInputFilter::haveFullscreenUnmanagedWindowShowing()
{
    bool haveUnmanaged = false;
    for (auto *unmanaged : Workspace::self()->unmanagedList()) {
        if (!unmanaged) {
            continue;
        }
        if (unmanaged->resourceName().contains("kwin")) {
            continue;
        }
        if ((unmanaged->size().width() >= getScreenWidth() - 1) && (unmanaged->size().height() >= getScreenHeight() - 1)) {
            haveUnmanaged = true;
            break;
        }
    }
    return haveUnmanaged;
}

GestureFollowInputFilter::FrontWindowType GestureFollowInputFilter::frontWindowType()
{
    FrontWindowType type = FrontWindowType::NormalWindow;

    if (!Workspace::self()->activeClient()) {
        type = haveFullscreenUnmanagedWindowShowing() ? FrontWindowType::UtilityWindow
                                                      : FrontWindowType::DesktopWindow;
    } else {
        if (Workspace::self()->activeClient()->isDesktop()) {
            type = FrontWindowType::DesktopWindow;
        }

        if (blackList.contains(Workspace::self()->activeClient()->resourceClass())) {
            type = FrontWindowType::UtilityWindow;
        }

        if (Workspace::self()->activeClient()->isLogout()
            || Workspace::self()->activeClient()->isScreenLock()
            || Workspace::self()->activeClient()->isSystemWindow()) {
            type = FrontWindowType::UtilityWindow;
        }
    }

    return type;
}

int GestureFollowInputFilter::getScreenHeight()
{
    QRect mRect = screens()->geometry();
    return mRect.height();
}

int GestureFollowInputFilter::getScreenWidth()
{
    QRect mRect = screens()->geometry();
    return mRect.width();
}

void GestureFollowInputFilter::desktopOpacityAnimation(double deltaY)
{
    if (!TabletManager::self()) {
        return;
    }

    // 4000是最大模糊程度，0.15代表从屏幕底部往上所占屏幕比例为15%
    double radius = -(4000 / (0.15 * getScreenHeight())) * deltaY + 4000 / 0.15;
    TabletManager::self()->setBlurBehind(radius > 4000 ? 4000 : radius);
}

void GestureFollowInputFilter::resetReferenceValue()
{
    m_YcriticalValue = getScreenHeight() - 0.15 * getScreenHeight();
    m_panelDisplayYCoordinate = getScreenHeight() - m_panelHeight / 2;
    if (m_gestureFollowWindow != nullptr) {
        m_panelOriginXCoordinate = getScreenWidth() / 2 - m_gestureFollowWindow->width() / 2;
    }
}

bool GestureFollowInputFilter::isAwakenViewGesture()
{
    // 计算手势滑动时长
    int res = 0;
    QTime curTime = QTime::currentTime();
    res = m_beginTime.msecsTo(curTime);
    if (res > 300) {
        return true;
    }
    return false;
}

void GestureFollowInputFilter::sendTriggerMultitaskView()
{
    KCustomProperty property[1];

    property[0].key = (char *)"gestureBehavior";
    property[0].value = (char *)"triggerMultitaskView";

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, 1);
    kdk_dia_upload_default(node, (char *)"screen_bottom_edge_gesture_event", (char *)"gesture-follow");
    kdk_dia_data_free(node);
}

void GestureFollowInputFilter::sendMultitaskViewDbusResult(quint32 type)
{
    KCustomProperty property[1];

    property[0].key = (char *)"multitaskViewReplyType";

    QDBusMessage::MessageType dbusType = static_cast<QDBusMessage::MessageType>(type);

    switch (dbusType) {
    case QDBusMessage::ReplyMessage:
        property[0].value = (char *)"ReplyMessage";
        break;
    case QDBusMessage::InvalidMessage:
        property[0].value = (char *)"InvalidMessage";
        break;
    case QDBusMessage::MethodCallMessage:
        property[0].value = (char *)"MethodCallMessage";
        break;
    case QDBusMessage::ErrorMessage:
        property[0].value = (char *)"ErrorMessage";
        break;
    case QDBusMessage::SignalMessage:
        property[0].value = (char *)"SignalMessage";
        break;
    default:
        property[0].value = (char *)"Default";
        break;
    }

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, 1);
    kdk_dia_upload_default(node, (char *)"multitask_view_reply_event", (char *)"gesture-follow");
    kdk_dia_data_free(node);
}

void GestureFollowInputFilter::touchDown(qint32 id, const QPointF &pos, quint32 time)
{

#ifndef QT_NO_DEBUG
    qDebug() << "当前按下的手指id为： " << id << "当前存储的手指数量为： " << gestureInfo.size();
#endif

    if (gestureInfo.size() >= 1) {
        return;
    }

    m_gestureId = id;
    gestureInfo.insert(std::make_pair(id, pos));
    m_beginTime = QTime::currentTime();
    resetReferenceValue();

    if (m_gestureFollowWindow == nullptr) {
        m_startGestureFollow = false;
        return;
    }
    if (TabletManager::self()->tabletMode() && !m_gestureFollowWindow->isHiddenInternal()) {
        handlePressEvent(pos.x(), pos.y());
    }

    if (TabletManager::self()->tabletMode() && gestureInfo[m_gestureId].y() >= getScreenHeight() - 10) {
        if (frontWindowType() == FrontWindowType::DesktopWindow) {
            m_startGestureFollow = false;
            // 显示时设置为0，使毛玻璃从0开始慢慢变模糊
            TabletManager::self()->showBackground(0);
        } else if (frontWindowType() == FrontWindowType::NormalWindow) {
            m_gestureFollowWindow->move(QPoint(m_panelOriginXCoordinate, getScreenHeight()));
            m_gestureFollowWindow->showClient();
            m_startGestureFollow = true;
        } else if (frontWindowType() == FrontWindowType::UtilityWindow) {
            m_startGestureFollow = false;
        }
    } else {
        m_startGestureFollow = false;
    }

#ifndef QT_NO_DEBUG
    qDebug() << "当前手势跟随是否开启： " << m_startGestureFollow << "当前窗口的类型为： " << frontWindowType();
#endif
}

void GestureFollowInputFilter::touchMotion(qint32 id, const QPointF &pos, quint32 time)
{

    if (gestureInfo.find(id) == gestureInfo.end()) {
        return;
    }

    if (TabletManager::self()->tabletMode() && frontWindowType() == FrontWindowType::DesktopWindow) {
        desktopOpacityAnimation(pos.y());
    }

    if (pos.y() > m_YcriticalValue && m_startGestureFollow && m_gestureFollowWindow != nullptr) {
        if (pos.y() > m_panelOriginYCoordinate) {
            m_gestureFollowWindow->move(QPoint(m_panelOriginXCoordinate, pos.y()));
        } else {
            m_gestureFollowWindow->move(QPoint(m_panelOriginXCoordinate, m_panelOriginYCoordinate));
        }

        if (pos.y() <= m_panelDisplayYCoordinate) {
            m_tabletPanelhide = false;
        } else {
            m_tabletPanelhide = true;
        }
    } else if (pos.y() <= m_YcriticalValue
               && gestureInfo[m_gestureId].y() >= getScreenHeight() - 10
               && frontWindowType() != FrontWindowType::UtilityWindow
               && isAwakenViewGesture()) {
        if (m_multitaskViewDisplay) {
            return;
        }

        QtConcurrent::run(this, &GestureFollowInputFilter::sendTriggerMultitaskView);

        if (m_gestureFollowWindow != nullptr) {
            m_gestureFollowWindow->hideClient();
        }
        QDBusMessage message = QDBusMessage::createMethodCall("org.kde.KWin",
                                                              "/MultitaskView",
                                                              "org.kde.KWin.MultitaskView",
                                                              "show");
        QDBusMessage response = QDBusConnection::sessionBus().call(message);

        if (response.type() == QDBusMessage::ReplyMessage) {
            m_multitaskViewDisplay = true;
            m_tabletPanelhide = true;
        }

        QtConcurrent::run(this,
                          &GestureFollowInputFilter::sendMultitaskViewDbusResult,
                          static_cast<quint32>(response.type()));
    }
}

void GestureFollowInputFilter::touchUp(qint32 id, quint32 time)
{

    if (m_gestureId != -1 && TabletManager::self()->tabletMode() && frontWindowType() == FrontWindowType::DesktopWindow
        && gestureInfo[m_gestureId].y() >= getScreenHeight() - 10) {
        TabletManager::self()->hideBackground();
    }

    if (m_gestureId == id) {
        m_gestureId = -1;
        gestureInfo.clear();
#ifndef QT_NO_DEBUG
        qDebug() << "gestureInfo.size() is ：" << gestureInfo.size();
#endif
    } else {
        return;
    }

    m_multitaskViewDisplay = false;
    if (!m_startGestureFollow || m_gestureFollowWindow == nullptr) {
        return;
    }

    if (m_tabletPanelhide) {
        m_gestureFollowWindow->hideClient();
    } else {
        m_gestureFollowWindow->move(QPoint(m_panelOriginXCoordinate, m_panelOriginYCoordinate));
    }
}

void GestureFollowInputFilter::handlePressEvent(int x, int y)
{
    if (!m_gestureFollowWindow || m_gestureFollowWindow->isHiddenInternal()) {
        return;
    }

    QRect taskbarRect(m_panelOriginXCoordinate, m_panelOriginYCoordinate, m_gestureFollowWindow->width(), m_gestureFollowWindow->height());
    if (taskbarRect.contains(x, y)) {
        m_clickPositionInPanel = true;
        return;
    }
    m_clickPositionInPanel = false;
    m_gestureFollowWindow->hideClient();
}

void GestureFollowInputFilter::pointerEvent(MouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *cur = static_cast<QMouseEvent *>(event);
        handlePressEvent(cur->pos().x(), cur->pos().y());
    }
}

}
