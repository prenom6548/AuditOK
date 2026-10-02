/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "quick-look-open-button.h"

#include <file-launch-manager.h>
#include <file-launch-action.h>
#include <file-lauch-dialog.h>
#include <file-utils.h>

#include <QHBoxLayout>
#include <QTimer>
#include <QToolButton>

using namespace Peony;

QuickLookOpenButton::QuickLookOpenButton(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_openWithButton = new QToolButton(this);
    m_openWithButton->setProperty("isImportant", true);
    m_openWithButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_openWithButton->setPopupMode(QToolButton::DelayedPopup);
    connect(m_openWithButton, &QToolButton::clicked, this, [this]() {
        openUri();
    });

    layout->addStretch();
    layout->addWidget(m_openWithButton);
    m_openWithButton->setVisible(false);
}

QuickLookOpenButton::~QuickLookOpenButton() = default;

void QuickLookOpenButton::clear()
{
    m_uri.clear();
    m_openWithButton->setVisible(false);
}

QString QuickLookOpenButton::resolveLaunchUri(const QString &uri) const
{
    if (uri.startsWith(QStringLiteral("filesafe:///")) ||
        uri.startsWith(QStringLiteral("recent:///"))) {
        const auto targetUri = FileUtils::getTargetUri(uri);
        if (!targetUri.isEmpty())
            return targetUri;
    }
    return uri;
}

void QuickLookOpenButton::setUri(const QString &uri)
{
    if (uri.isEmpty()) {
        clear();
        return;
    }
    m_uri = uri;
    QTimer::singleShot(0, this, [this]() {
        refreshPrimaryOpenButton();
    });
}

void QuickLookOpenButton::openUri()
{
    const QString launchUri = resolveLaunchUri(m_uri);
    if (launchUri.isEmpty())
        return;

    auto *action = FileLaunchManager::getDefaultAction(launchUri);
    if (action && !action->getAppInfoDisplayName().isEmpty()) {
        action->lauchFileAsync();
        action->deleteLater();
        return;
    }

    if (action)
        action->deleteLater();

    FileLauchDialog dialog(launchUri, this);
    dialog.exec();
    refreshPrimaryOpenButton();
}

void QuickLookOpenButton::refreshPrimaryOpenButton()
{
    if (m_uri.isEmpty()) {
        m_openWithButton->setVisible(false);
        return;
    }

    const QString launchUri = resolveLaunchUri(m_uri);
    auto *action = FileLaunchManager::getDefaultAction(launchUri);
    if (!action || action->getAppInfoDisplayName().isEmpty()) {
        if (action)
            action->deleteLater();
        m_openWithButton->setText(tr("Open"));
        m_openWithButton->setIcon(QIcon::fromTheme(QStringLiteral("document-open-symbolic")));
        m_openWithButton->setVisible(true);
        return;
    }

    m_openWithButton->setText(tr("Open with %1").arg(action->getAppInfoDisplayName()));
    m_openWithButton->setIcon(action->icon());
    m_openWithButton->setVisible(true);
    action->deleteLater();
}
