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
 * 解锁动效编排：主 UI 退场与壁纸过渡并行，以 finished 对齐，墙钟作兜底。
 */
#ifndef LOCKSCREENTRANSITION_H
#define LOCKSCREENTRANSITION_H

#include <QObject>
#include <QString>

class LockWidget;
class FullBackgroundWidget;
class QTimer;

class LockScreenTransitionController : public QObject
{
    Q_OBJECT
public:
    explicit LockScreenTransitionController(QObject *parent = nullptr);

    void playUnlock(LockWidget *lockWidget, FullBackgroundWidget *background, const QString &userName);

Q_SIGNALS:
    void unlockFinished(const QString &userName);

private:
    void cancelActive();
    void onUnlockBarrierReached();

    QTimer *m_failsafeTimer = nullptr;
    QMetaObject::Connection m_mainUiBarrierConn;
    QMetaObject::Connection m_wallpaperBarrierConn;
    QString m_pendingUserName;
    int m_pendingBarriers = 0;
    bool m_unlockCompletionEmitted = false;
};

#endif // LOCKSCREENTRANSITION_H
