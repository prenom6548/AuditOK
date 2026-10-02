/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#include "accessibilityinterface.h"
#include <QDebug>
#include <QProcess>
#include <QDateTime>
//#include <KSharedConfig>
//#include <KConfigGroup>
#include <QDBusReply>
#include <QtConcurrent>
#include <kysdk/kysdk-system/libkysysinfo.h>

static constexpr char kKylinSpeechWarmupToken[] = "__KYLIN_SPEECH_WARMUP__";
static constexpr char kXunfeiSpeechWarmupToken[] = "__XUNFEI_SPEECH_WARMUP__";
// TTS 模块预热可能较久；禁止无限等待以免拖死 serviceManager。
static constexpr int kWarmupSpdSayTimeoutMs = 15000;

static QString speechWarmupModuleName()
{
    const QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    // openKylin 默认 kylin_speech，商业版默认 xunfei_speech；须与 Orca/speechd DefaultModule 一致。
    if (systemName.compare(QStringLiteral("openkylin"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("kylin_speech");
    }
    return QStringLiteral("xunfei_speech");
}

static const char *speechWarmupTokenForModule(const QString &module)
{
    return module == QStringLiteral("kylin_speech") ? kKylinSpeechWarmupToken
                                                    : kXunfeiSpeechWarmupToken;
}

static void warmupSpeechSessionWorker()
{
    qDebug() << "warmupSpeechSession: ensuring speech-dispatcher";

    // 先探测用户会话 speech-dispatcher 是否已 active，避免重复 systemctl start。
    QProcess check;
    check.start(QStringLiteral("systemctl"),
                {QStringLiteral("--user"), QStringLiteral("is-active"),
                 QStringLiteral("speech-dispatcher")});
    if (!check.waitForFinished(3000) || check.exitCode() != 0) {
        QProcess start;
        start.start(QStringLiteral("systemctl"),
                    {QStringLiteral("--user"), QStringLiteral("start"),
                     QStringLiteral("speech-dispatcher")});
        if (!start.waitForFinished(10000)) {
            qWarning() << "warmupSpeechSession: failed to start speech-dispatcher";
            return;
        }
    }

    const QString speechModule = speechWarmupModuleName();
    const char *warmupToken = speechWarmupTokenForModule(speechModule);
    qDebug() << "warmupSpeechSession: warming module" << speechModule;

    // 预热 TTS 模块（与 speechd DefaultModule / Orca 当前引擎对齐）
    QProcess warm;
    warm.start(QStringLiteral("spd-say"),
               {QStringLiteral("-o"), speechModule,
                 QStringLiteral("-l"), QStringLiteral("0"),
                 QStringLiteral("-m"), QStringLiteral("none"),
                 QStringLiteral("-P"), QStringLiteral("notification"),
                 QString::fromUtf8(warmupToken)});
    if (!warm.waitForFinished(kWarmupSpdSayTimeoutMs)) {
        warm.kill();
        qWarning() << "warmupSpeechSession: spd-say warmup timed out after"
                   << kWarmupSpdSayTimeoutMs << "ms";
        return;
    }
    qDebug() << "warmupSpeechSession: done exit" << warm.exitCode();
}

#define STYLE_NAME_KEY "style-name"
#define GNOME_A11Y_APPLICATIONS_SCHEMA "org.gnome.desktop.a11y.applications"
#define SCREEN_READER_ENABLED_KEY "screen-reader-enabled"
#define SCREEN_READER_ENABLED_UKUI_KEY "screenReaderEnabled"

static bool readScreenReaderEnabled(QGSettings *settings)
{
    if (!settings) {
        return false;
    }
    const QStringList keys = settings->keys();
    bool enabled = false;
    if (keys.contains(SCREEN_READER_ENABLED_KEY)) {
        enabled = enabled || settings->get(SCREEN_READER_ENABLED_KEY).toBool();
    }
    if (keys.contains(SCREEN_READER_ENABLED_UKUI_KEY)) {
        enabled = enabled || settings->get(SCREEN_READER_ENABLED_UKUI_KEY).toBool();
    }
    return enabled;
}

static bool readScreenReaderGnomeKey(QGSettings *settings)
{
    if (!settings) {
        return false;
    }
    const QStringList keys = settings->keys();
    if (keys.contains(SCREEN_READER_ENABLED_KEY)) {
        return settings->get(SCREEN_READER_ENABLED_KEY).toBool();
    }
    return readScreenReaderEnabled(settings);
}

static void setScreenReaderEnabled(QGSettings *settings, bool enabled)
{
    if (!settings) {
        return;
    }
    const QStringList keys = settings->keys();
    if (keys.contains(SCREEN_READER_ENABLED_KEY)) {
        settings->set(SCREEN_READER_ENABLED_KEY, enabled);
    }
    if (keys.contains(SCREEN_READER_ENABLED_UKUI_KEY)) {
        settings->set(SCREEN_READER_ENABLED_UKUI_KEY, enabled);
    }
}

AccessibilityInterface::AccessibilityInterface():UKUI::ServiceObject(nullptr)
{
    styleGsettings = UniversalInterface::self()->ukuiStyleGsettings();
    if (QGSettings::isSchemaInstalled(QByteArray(GNOME_A11Y_APPLICATIONS_SCHEMA))) {
        m_screenReaderGsettings = new QGSettings(QByteArray(GNOME_A11Y_APPLICATIONS_SCHEMA), QByteArray(), this);
    }
    m_wlcomRenderDBus = UniversalInterface::self()->wlcomDbus();
    m_wlcomInterface = UniversalInterface::self()->wlcomEffectDbusInterface();
    m_kwinInterface = UniversalInterface::self()->kwinMagnifierInterface();
    m_kwinEffectInterface = UniversalInterface::self()->kwinEffectInterface();
    m_wlcomInterface->asyncCall("EnableEffect", "zoom", true);  // 打开后才有Magnifier路径
    m_stopOrcaTimer = new QTimer(this);
    m_stopOrcaTimer->setSingleShot(true);
    connect(m_stopOrcaTimer, &QTimer::timeout, this, &AccessibilityInterface::deferredStopOrcaSlot);
    m_startOrcaRetryTimer = new QTimer(this);
    m_startOrcaRetryTimer->setSingleShot(true);
    connect(m_startOrcaRetryTimer, &QTimer::timeout, this, &AccessibilityInterface::deferredStartOrcaSlot);
    initConnect();
    // registerServiceByPlugin 会同步构造本对象；此处若直接预热 speechd/Orca 会卡死
    // ukui-session-service-manager，导致 org.ukui.ukcc.session 注册失败。
    QTimer::singleShot(0, this, [this]() { deferredBootScreenReaderIfNeeded(); });
}

AccessibilityInterface::~AccessibilityInterface()
{

}

QString AccessibilityInterface::getStyleName()
{
    if (styleGsettings->keys().contains("styleName")) {
        return styleGsettings->get(STYLE_NAME_KEY).toString();
    } else {
        qCritical() << "mStyleGsettings not contains the key: " << STYLE_NAME_KEY;
    }
    return QString();
}

void AccessibilityInterface::initConnect()
{
    connect(styleGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == STYLE_NAME_KEY) {
            Q_EMIT changed("styleName");
        }
    });
    if (m_screenReaderGsettings) {
        connect(m_screenReaderGsettings, &QGSettings::changed,
                this, &AccessibilityInterface::screenReaderEnabledChangedSlot);
    }
    QDBusConnection::sessionBus().connect("org.ukui.KWin",
                                          "/Magnifier",
                                          "org.ukui.Magnifier",
                                          "zoomStop",
                                          this,
                                          SLOT(zoomStopSlot()));
    QDBusConnection::sessionBus().connect("org.ukui.KWin",
                                          "/Magnifier",
                                          "org.ukui.Magnifier",
                                          "zoomStart",
                                          this,
                                          SLOT(zoomStartSlot()));
    QDBusConnection::sessionBus().connect("org.ukui.KWin",
                                          "/Magnifier",
                                          "org.ukui.KWin",
                                          "modeChanged",
                                          this,
                                          SLOT(modeChangedSlot(int)));
    QDBusConnection::sessionBus().connect("com.kylin.Wlcom",
                                          "/ColorFilter",
                                          "org.ukui.colorfilter",
                                          "colorFilterTypeChanged",
                                          this,
                                          SLOT(colorFilterTypeChangedSlot(QString)));
    QDBusConnection::sessionBus().connect("com.kylin.Wlcom",
                                          "/ColorFilter",
                                          "org.ukui.colorfilter",
                                          "statusChanged",
                                          this,
                                          SLOT(filterStatusChangedSlot(bool)));
    QDBusConnection::sessionBus().connect("com.kylin.Wlcom",
                                          "/ColorFilter",
                                          "org.ukui.colorfilter",
                                          "colorFilterShortCutEnabledChanged",
                                          this,
                                          SLOT(colorFilterShortCutEnabledChangedSlot(bool)));
}

bool AccessibilityInterface::isOrcaRunning() const
{
    QProcess process;
    // 匹配 "python3 /usr/bin/orca" 或 "/usr/bin/orca"，避免误匹配路径中含 orca 的进程
    process.start("pgrep", QStringList() << "-f" << "(^| )/usr/bin/orca( |$|--)");
    if (!process.waitForFinished(1000)) {
        qWarning() << Q_FUNC_INFO << "pgrep orca timeout";
        return false;
    }
    const bool running = (process.exitCode() == 0);
    qDebug() << Q_FUNC_INFO << "pgrep orca exit code" << process.exitCode() << "running" << running;
    return running;
}

bool AccessibilityInterface::isScreenReaderRunning()
{
    const bool running = isOrcaRunning();
    qDebug() << Q_FUNC_INFO << "isScreenReaderRunning" << running;
    return running;
}

void AccessibilityInterface::cancelStartOrcaRetry()
{
    m_startOrcaRetryAttempt = 0;
    if (m_startOrcaRetryTimer) {
        m_startOrcaRetryTimer->stop();
    }
}

void AccessibilityInterface::scheduleStartOrcaRetry()
{
    if (!m_startOrcaRetryTimer) {
        qWarning() << Q_FUNC_INFO << "m_startOrcaRetryTimer is null";
        return;
    }
    if (m_screenReaderGsettings && !readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader no longer enabled, cancel start retry";
        cancelStartOrcaRetry();
        return;
    }
    if (m_startOrcaRetryAttempt >= ORCA_START_RETRY_MAX_ATTEMPTS) {
        qWarning() << Q_FUNC_INFO << "startOrca retry exhausted after"
                   << ORCA_START_RETRY_MAX_ATTEMPTS << "attempts";
        cancelStartOrcaRetry();
        return;
    }
    m_startOrcaRetryAttempt++;
    qDebug() << Q_FUNC_INFO << "schedule startOrca retry" << m_startOrcaRetryAttempt
             << "of" << ORCA_START_RETRY_MAX_ATTEMPTS << "in" << ORCA_START_RETRY_MS << "ms";
    m_startOrcaRetryTimer->start(ORCA_START_RETRY_MS);
}

void AccessibilityInterface::deferredBootScreenReaderIfNeeded()
{
    if (m_screenReaderGsettings && readScreenReaderEnabled(m_screenReaderGsettings)) {
        ensureSpeechThenStartOrca();
    } else {
        scheduleSpeechSessionWarmup();
    }
}

void AccessibilityInterface::scheduleSpeechSessionWarmup()
{
    if (m_speechWarmupRunning.exchange(true)) {
        return;
    }
    QtConcurrent::run([this]() {
        warmupSpeechSessionWorker();
        QMetaObject::invokeMethod(
            this, &AccessibilityInterface::onSpeechWarmupFinished, Qt::QueuedConnection);
    });
}

void AccessibilityInterface::onSpeechWarmupFinished()
{
    m_speechWarmupRunning.store(false);
    if (!m_pendingOrcaStartAfterWarmup) {
        return;
    }
    m_pendingOrcaStartAfterWarmup = false;
    if (m_screenReaderGsettings && !readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader disabled after warmup, skip start";
        return;
    }
    startOrca();
}

void AccessibilityInterface::ensureSpeechThenStartOrca()
{
    if (m_screenReaderGsettings && !readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader disabled in gsettings, skip ensure";
        cancelStartOrcaRetry();
        m_pendingOrcaStartAfterWarmup = false;
        return;
    }
    cancelDeferredStopOrca();
    cancelStartOrcaRetry();

    // 查询用户会话 speech-dispatcher 是否已在运行；已 active 则 TTS 模块可用，
    // 跳过 spd-say 阻塞预热，直接拉起 Orca（关→开、连按开关时缩短等待）。
    QProcess check;
    check.start(QStringLiteral("systemctl"),
              {QStringLiteral("--user"), QStringLiteral("is-active"),
               QStringLiteral("speech-dispatcher")});
    if (check.waitForFinished(2000) && check.exitCode() == 0) {
        qDebug() << Q_FUNC_INFO << "speech-dispatcher active, start orca directly";
        m_pendingOrcaStartAfterWarmup = false;
        startOrca();
        return;
    }

    m_pendingOrcaStartAfterWarmup = true;
    if (m_speechWarmupRunning.load()) {
        qDebug() << Q_FUNC_INFO << "speech warmup already running, orca queued after warmup";
        return;
    }
    scheduleSpeechSessionWarmup();
}

void AccessibilityInterface::deferredStartOrcaSlot()
{
    startOrca();
}

void AccessibilityInterface::launchOrcaProcess()
{
    QString waylandDisplay = qEnvironmentVariable("WAYLAND_DISPLAY");
    if (waylandDisplay.isEmpty()) {
        waylandDisplay = "wayland-0";
    }
    const QString command = QString("WAYLAND_DISPLAY=%1 /usr/bin/orca > /tmp/ukui-orca.log 2>&1")
                                .arg(waylandDisplay);
    const bool started = QProcess::startDetached("/bin/sh", QStringList() << "-c" << command);
    qDebug() << Q_FUNC_INFO << "startDetached(/bin/sh -c orca) returned" << started
             << "WAYLAND_DISPLAY" << waylandDisplay << "log" << "/tmp/ukui-orca.log";
}

void AccessibilityInterface::startOrca()
{
    // 调用意图：仅当用户要开（gsettings true）时才会进入此函数。
    if (m_screenReaderGsettings && !readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader disabled in gsettings, skip start";
        cancelStartOrcaRetry();
        return;
    }

    if (isOrcaRunning()) {
        // 旧实例仍在退出：短间隔重试，避免 SIGKILL 误杀正在拉起的 Orca。
        qDebug() << Q_FUNC_INFO << "orca still present after user enable, retry start later";
        scheduleStartOrcaRetry();
        return;
    }

    cancelStartOrcaRetry();
    launchOrcaProcess();
}

void AccessibilityInterface::requestOrcaShutdown()
{
    cancelStartOrcaRetry();
    // 关→开竞态：迟到的 disabled 信号不得再预约 SIGKILL。
    if (m_screenReaderGsettings && readScreenReaderEnabled(m_screenReaderGsettings)) {
        cancelDeferredStopOrca();
        qDebug() << Q_FUNC_INFO << "screen reader already enabled, skip requestOrcaShutdown";
        return;
    }
    if (!isOrcaRunning()) {
        cancelDeferredStopOrca();
        qDebug() << Q_FUNC_INFO << "orca not running, skip requestOrcaShutdown";
        return;
    }

    // Orca 已通过 gsettings 自行优雅退出；仅安排 SIGKILL 兜底，避免 SIGTERM 打断告别/重开。
    qDebug() << Q_FUNC_INFO << "schedule SIGKILL fallback in" << ORCA_STOP_DEFER_MS << "ms (no SIGTERM)";
    scheduleStopOrca();
}

void AccessibilityInterface::scheduleStopOrca()
{
    if (!m_stopOrcaTimer) {
        qWarning() << Q_FUNC_INFO << "m_stopOrcaTimer is null";
        return;
    }
    // 预约前再确认仍为关闭意图，避免重开后仍启动定时器。
    if (m_screenReaderGsettings && readScreenReaderEnabled(m_screenReaderGsettings)) {
        cancelDeferredStopOrca();
        qDebug() << Q_FUNC_INFO << "screen reader enabled, skip schedule stopOrca";
        return;
    }
    if (!isOrcaRunning()) {
        qDebug() << Q_FUNC_INFO << "orca not running, skip schedule stopOrca";
        m_stopOrcaTimer->stop();
        return;
    }
    qDebug() << Q_FUNC_INFO << "schedule SIGKILL fallback in" << ORCA_STOP_DEFER_MS << "ms";
    m_stopOrcaTimer->start(ORCA_STOP_DEFER_MS);
}

void AccessibilityInterface::cancelDeferredStopOrca()
{
    if (!m_stopOrcaTimer) {
        return;
    }
    if (m_stopOrcaTimer->isActive()) {
        qDebug() << Q_FUNC_INFO << "screen reader re-enabled, cancel deferred stopOrca";
    }
    m_stopOrcaTimer->stop();
}

void AccessibilityInterface::deferredStopOrcaSlot()
{
    if (!m_screenReaderGsettings) {
        qDebug() << Q_FUNC_INFO << "deferred stopOrca firing";
        stopOrca();
        return;
    }
    // 任一开关为开即取消强杀（与 readScreenReaderEnabled 一致，避免双 key 不同步误杀）。
    if (readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader re-enabled, cancel deferred stopOrca";
        return;
    }
    qDebug() << Q_FUNC_INFO << "deferred stopOrca firing";
    stopOrca();
}

void AccessibilityInterface::stopOrca()
{
    if (!isOrcaRunning()) {
        qDebug() << Q_FUNC_INFO << "orca not running, skip (already exited via gsettings shutdown)";
        return;
    }
    // 兜底触发时若用户已重开，绝不再 SIGKILL（防止定时器与 gsettings 竞态）。
    if (m_screenReaderGsettings && readScreenReaderEnabled(m_screenReaderGsettings)) {
        qDebug() << Q_FUNC_INFO << "screen reader re-enabled, skip SIGKILL";
        return;
    }

    qDebug() << Q_FUNC_INFO << "orca still running after grace period, SIGKILL";
    QProcess killProcess;
    killProcess.start("pkill", QStringList() << "-KILL" << "-f" << "(^| )/usr/bin/orca( |$|--)");
    if (!killProcess.waitForFinished(1000)) {
        qWarning() << Q_FUNC_INFO << "pkill -KILL orca timeout";
    } else {
        qDebug() << Q_FUNC_INFO << "pkill -KILL orca exit code" << killProcess.exitCode() << "orca running" << isOrcaRunning();
    }
}

void AccessibilityInterface::toggleScreenReader()
{
    if (!m_screenReaderGsettings) {
        qWarning() << Q_FUNC_INFO << "m_screenReaderGsettings is null";
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const bool orcaRunning = isOrcaRunning();
    qDebug() << Q_FUNC_INFO << "orcaRunning" << orcaRunning;

    // Orca 在跑时：开关已开 → 关闭；开关已关但进程仍在 → 强制结束（勿当成重开）。
    // 旧逻辑在「key=false 且 orca 仍在」时会 set true 重开；启动扫描卡死主循环时
    // 用户连按关闭会反复重开，表现为关不掉。
    if (orcaRunning) {
        if (m_lastToggleMs > 0 && nowMs - m_lastToggleMs < TOGGLE_DEBOUNCE_MS) {
            qDebug() << Q_FUNC_INFO << "debounced toggle" << (nowMs - m_lastToggleMs) << "ms";
            return;
        }
        m_lastToggleMs = nowMs;

        if (readScreenReaderGnomeKey(m_screenReaderGsettings)) {
            qDebug() << Q_FUNC_INFO << "orca running, disable screen reader";
            cancelDeferredStopOrca();
            cancelStartOrcaRetry();
            setScreenReaderEnabled(m_screenReaderGsettings, false);
            return;
        }

        qDebug() << Q_FUNC_INFO << "orca running but screen reader off, force stop";
        cancelStartOrcaRetry();
        setScreenReaderEnabled(m_screenReaderGsettings, false);
        cancelDeferredStopOrca();
        stopOrca();
        return;
    }

    cancelDeferredStopOrca();
    cancelStartOrcaRetry();
    setScreenReaderEnabled(m_screenReaderGsettings, true);
    ensureSpeechThenStartOrca();
}

void AccessibilityInterface::screenReaderEnabledChangedSlot(const QString &key)
{
    if (!m_screenReaderGsettings) {
        qDebug() << Q_FUNC_INFO << "m_screenReaderGsettings not exist, skip" << key;
        return;
    }
    if (key != SCREEN_READER_ENABLED_KEY && key != SCREEN_READER_ENABLED_UKUI_KEY) {
        qDebug() << Q_FUNC_INFO << "skip unrelated key" << key;
        return;
    }

    // 以本次变更的 key 为准，勿用 OR 误判（关 screen-reader-enabled 时 UKUI key 仍 true 会误触发重开）。
    const bool enabled = m_screenReaderGsettings->get(key).toBool();
    qDebug() << Q_FUNC_INFO << "screenReaderEnabledChangedSlot key" << key << "status" << enabled;
    if (enabled) {
        setScreenReaderEnabled(m_screenReaderGsettings, true);
        // 打开时一律取消 SIGKILL 兜底；关→开竞态下迟到的 disable 也可能已重新预约。
        cancelDeferredStopOrca();
        cancelStartOrcaRetry();
        m_pendingOrcaStartAfterWarmup = false;
        if (isOrcaRunning()) {
            qDebug() << Q_FUNC_INFO << "orca running, defer re-enable to Orca onEnabledChanged";
            return;
        }
        ensureSpeechThenStartOrca();
        return;
    }
    // enabled==false：若另一 key 仍为开（关→开竞态/双 key 异步），勿强制全关、勿预约强杀。
    if (readScreenReaderEnabled(m_screenReaderGsettings)) {
        cancelDeferredStopOrca();
        qDebug() << Q_FUNC_INFO << "another key still enabled, skip shutdown";
        return;
    }
    setScreenReaderEnabled(m_screenReaderGsettings, false);
    m_pendingOrcaStartAfterWarmup = false;
    requestOrcaShutdown();
}

void AccessibilityInterface::zoomStartSlot() {
    qDebug() << Q_FUNC_INFO << "signal zoomStart";
    emit statusChanged(true);
}

void AccessibilityInterface::zoomStopSlot() {
    qDebug() << Q_FUNC_INFO << "signal zoomStop";
    emit statusChanged(false);
}

void AccessibilityInterface::modeChangedSlot(int mode) {
    emit modeChanged(mode);
}

void AccessibilityInterface::colorFilterTypeChangedSlot(QString type) {
    emit colorFilterTypeChanged(type);
}

void AccessibilityInterface::filterStatusChangedSlot(bool enabled) {
    qDebug() << Q_FUNC_INFO << "filterStatusChangedSlot" << enabled;
    emit filterStatusChanged(enabled);
}

void AccessibilityInterface::colorFilterShortCutEnabledChangedSlot(bool enabled) {
    emit colorFilterShortCutEnabledChanged(enabled);
}

int AccessibilityInterface::GetType()
{
    QDBusReply<int> zoomModeReply = m_kwinInterface->call("GetType");
    qDebug() << Q_FUNC_INFO << "GetType" << zoomModeReply.value();
    return zoomModeReply.value();
}

bool AccessibilityInterface::isEffectSupported() {
    QDBusReply<bool> isEffectSupportedReply = m_wlcomRenderDBus->call("IsHardwareRendering");
    qDebug() << Q_FUNC_INFO << "isEffectSupported" << isEffectSupportedReply.value();
    return isEffectSupportedReply.value();
}

bool AccessibilityInterface::isColorFilterShortCutEnabled() {
    QDBusReply<bool> isColorFilterShortCutEnabledReply = m_kwinEffectInterface->call("isColorFilterShortCutEnabled");
    qDebug() << Q_FUNC_INFO << "isColorFilterShortCutEnabled" << isColorFilterShortCutEnabledReply.value();
    return isColorFilterShortCutEnabledReply.value();
}

bool AccessibilityInterface::isColorFilterEnabled() {
    QDBusReply<bool> isColorFilterEnabledReply = m_kwinEffectInterface->call("isColorFilterEnabled");
    qDebug() << Q_FUNC_INFO << "isColorFilterEnabled" << isColorFilterEnabledReply.value();
    return isColorFilterEnabledReply.value();
}

QString AccessibilityInterface::getCurrentColorFilter() {
    QString currentColorFilter;
    QDBusReply<QString> curColorFilterReply = m_kwinEffectInterface->call("getCurrentColorFilter");
    if (curColorFilterReply.isValid()) {
        currentColorFilter = curColorFilterReply.value();
    }
    qDebug() << Q_FUNC_INFO << "getCurrentColorFilter" << currentColorFilter;
    return currentColorFilter;
}

void AccessibilityInterface::setZoomMode(int index) {
    uint32_t idx = index;
    m_kwinInterface->call("SetMagnifierMode", idx);
    qDebug() << Q_FUNC_INFO << "setZoomMode" << idx;
    // 控制面板下拉框靠 modeChanged 同步；合成器未必回传，DBus 设置后主动通知
    emit modeChanged(static_cast<int>(idx));
}

void AccessibilityInterface::setColorFilter(const QString& mode) {
    qDebug() << Q_FUNC_INFO << "" << mode;
    m_kwinEffectInterface->call("setColorFilter", mode);
}

void AccessibilityInterface::setZoomSwitch(bool status) {
    qDebug() << Q_FUNC_INFO << "setZoomSwitch" << status;
    if (status) {
        QDBusReply<int32_t> scaleReply = m_kwinInterface->call("GetScale");
        uint32_t scale = scaleReply.value();
        QDBusReply<int32_t> modeReply = m_kwinInterface->call("GetType");
        uint32_t mode = modeReply.value();
        qDebug() << Q_FUNC_INFO << "StartZoom" << "mode" << mode << "scale" << scale << scaleReply.isValid() << modeReply.isValid();
        m_kwinInterface->call("StartZoom", mode, scale);
    } else {
        m_kwinInterface->call("StopZoom");
    }
}

void AccessibilityInterface::setColorFilterSwitch(bool toggled) {
    if (toggled) {
        qDebug() << Q_FUNC_INFO << "openColorFilter" << toggled;
        m_kwinEffectInterface->call("openColorFilter");
    } else {
        qDebug() << Q_FUNC_INFO << "closeColorFilter" << toggled;
        m_kwinEffectInterface->call("closeColorFilter");
    }
}

void AccessibilityInterface::setColorFilterShortCutEnabled(bool enabled) {
    qDebug() << Q_FUNC_INFO << "setColorFilterShortCutEnabled" << enabled;
    m_kwinEffectInterface->call("setColorFilterShortCutEnabled", enabled);
}

bool AccessibilityInterface::isZooming()
{
    QDBusReply<bool> isZoomingReply = m_kwinInterface->call("IsZooming");
    qDebug() << Q_FUNC_INFO << "IsZooming" << isZoomingReply.value();
    return isZoomingReply.value();
}
