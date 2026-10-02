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

#include "microphoneMuteDialog.h"
#include "themeIconProvider.h"

#include <QEvent>
#include <QGuiApplication>
#include "logger.h"
#include <QQmlContext>
#include <QQmlError>
#include <QProcess>
#include <QScreen>

MicrophoneMuteDialog::MicrophoneMuteDialog(QWindow *parent)
    : QQuickView(parent)
{
    setFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint);
    setTitle(tr("Microphone Muted"));

    setResizeMode(QQuickView::SizeRootObjectToView);
    resize(kDialogWidth, kDialogHeight);

    m_infoIconSource = ThemeIconHelper::buildIconDataUrl(QStringLiteral("dialog-info"), false);
    rootContext()->setContextProperty(QStringLiteral("muteDialog"), this);
    setSource(QUrl(QStringLiteral("qrc:/microphoneMuteDialog.qml")));

    if (status() == QQuickView::Error) {
        for (const QQmlError &error : errors()) {
            VW_LOG(Logger::Level::Error) << "Failed to load microphoneMuteDialog.qml:"
                                         << error.toString();
        }
    }

    initWindowHelper();
}

QString MicrophoneMuteDialog::infoIconSource() const
{
    return m_infoIconSource;
}

void MicrophoneMuteDialog::initWindowHelper()
{
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::Dialog);
    m_windowHelper->setWindowState(UkuiWindowHelper::State::Closeable | UkuiWindowHelper::State::Focusable
                                 | UkuiWindowHelper::State::Activatable | UkuiWindowHelper::State::Movable);
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
}

void MicrophoneMuteDialog::updateCenterPosition()
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
    const int y = geo.y() + (geo.height() - height()) / 2;
    setPosition(x, y);
}

bool MicrophoneMuteDialog::event(QEvent *event)
{
    if (event->type() == QEvent::Close) {
        hide();
        event->accept();
        return true;
    }
    return QQuickView::event(event);
}

void MicrophoneMuteDialog::showCentered()
{
    updateCenterPosition();
    show();
    raise();
    requestActivate();
}

void MicrophoneMuteDialog::onCancelClicked()
{
    hide();
}

void MicrophoneMuteDialog::onCloseClicked()
{
    hide();
}

void MicrophoneMuteDialog::openSoundSettings()
{
    const bool started = QProcess::startDetached(
        QStringLiteral("/usr/bin/ukui-control-center"),
        {QStringLiteral("-m"), QStringLiteral("Audio")});
    if (!started) {
        VW_LOG(Logger::Level::Error) << "Failed to launch ukui-control-center for sound settings";
    }
}

void MicrophoneMuteDialog::onSettingsClicked()
{
    openSoundSettings();
    hide();
}
