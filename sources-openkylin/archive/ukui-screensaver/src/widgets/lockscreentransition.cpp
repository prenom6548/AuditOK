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
 */
#include "lockscreentransition.h"
#include "fullbackgroundwidget.h"
#include "lockwidget.h"
#include "lockmainuianim.h"

#include <QTimer>

LockScreenTransitionController::LockScreenTransitionController(QObject *parent)
    : QObject(parent)
{
}

void LockScreenTransitionController::cancelActive()
{
    if (m_failsafeTimer) {
        m_failsafeTimer->stop();
    }
    if (m_mainUiBarrierConn) {
        disconnect(m_mainUiBarrierConn);
        m_mainUiBarrierConn = QMetaObject::Connection();
    }
    if (m_wallpaperBarrierConn) {
        disconnect(m_wallpaperBarrierConn);
        m_wallpaperBarrierConn = QMetaObject::Connection();
    }
    m_pendingBarriers = 0;
    m_unlockCompletionEmitted = false;
    m_pendingUserName.clear();
}

void LockScreenTransitionController::onUnlockBarrierReached()
{
    if (m_unlockCompletionEmitted) {
        return;
    }
    if (m_pendingBarriers > 0) {
        --m_pendingBarriers;
    }
    if (m_pendingBarriers > 0) {
        return;
    }
    m_unlockCompletionEmitted = true;
    if (m_failsafeTimer) {
        m_failsafeTimer->stop();
    }
    if (m_mainUiBarrierConn) {
        disconnect(m_mainUiBarrierConn);
        m_mainUiBarrierConn = QMetaObject::Connection();
    }
    if (m_wallpaperBarrierConn) {
        disconnect(m_wallpaperBarrierConn);
        m_wallpaperBarrierConn = QMetaObject::Connection();
    }
    const QString userName = m_pendingUserName;
    m_pendingUserName.clear();
    Q_EMIT unlockFinished(userName);
}

void LockScreenTransitionController::playUnlock(LockWidget *lockWidget, FullBackgroundWidget *background,
                                                const QString &userName)
{
    if (!lockWidget || !background) {
        return;
    }

    cancelActive();
    m_pendingUserName = userName;
    m_pendingBarriers = 2;

    m_mainUiBarrierConn = connect(lockWidget, &LockWidget::mainUiExitAnimationFinished, this,
                                  &LockScreenTransitionController::onUnlockBarrierReached);
    m_wallpaperBarrierConn = connect(background, &FullBackgroundWidget::wallpaperTransitionFinished, this,
                                     &LockScreenTransitionController::onUnlockBarrierReached);

    if (!m_failsafeTimer) {
        m_failsafeTimer = new QTimer(this);
        m_failsafeTimer->setSingleShot(true);
        connect(m_failsafeTimer, &QTimer::timeout, this, [this]() {
            m_pendingBarriers = 0;
            onUnlockBarrierReached();
        });
    }
    m_failsafeTimer->start(LockMainUiAnim::kUnlockWallClockMs);

    lockWidget->startMainUiExitAnimation();
    background->startUnlockTransition();
}
