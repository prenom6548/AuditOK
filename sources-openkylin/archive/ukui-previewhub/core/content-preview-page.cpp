/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
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
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#include "content-preview-page.h"
#include "preview-page-manger.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QUrl>

#include <QDebug>

ContentPreviewPage::ContentPreviewPage(QWidget *parent, bool simplePreview)
    : QStackedWidget(parent),
      m_simple_preview(simplePreview)
{
    qDebug() << "Init ContentPreviewPage";

    setContentsMargins(10, 0, 10, 0);
    auto label = new QLabel(tr("Select the file you want to preview..."), this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    m_empty_tab_widget = label;
    addWidget(m_empty_tab_widget);
    setCurrentWidget(m_empty_tab_widget);

    m_page_manger = new PreviewPageManger(this, m_simple_preview);
}

ContentPreviewPage::~ContentPreviewPage()
{
    cancel();
    delete m_page_manger;
}


void ContentPreviewPage::prepare(const QString &uri) {
    qDebug() << "prepare: " << uri;
    m_current_uri = uri;

    m_file_info = PreviewFileType::fromUri(uri);
    m_support = uri.contains("file:///");

    if (m_watcher) {
        disconnect(m_watcher, nullptr, this, nullptr);
        delete m_watcher;
        m_watcher = nullptr;
    }
    if (uri.startsWith(QStringLiteral("file://"))) {
        m_watcher = new PreviewHubFileWatcher(uri, this);
        connect(m_watcher, &PreviewHubFileWatcher::locationChanged, this,
                [this](const QString &, const QString &newUri) {
                    this->prepare(newUri);
                    this->startPreview();
                }, Qt::QueuedConnection);
        m_watcher->startMonitor();
    }
}

void ContentPreviewPage::startPreview() {
    qDebug() << "startPreview: " << m_current_uri;
    m_file_info->sync();

    if (m_support) {
        m_page_manger->startPreview(m_file_info.get());
    } else {
        QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
        label->setText(tr("Cann not preview this file."));
    }
}

void ContentPreviewPage::cancel() {
    qDebug() << "in cancel ";
    if (m_watcher) {
        disconnect(m_watcher, nullptr, this, nullptr);
        delete m_watcher;
        m_watcher = nullptr;
    }
    m_page_manger->cancel();
    setCurrentWidget(m_empty_tab_widget);
    QLabel *label = qobject_cast<QLabel*>(m_empty_tab_widget);
    label->setText(tr("Select the file you want to preview..."));
    qDebug() << "out cancel";
}

void ContentPreviewPage::hideEvent(QHideEvent *event) {
    m_page_manger->cancel();
    QStackedWidget::hideEvent(event);
}

void ContentPreviewPage::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(this->rect(), this->palette().base());
    QWidget::paintEvent(e);
}
