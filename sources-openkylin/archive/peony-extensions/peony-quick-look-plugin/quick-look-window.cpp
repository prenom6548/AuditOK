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

#include "quick-look-window.h"
#include "quick-look-info-panel.h"
#include "quick-look-open-button.h"

#include <file-info.h>
#include <file-utils.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMimeDatabase>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include <cstring>

#include <ukui_previewhub_api.h>

static bool isOfficeMime(const QString &mime)
{
    for (int i = 0; std::strcmp(Peony::office_mime_types[i], "end") != 0; ++i) {
        if (mime == QLatin1String(Peony::office_mime_types[i]))
            return true;
    }
    return false;
}

bool QuickLookWindow::isHubMime(const QString &mime)
{
    if (mime.isEmpty())
        return false;
    if (mime.startsWith(QLatin1String("text/")) ||
        mime.startsWith(QLatin1String("application/xml")) ||
        mime.startsWith(QLatin1String("application/json")) ||
        mime.startsWith(QLatin1String("application/javascript")) ||
        mime.startsWith(QLatin1String("application/vnd.nokia.qt.qmakeprofile")))
        return true;
    if (mime.startsWith(QLatin1String("image/")))
        return true;
    if (mime.startsWith(QLatin1String("audio/")))
        return true;
    if (mime.startsWith(QLatin1String("video/")))
        return true;
    if (mime == QLatin1String("application/pdf"))
        return true;
    if (mime.startsWith(QLatin1String("application/vnd.oasis.opendocument.")))
        return true;
    if (mime.startsWith(QLatin1String("application/vnd.openxmlformats-officedocument.")))
        return true;
    return isOfficeMime(mime);
}

QString QuickLookWindow::resolveMime(const QString &uri)
{
    auto info = Peony::FileInfo::fromUri(uri);
    if (info) {
        const QString fromInfo = info->mimeType();
        if (!fromInfo.isEmpty())
            return fromInfo;
    }
    const QUrl u = QUrl(uri);
    if (u.isLocalFile()) {
        const QString path = u.toLocalFile();
        if (!path.isEmpty())
            return QMimeDatabase().mimeTypeForFile(path, QMimeDatabase::MatchDefault).name();
    }
    return QString();
}

QuickLookWindow::QuickLookWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setMinimumSize(480, 360);
    resize(960, 640);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_contentStack = new QStackedWidget(this);

    m_hubPage = new QWidget(this);
    auto *hubLayout = new QVBoxLayout(m_hubPage);
    hubLayout->setContentsMargins(0, 0, 0, 0);
    hubLayout->setSpacing(0);

    m_hubLoadingLabel = new QLabel(tr("Loading preview..."), m_hubPage);
    m_hubLoadingLabel->setAlignment(Qt::AlignCenter);
    m_hubLoadingLabel->setWordWrap(true);
    hubLayout->addWidget(m_hubLoadingLabel, 1);

    m_infoPanel = new QuickLookInfoPanel(this);

    m_contentStack->addWidget(m_hubPage);
    m_contentStack->addWidget(m_infoPanel);
    root->addWidget(m_contentStack, 1);

    m_footer = new QWidget(this);
    auto *footerLayout = new QVBoxLayout(m_footer);
    footerLayout->setContentsMargins(24, 12, 24, 24);
    m_openButton = new QuickLookOpenButton(m_footer);
    footerLayout->addWidget(m_openButton);
    root->addWidget(m_footer);

    setLayout(root);
}

QuickLookWindow::~QuickLookWindow()
{
    closeQuickLook();
}

void QuickLookWindow::ensureHubWidget()
{
    if (m_hubWidget)
        return;

    auto *hubLayout = qobject_cast<QVBoxLayout *>(m_hubPage->layout());
    if (!hubLayout)
        return;

    m_hubWidget = UkuiPreviewHub::createPreviewWidget(
        m_hubPage, UkuiPreviewHub::PreviewMode::Complex);
    if (!m_hubWidget)
        return;
    m_hubWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hubLayout->insertWidget(0, m_hubWidget, 1);
    m_hubWidget->hide();
}

void QuickLookWindow::scheduleHubPreview()
{
    ++m_hubPreviewGeneration;
    const int generation = m_hubPreviewGeneration;
    QTimer::singleShot(0, this, [this, generation]() {
        if (generation != m_hubPreviewGeneration || !m_useHub)
            return;
        ensureHubWidget();
        if (!m_hubWidget || m_uri.isEmpty())
            return;

        UkuiPreviewHub::cancelPreview(m_hubWidget);
        UkuiPreviewHub::preparePreview(m_hubWidget, m_uri);
        if (m_hubLoadingLabel)
            m_hubLoadingLabel->hide();
        m_hubWidget->show();
        UkuiPreviewHub::startPreview(m_hubWidget);
    });
}

void QuickLookWindow::updateModeAndContent(const QString &uri)
{
    ++m_hubPreviewGeneration;
    m_uri = uri;

    auto info = Peony::FileInfo::fromUri(uri);
    m_isDir = info && info->isDir();
    const bool canRead = info && info->canRead() && info->isExistTargetOfSymlink();
    m_useHub = !m_isDir && canRead && isHubMime(resolveMime(uri));

    if (m_hubWidget)
        UkuiPreviewHub::cancelPreview(m_hubWidget);
    m_infoPanel->clearPanel();
    m_openButton->clear();

    QString displayName = info ? info->displayName() : QString();
    if (displayName.isEmpty())
        displayName = Peony::FileUtils::getUriBaseName(uri);
    setWindowTitle(displayName);
    if (info)
        setWindowIcon(QIcon::fromTheme(info->iconName()));

    if (m_isDir) {
        m_contentStack->setCurrentWidget(m_infoPanel);
        m_infoPanel->setFolderInfo(uri);
        m_footer->setVisible(false);
        return;
    }

    m_footer->setVisible(true);
    if (m_useHub) {
        m_contentStack->setCurrentWidget(m_hubPage);
        if (m_hubLoadingLabel)
            m_hubLoadingLabel->show();
        if (m_hubWidget)
            m_hubWidget->hide();
        m_openButton->setUri(uri);
    } else {
        m_contentStack->setCurrentWidget(m_infoPanel);
        m_infoPanel->setFileInfo(uri);
        m_openButton->setUri(uri);
    }
}

void QuickLookWindow::prepare(const QString &uri)
{
    updateModeAndContent(uri);
}

void QuickLookWindow::show(QWidget *parent)
{
    Q_UNUSED(parent)
    QDialog::show();
    raise();
    activateWindow();

    if (m_useHub)
        scheduleHubPreview();
}

void QuickLookWindow::closeQuickLook()
{
    ++m_hubPreviewGeneration;
    if (m_hubWidget)
        UkuiPreviewHub::cancelPreview(m_hubWidget);
    m_infoPanel->clearPanel();
    m_openButton->clear();
    if (m_hubLoadingLabel)
        m_hubLoadingLabel->show();
    if (m_hubWidget)
        m_hubWidget->hide();
    if (isVisible())
        hide();
}

bool QuickLookWindow::isVisible() const
{
    return QDialog::isVisible();
}

QObject *QuickLookWindow::asObject()
{
    return this;
}
