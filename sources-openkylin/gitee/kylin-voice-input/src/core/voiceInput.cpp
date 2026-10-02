/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "voiceInput.h"
#include "audioInput.h"
#include "shortcut.h"

#include <QDateTime>
#include <QTimer>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include "logger.h"
#include <QDBusReply>
#include <QString>

#include "voiceToStr.h"
#include "voiceInputWindow.h"
#include "tipDialog.h"

namespace {

constexpr char kVoiceWingDBusService[] = "org.fcitx.Fcitx5";
constexpr char kVoiceWingDBusPath[] = "/voice";
constexpr char kVoiceWingDBusInterface[] = "org.fcitx.Fcitx5.Voice";

constexpr char kKylinAiAssistantService[] = "org.ukui.kylin_aiassistant";
constexpr char kKylinAiAssistantPath[] = "/org/ukui/kylin_aiassistant";
constexpr char kKylinAiAssistantInterface[] = "org.ukui.kylin_aiassistant";

constexpr char kKylinPearlService[] = "org.ukui.kylin_pearl";
constexpr char kKylinPearlPath[] = "/org/ukui/kylin_pearl";
constexpr char kKylinPearlInterface[] = "org.ukui.kylin_pearl";

constexpr char kVolumeControlService[] = "org.ukui.volume.control";
constexpr char kVolumeControlPath[] = "/org/ukui/volume/control";
constexpr char kVolumeControlInterface[] = "org.ukui.volume.control";

// type: 0 = input (microphone), consistent with getMute
constexpr int kVolumeControlDeviceTypeInput = 0;

QDBusInterface volumeControlInterface()
{
    return QDBusInterface(QString::fromLatin1(kVolumeControlService),
                          QString::fromLatin1(kVolumeControlPath),
                          QString::fromLatin1(kVolumeControlInterface),
                          QDBusConnection::sessionBus());
}

bool isValidInputDevice()
{
    QDBusInterface iface = volumeControlInterface();
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "Failed to connect to ukui volume control D-Bus service:"
                                     << iface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = iface.call(QStringLiteral("isValidDevice"),
                                        kVolumeControlDeviceTypeInput);
    if (!reply.isValid()) {
        VW_LOG(Logger::Level::Error) << "Failed to query input device validity:"
                                     << reply.error().message();
        return false;
    }

    return reply.value();
}

bool isInputDeviceMuted()
{
    QDBusInterface iface = volumeControlInterface();
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "Failed to connect to ukui volume control D-Bus service:"
                                     << iface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = iface.call(QStringLiteral("getMute"), kVolumeControlDeviceTypeInput, -1);
    if (!reply.isValid()) {
        VW_LOG(Logger::Level::Error) << "Failed to query input mute state:" << reply.error().message();
        return false;
    }

    return reply.value();
}

} // namespace

VoiceInput *VoiceInput::instance()
{
    static VoiceInput inst;
    return &inst;
}

VoiceInput::VoiceInput(QObject *parent)
    : QObject(parent)
{
    m_window = new VoiceInputWindow();
    m_window->hideWindow();
    connect(this, &VoiceInput::isRecognizingChanged, m_window, [this]() {
        m_window->updateIsRecognizing(m_isRecognizing);
    });

    Shortcut *s = Shortcut::instance();
    connect(s, &Shortcut::shortcutClicked, this, &VoiceInput::onShortcutClicked);
    connect(s, &Shortcut::shortcutLongPressed, this, &VoiceInput::onShortcutLongPressed);
    connect(s, &Shortcut::shortcutReleased, this, &VoiceInput::onShortcutReleased);

    AudioInput *audioInput = AudioInput::instance();
    connect(audioInput, &AudioInput::volumeChanged, m_window, &VoiceInputWindow::onVolumeLevelChanged);

    VoiceToStr *vt = VoiceToStr::instance();
    if (vt) {
        connect(vt, &VoiceToStr::voiceInputStarted, this, []() {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: started";
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::textRecognizing, this, [this](const QString &text) {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: interim ->" << text;
            if (!text.isEmpty()) {
                const QString incremental = text.mid(m_lastInterimLength);
                if (!incremental.isEmpty()) {
                    VW_LOG(Logger::Level::Debug) << "VoiceToStr: commit incremental ->" << incremental;
                    commitString(incremental, 0);
                }
                m_lastInterimLength = text.length();
            }
            setIsRecognizing(true);
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::textRecognized, this, [this](const QString &text) {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: final ->" << text;
            if (!text.isEmpty()) {
                VW_LOG(Logger::Level::Debug) << "VoiceToStr: commit final ->" << text;
                commitString(text, 1);
            }
            m_lastInterimLength = 0;
            setIsRecognizing(false);
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::voiceInputPaused, this, [this]() {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: paused/ended";
            setIsRecognizing(false);
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::errorOccurred, this, [this](int code, const QString &message) {
            VW_LOG(Logger::Level::Error) << "VoiceToStr error:" << code << message;
            setIsRecognizing(false);
            // 仅在语音输入已激活时弹窗（避免与触发前预检重复提示）
            if (m_voiceInputActive) {
                stopVoiceInput();
                QTimer::singleShot(0, this, [this]() { createTipDialog()->showNoVoiceModel(); });
            }
        }, Qt::UniqueConnection);
    }

    const bool focusSignalConnected = QDBusConnection::sessionBus().connect(
        QString::fromLatin1(kVoiceWingDBusService),
        QString::fromLatin1(kVoiceWingDBusPath),
        QString::fromLatin1(kVoiceWingDBusInterface),
        QStringLiteral("FocusChanged"),
        this,
        SLOT(onFocusChanged()));
    if (!focusSignalConnected) {
        VW_LOG(Logger::Level::Error) << "Failed to connect FocusChanged signal";
    }

}

void VoiceInput::onShortcutClicked()
{
    VW_LOG(Logger::Level::Debug) << "shortcut clicked";
    callKylinAiAssistantAsync(QStringLiteral("showOrHideView"));
}

void VoiceInput::callDBusServiceAsync(const char *service,
                                      const char *path,
                                      const char *interface,
                                      const QString &methodName,
                                      const QList<QVariant> &arguments)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        VW_LOG(Logger::Level::Error) << service << methodName
                                     << "failed: session bus not connected";
        return;
    }

    QDBusConnectionInterface *busIface = bus.interface();
    if (!busIface) {
        VW_LOG(Logger::Level::Error) << service << methodName
                                     << "failed: session bus interface unavailable";
        return;
    }

    if (!busIface->isServiceRegistered(QString::fromUtf8(service))) {
        VW_LOG(Logger::Level::Warning) << service << methodName
                                       << "skipped: service not registered";
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(service, path, interface, methodName);
    if (!arguments.isEmpty()) {
        msg.setArguments(arguments);
    }
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [service, methodName, watcher]() {
        QDBusPendingReply<void> reply = *watcher;
        if (reply.isError()) {
            VW_LOG(Logger::Level::Error) << service << methodName
                                         << "failed:" << reply.error().name()
                                         << reply.error().message();
        } else {
            VW_LOG(Logger::Level::Debug) << service << methodName << "succeeded";
        }
        watcher->deleteLater();
    });
}

void VoiceInput::callKylinAiAssistantAsync(const QString &methodName,
                                           const QList<QVariant> &arguments)
{
    callDBusServiceAsync(kKylinAiAssistantService,
                         kKylinAiAssistantPath,
                         kKylinAiAssistantInterface,
                         methodName,
                         arguments);
}

void VoiceInput::callKylinPearlAsync(const QString &methodName,
                                     const QList<QVariant> &arguments)
{
    callDBusServiceAsync(kKylinPearlService,
                         kKylinPearlPath,
                         kKylinPearlInterface,
                         methodName,
                         arguments);
}

bool VoiceInput::hasFocusWindow() const
{
    QDBusInterface iface(kVoiceWingDBusService, kVoiceWingDBusPath,
                         kVoiceWingDBusInterface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "fcitx5 VoiceWing DBus interface unavailable (HasFocusWindow)";
        return false;
    }

    QDBusReply<bool> reply = iface.call(QStringLiteral("HasFocusWindow"));
    if (!reply.isValid()) {
        VW_LOG(Logger::Level::Error) << "HasFocusWindow failed:" << reply.error().message();
        return false;
    }

    return reply.value();
}

void VoiceInput::commitString(const QString &text, int flags)
{
    QDBusInterface iface(kVoiceWingDBusService, kVoiceWingDBusPath,
                         kVoiceWingDBusInterface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "fcitx5 VoiceWing DBus interface unavailable (CommitString)";
        return;
    }

    QDBusMessage reply = iface.call(QStringLiteral("CommitString"), text, flags);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        VW_LOG(Logger::Level::Error) << "CommitString failed:" << reply.errorMessage();
    }
}

void VoiceInput::stopVoiceInput()
{
    m_voiceInputActive = false;
    m_lastInterimLength = 0;
    setIsRecognizing(false);
    AudioInput::instance()->stopMonitoring();
    m_window->hideWindow();

    // 释放快捷键：停止识别、断开连接并清理初始化
    VoiceToStr *vt = VoiceToStr::instance();
    if (vt) {
        vt->disconnectAndCleanup();
    }
}

void VoiceInput::onFocusChanged()
{
    VW_LOG(Logger::Level::Debug) << "FocusChanged: stop voice input and hide UI";
    stopVoiceInput();
}

TipDialog *VoiceInput::createTipDialog()
{
    if (!m_tipDialog) {
        m_tipDialog = new TipDialog();
    }
    return m_tipDialog;
}

void VoiceInput::onShortcutLongPressed()
{
    VW_LOG(Logger::Level::Debug) << "shortcut long pressed, checking input device availability";
    if (!isValidInputDevice()) {
        VW_LOG(Logger::Level::Debug) << "no valid input device, show unavailable dialog";
        QTimer::singleShot(0, this, [this]() { createTipDialog()->showUnavailable(); });
        return;
    }

    VW_LOG(Logger::Level::Debug) << "checking default source mute";
    if (isInputDeviceMuted()) {
        VW_LOG(Logger::Level::Debug) << "default input device is muted, show mute dialog";
        QTimer::singleShot(0, this, [this]() { createTipDialog()->showMuted(); });
        return;
    }

    VW_LOG(Logger::Level::Debug) << "checking focused input window";
    if (!hasFocusWindow()) {
        VW_LOG(Logger::Level::Debug) << "no focused input window, show AI ball";
        callKylinPearlAsync(QStringLiteral("showAIBall"), {true});
        return;
    }

    // 每次触发都同步初始化并连接 AI 模块；失败则弹窗提示
    VoiceToStr *vt = VoiceToStr::instance();
    if (!vt || !vt->initializeAndConnect()) {
        VW_LOG(Logger::Level::Error) << "AI module init/connect failed, show tip dialog";
        QTimer::singleShot(0, this, [this]() { createTipDialog()->showNoVoiceModel(); });
        return;
    }

    if (!vt->startRecognition()) {
        VW_LOG(Logger::Level::Error) << "startRecognition failed after init, show tip dialog";
        vt->disconnectAndCleanup();
        QTimer::singleShot(0, this, [this]() { createTipDialog()->showNoVoiceModel(); });
        return;
    }

    m_voiceInputActive = true;
    m_lastInterimLength = 0;
    m_window->showWindow();
    AudioInput::instance()->startMonitoring();
}

void VoiceInput::onShortcutReleased()
{
    if (!m_voiceInputActive) {
        VW_LOG(Logger::Level::Debug) << "voice input not active, hide AI ball";
        callKylinPearlAsync(QStringLiteral("showAIBall"), {false});
        return;
    }

    stopVoiceInput();
}

bool VoiceInput::isRecognizing() const
{
    return m_isRecognizing;
}

void VoiceInput::setIsRecognizing(bool isRecognizing)
{
    if (m_isRecognizing == isRecognizing) {
        return;
    }
    m_isRecognizing = isRecognizing;
    Q_EMIT isRecognizingChanged();
}
