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

#include "voiceInputWindow.h"
#include "themeIconProvider.h"

#include <QGuiApplication>
#include "logger.h"
#include <QQmlContext>
#include <QQmlError>
#include <QQuickItem>
#include <QScreen>
#include <QSurfaceFormat>

VoiceInputWindow::VoiceInputWindow(QWindow *parent)
    : QQuickView(parent)
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(8);
    setFormat(format);

    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setColor(Qt::transparent);
    setResizeMode(QQuickView::SizeRootObjectToView);

    m_loadingDelayTimer = new QTimer(this);
    m_loadingDelayTimer->setSingleShot(true);
    m_loadingDelayTimer->setInterval(kLoadingDelayMs);
    connect(m_loadingDelayTimer, &QTimer::timeout, this, &VoiceInputWindow::startLoadingAnimation);

    m_loadingCycleTimer = new QTimer(this);
    m_loadingCycleTimer->setInterval(kLoadingIntervalMs);
    connect(m_loadingCycleTimer, &QTimer::timeout, this, &VoiceInputWindow::advanceLoadingIcon);

    m_volumeSampleTimer = new QTimer(this);
    m_volumeSampleTimer->setInterval(kVolumeSampleIntervalMs);
    connect(m_volumeSampleTimer, &QTimer::timeout,
            this, &VoiceInputWindow::onVolumeSampleTimeout);

    rootContext()->setContextProperty(QStringLiteral("voiceWindow"), this);
    initThemeColor();
    setSource(QUrl(QStringLiteral("qrc:/voiceInputWindow.qml")));

    if (status() == QQuickView::Error) {
        VW_LOG(Logger::Level::Error) << "Failed to load voiceInputWindow.qml:" << errors();
    } else {
        updateIconSource();
    }

    resize(hostSize(), hostSize());
    initWindowHelper();
}

int VoiceInputWindow::hostSize() const
{
    return m_windowSize + kPulseExpand;
}

int VoiceInputWindow::pulseToken() const
{
    return m_pulseToken;
}

void VoiceInputWindow::playRecognizingPulse()
{
    ++m_pulseToken;
    Q_EMIT pulseTokenChanged();
}

void VoiceInputWindow::onTextRecognizing()
{
    playRecognizingPulse();

    if (!kEnableLoadingIcon) {
        return;
    }

    if (!m_isLoadingIcon) {
        m_loadingDelayTimer->start();
    }
}

void VoiceInputWindow::onTextRecognized()
{
    m_loadingDelayTimer->stop();
    stopLoadingAnimation();
}

QString VoiceInputWindow::iconNameForVolumeLevel(qreal level)
{
    const int pct = qRound(level * 100.0);
    if (pct > 35) {
        return QStringLiteral("microphone-sensitivity-high-symbolic");
    }
    if (pct > 15) {
        return QStringLiteral("microphone-sensitivity-medium-symbolic");
    }
    if (pct > 5) {
        return QStringLiteral("microphone-sensitivity-low-symbolic");
    }
    return QStringLiteral("audio-input-microphone-symbolic");
}

void VoiceInputWindow::onVolumeSampleTimeout()
{
    if (m_isLoadingIcon) {
        return;
    }
    setDisplayIcon(iconNameForVolumeLevel(m_periodMaxVolumeLevel));
    m_periodMaxVolumeLevel = 0.0;
}

void VoiceInputWindow::onVolumeLevelChanged(qreal level)
{
    m_periodMaxVolumeLevel = qMax(m_periodMaxVolumeLevel, level);
}

void VoiceInputWindow::setDisplayIcon(const QString &iconName)
{
    if (m_iconName == iconName) {
        return;
    }

    m_iconName = iconName;
    updateIconSource();
}

void VoiceInputWindow::restoreDefaultIcon()
{
    m_isLoadingIcon = false;
    m_loadingIndex = 0;
    setDisplayIcon(m_defaultIconName);
}

void VoiceInputWindow::startLoadingAnimation()
{
    if (m_isLoadingIcon) {
        return;
    }

    m_isLoadingIcon = true;
    m_loadingIndex = 0;
    advanceLoadingIcon();
    m_loadingCycleTimer->start();
}

void VoiceInputWindow::stopLoadingAnimation()
{
    m_loadingCycleTimer->stop();
    if (!m_isLoadingIcon && m_iconName == m_defaultIconName) {
        return;
    }
    restoreDefaultIcon();
}

void VoiceInputWindow::advanceLoadingIcon()
{
    setDisplayIcon(QStringLiteral("ukui-loading-%1-symbolic").arg(m_loadingIndex));
    m_loadingIndex = (m_loadingIndex + 1) % kLoadingIconCount;
}

QColor VoiceInputWindow::parseThemeColor(const QString &value)
{
    QString cleaned = value.trimmed();
    if (cleaned.startsWith(QLatin1Char('\'')) && cleaned.endsWith(QLatin1Char('\''))) {
        cleaned = cleaned.mid(1, cleaned.size() - 2);
    }

    const QStringList parts = cleaned.split(QLatin1Char(','), QString::SkipEmptyParts);
    if (parts.size() < 3) {
        return QColor();
    }

    bool ok = false;
    const int r = parts.at(0).trimmed().toInt(&ok);
    if (!ok) {
        return QColor();
    }
    const int g = parts.at(1).trimmed().toInt(&ok);
    if (!ok) {
        return QColor();
    }
    const int b = parts.at(2).trimmed().toInt(&ok);
    if (!ok) {
        return QColor();
    }

    int alpha = 255;
    if (parts.size() >= 4) {
        const qreal a = parts.at(3).trimmed().toDouble(&ok);
        if (!ok) {
            return QColor();
        }
        alpha = a <= 1.0 ? qRound(a * 255) : qRound(a);
    }

    return QColor(r, g, b, alpha);
}

void VoiceInputWindow::initThemeColor()
{
    if (!QGSettings::isSchemaInstalled(STYLE_SCHEMA_ID)) {
        VW_LOG(Logger::Level::Error) << "GSettings schema not installed:" << STYLE_SCHEMA_ID;
        return;
    }

    m_styleSettings = new QGSettings(QByteArray(STYLE_SCHEMA_ID), QByteArray(), this);
    connect(m_styleSettings, &QGSettings::changed,
            this, &VoiceInputWindow::onStyleSettingsChanged);

    updateThemeColorFromSettings();
}

void VoiceInputWindow::updateThemeColorFromSettings()
{
    if (!m_styleSettings || !m_styleSettings->keys().contains(KEY_THEME_COLOR)) {
        return;
    }

    const QColor color = parseThemeColor(m_styleSettings->get(KEY_THEME_COLOR).toString());
    if (!color.isValid()) {
        VW_LOG(Logger::Level::Warning) << "Invalid theme-color value from gsettings";
        return;
    }

    if (m_themeColor == color) {
        return;
    }

    m_themeColor = color;
    VW_LOG(Logger::Level::Debug) << "themeColor changed to" << color;

    Q_EMIT themeColorChanged();
}

void VoiceInputWindow::onStyleSettingsChanged(const QString &key)
{
    if (key == QLatin1String(KEY_THEME_COLOR) || key == QLatin1String("themeColor")) {
        updateThemeColorFromSettings();
    }
}

void VoiceInputWindow::updateIconSource()
{
    const QString newSource = ThemeIconHelper::buildIconDataUrl(m_iconName);

    if (m_iconSource == newSource) {
        return;
    }

    m_iconSource = newSource;
    Q_EMIT iconSourceChanged();
}

QColor VoiceInputWindow::themeColor() const
{
    return m_themeColor;
}

QString VoiceInputWindow::iconName() const
{
    return m_iconName;
}

void VoiceInputWindow::setIconName(const QString &iconName)
{
    if (m_defaultIconName == iconName && m_iconName == iconName) {
        return;
    }

    m_defaultIconName = iconName;
    stopLoadingAnimation();
    setDisplayIcon(iconName);
    Q_EMIT iconNameChanged();
}

QString VoiceInputWindow::iconSource() const
{
    return m_iconSource;
}

int VoiceInputWindow::windowSize() const
{
    return m_windowSize;
}

void VoiceInputWindow::setWindowSize(int size)
{
    if (size <= 0 || m_windowSize == size) {
        return;
    }
    m_windowSize = size;
    resize(hostSize(), hostSize());
    Q_EMIT windowSizeChanged();
    if (isVisible()) {
        updatePosition();
    }
}

void VoiceInputWindow::updatePosition()
{
    QScreen *targetScreen = screen();
    if (!targetScreen) {
        targetScreen = QGuiApplication::primaryScreen();
    }
    if (!targetScreen) {
        return;
    }

    setScreen(targetScreen);

    const QRect geo = targetScreen->availableGeometry();
    const int x = geo.x() + (geo.width() - width()) / 2;
    const int y = geo.y() + geo.height() - height() - kBottomMargin;
    setPosition(x, y);
}

void VoiceInputWindow::showWindow()
{
    updatePosition();
    show();
    raise();
    requestUpdate();
    onVolumeSampleTimeout();
    m_volumeSampleTimer->start();
}

void VoiceInputWindow::hideWindow()
{
    m_loadingDelayTimer->stop();
    stopLoadingAnimation();
    m_volumeSampleTimer->stop();
    m_periodMaxVolumeLevel = 0.0;
    hide();
}

void VoiceInputWindow::initWindowHelper()
{
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setWindowState(UkuiWindowHelper::State::KeepAbove);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
}
