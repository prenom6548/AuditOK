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
#include "microphoneMuteDialog.h"

namespace {

constexpr char kVoiceWingDBusService[] = "org.fcitx.Fcitx5";
constexpr char kVoiceWingDBusPath[] = "/voice";
constexpr char kVoiceWingDBusInterface[] = "org.fcitx.Fcitx5.Voice";

constexpr char kKylinAiAssistantService[] = "org.ukui.kylin_aiassistant";
constexpr char kKylinAiAssistantPath[] = "/org/ukui/kylin_aiassistant";
constexpr char kKylinAiAssistantInterface[] = "org.ukui.kylin_aiassistant";

constexpr char kVolumeControlService[] = "org.ukui.volume.control";
constexpr char kVolumeControlPath[] = "/org/ukui/volume/control";
constexpr char kVolumeControlInterface[] = "org.ukui.volume.control";

bool isInputMuted()
{
    QDBusInterface iface(QString::fromLatin1(kVolumeControlService),
                         QString::fromLatin1(kVolumeControlPath),
                         QString::fromLatin1(kVolumeControlInterface),
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "Failed to connect to ukui volume control D-Bus service:"
                                     << iface.lastError().message();
        return false;
    }

    QDBusReply<bool> reply = iface.call(QStringLiteral("getMute"), 0, -1);
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

    m_muteDialog = new MicrophoneMuteDialog();
    m_muteDialog->hide();

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

        connect(vt, &VoiceToStr::textRecognizing, m_window, [this](const QString &text) {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: interim ->" << text;
            if (!text.isEmpty()) {
                const QString incremental = text.mid(m_lastInterimLength);
                if (!incremental.isEmpty()) {
                    commitString(incremental, 0);
                }
                m_lastInterimLength = text.length();
            }
            m_window->onTextRecognizing();
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::textRecognized, this, [this](const QString &text) {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: final ->" << text;
            if (!text.isEmpty()) {
                commitString(text, 1);
            }
            m_lastInterimLength = 0;
            m_window->onTextRecognized();
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::voiceInputPaused, m_window, [this]() {
            VW_LOG(Logger::Level::Debug) << "VoiceToStr: paused/ended";
            m_window->onTextRecognized();
        }, Qt::UniqueConnection);

        connect(vt, &VoiceToStr::errorOccurred, m_window, [this](int, const QString &) {
            m_window->onTextRecognized();
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

void VoiceInput::callKylinAiAssistantAsync(const QString &methodName,
                                           const QList<QVariant> &arguments)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        VW_LOG(Logger::Level::Error) << "kylin_aiassistant" << methodName
                                     << "failed: session bus not connected";
        return;
    }

    QDBusConnectionInterface *busIface = bus.interface();
    if (!busIface) {
        VW_LOG(Logger::Level::Error) << "kylin_aiassistant" << methodName
                                     << "failed: session bus interface unavailable";
        return;
    }

    if (!busIface->isServiceRegistered(QString::fromUtf8(kKylinAiAssistantService))) {
        VW_LOG(Logger::Level::Warning) << "kylin_aiassistant" << methodName
                                       << "skipped: service not registered";
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(kKylinAiAssistantService,
                                                        kKylinAiAssistantPath,
                                                        kKylinAiAssistantInterface,
                                                        methodName);
    if (!arguments.isEmpty()) {
        msg.setArguments(arguments);
    }
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [methodName, watcher]() {
        QDBusPendingReply<void> reply = *watcher;
        if (reply.isError()) {
            VW_LOG(Logger::Level::Error) << "kylin_aiassistant" << methodName
                                         << "failed:" << reply.error().name()
                                         << reply.error().message();
        } else {
            VW_LOG(Logger::Level::Debug) << "kylin_aiassistant" << methodName << "succeeded";
        }
        watcher->deleteLater();
    });
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

QString VoiceInput::appName() const
{
    QDBusInterface iface(kVoiceWingDBusService, kVoiceWingDBusPath,
                         kVoiceWingDBusInterface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        VW_LOG(Logger::Level::Error) << "fcitx5 VoiceWing DBus interface unavailable (AppName)";
        return {};
    }

    QDBusReply<QString> reply = iface.call(QStringLiteral("AppName"));
    if (!reply.isValid()) {
        VW_LOG(Logger::Level::Error) << "AppName failed:" << reply.error().message();
        return {};
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
    m_focusAppName.clear();
    m_lastInterimLength = 0;
    AudioInput::instance()->stopMonitoring();
    m_window->hideWindow();

    VoiceToStr *vt = VoiceToStr::instance();
    if (vt) {
        vt->stopRecognition();
    }
}

void VoiceInput::onFocusChanged()
{
    VW_LOG(Logger::Level::Info) << "FocusChanged: stop voice input and hide UI";
    stopVoiceInput();
}

void VoiceInput::onShortcutLongPressed()
{
    VW_LOG(Logger::Level::Info) << "shortcut long pressed, checking default source mute";
    if (isInputMuted()) {
        VW_LOG(Logger::Level::Info) << "default input device is muted, show mute dialog";
        QTimer::singleShot(0, m_muteDialog, [this]() { m_muteDialog->showCentered(); });
        return;
    }

    if (!hasFocusWindow()) {
        VW_LOG(Logger::Level::Debug) << "no focused input window, show AI ball";
        callKylinAiAssistantAsync(QStringLiteral("showAIBall"), {true});
        return;
    }
    m_focusAppName = appName();
    VW_LOG(Logger::Level::Info) << "focused input window, appName =" << m_focusAppName
                           << ", start voice input";

    m_voiceInputActive = true;
    m_lastInterimLength = 0;
    m_window->showWindow();
    AudioInput::instance()->startMonitoring();

    VoiceToStr *vt = VoiceToStr::instance();
    if (vt) {
        vt->startRecognition();
    }
}

void VoiceInput::onShortcutReleased()
{
    if (!m_voiceInputActive) {
        VW_LOG(Logger::Level::Debug) << "voice input not active, hide AI ball";
        callKylinAiAssistantAsync(QStringLiteral("showAIBall"), {false});
        return;
    }

    stopVoiceInput();
}
