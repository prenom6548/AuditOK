/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef ACCESSIBILITYINTERFACE_H
#define ACCESSIBILITYINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QDBusArgument>
#include <QVariant>
#include <QDBusMetaType>
#include <QGSettings>
#include <QTimer>
#include <atomic>
#include "../../universalinterface.h"

class AccessibilityInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Accessibility")
    Q_PROPERTY(QString styleName READ getStyleName)

public:
    AccessibilityInterface();
    ~AccessibilityInterface();
    void initConnect();

protected:
    QString getStyleName();

public Q_SLOTS:
    bool isZooming();
    int GetType();
    bool isEffectSupported();
    bool isColorFilterShortCutEnabled();
    bool isColorFilterEnabled();
    QString getCurrentColorFilter();
    void setZoomMode(int index);
    void setColorFilter(const QString& mode);
    void setZoomSwitch(bool status);
    void setColorFilterSwitch(bool status);
    void setColorFilterShortCutEnabled(bool enabled);
    bool isScreenReaderRunning();
    void toggleScreenReader();

private Q_SLOTS:
    void zoomStartSlot();
    void zoomStopSlot();
    void modeChangedSlot(int mode);
    void colorFilterTypeChangedSlot(QString type);
    void filterStatusChangedSlot(bool enabled);
    void colorFilterShortCutEnabledChangedSlot(bool enabled);
    void screenReaderEnabledChangedSlot(const QString &key);
    void deferredStopOrcaSlot();
    void deferredStartOrcaSlot();
    void onSpeechWarmupFinished();

Q_SIGNALS:
    void changed(QString keyName);
    void statusChanged(bool status);
    void filterStatusChanged(bool status);
    void modeChanged(int mode);
    void colorFilterTypeChanged(QString type);
    void colorFilterShortCutEnabledChanged(bool status);

private:
    bool isOrcaRunning() const;
    void deferredBootScreenReaderIfNeeded();
    void scheduleSpeechSessionWarmup();
    void ensureSpeechThenStartOrca();
    void startOrca();
    void launchOrcaProcess();
    void scheduleStartOrcaRetry();
    void cancelStartOrcaRetry();
    void requestOrcaShutdown();
    void scheduleStopOrca();
    void cancelDeferredStopOrca();
    void stopOrca();

    // gsettings 触发 Orca 自行退出；超时后 SIGKILL 兜底（不发 SIGTERM，以免打断告别/重开）。
    static constexpr int ORCA_STOP_DEFER_MS = 8000;
    // 快捷键连按防抖：仅用于关闭路径（orca 在跑时）。
    static constexpr int TOGGLE_DEBOUNCE_MS = 400;
    // 用户开启意图：旧 Orca 仍在退出时，等待进程消失后再拉起。
    static constexpr int ORCA_START_RETRY_MS = 200;
    static constexpr int ORCA_START_RETRY_MAX_ATTEMPTS = 30;

    QTimer *m_stopOrcaTimer = nullptr;
    QTimer *m_startOrcaRetryTimer = nullptr;
    int m_startOrcaRetryAttempt = 0;
    std::atomic<bool> m_speechWarmupRunning{false};
    bool m_pendingOrcaStartAfterWarmup = false;
    qint64 m_lastToggleMs = 0;

    QGSettings *styleGsettings = nullptr;
    QGSettings *m_screenReaderGsettings = nullptr;
    QDBusInterface* m_wlcomInterface = nullptr;
    QDBusInterface* m_wlcomRenderDBus = nullptr;
    QDBusInterface* m_kwinInterface = nullptr;
    QDBusInterface* m_kwinEffectInterface = nullptr;
};

#endif // ACCESSIBILITYINTERFACE_H
