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

#include <QFileInfo>
#include <QFileIconProvider>

#include "previewpage/other.h"

#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QPaintEvent>
#include <QResizeEvent>

OtherPreviewPage::OtherPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    qDebug() << "init OtherPreviewPage";

    m_icon = new IconButton(this);
    m_icon->setIconSize(QSize(96, 96));
    base_layout->addWidget(m_icon);

    m_form = new QFormLayout(this);
    m_display_name_label = new QLabel(this);
    m_display_name_label->setWordWrap(true);
    m_form->addRow(tr("File Name:"), m_display_name_label);
    m_type_label = new QLabel(this);
    m_form->addRow(tr("File Type:"), m_type_label);
    m_time_access_label = new QLabel(this);
    m_form->addRow(tr("Time Access:"), m_time_access_label);
    m_time_modified_label = new QLabel(this);
    m_form->addRow(tr("Time Modified:"), m_time_modified_label);

    m_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_form->setFormAlignment(Qt::AlignHCenter);
    m_form->setLabelAlignment(Qt::AlignRight);

    QWidget *form = new QWidget(this);
    form->setLayout(m_form);
    base_layout->addWidget(form);
}

void OtherPreviewPage::updateInfo(PreviewHubFileInfo *info)
{
    qDebug() << "OtherPreviewPage::updateInfo";
    QFileIconProvider prov;
    const QIcon thumb = prov.icon(QFileInfo(info->filePath()));
    const QIcon fallback = QIcon::fromTheme(info->iconName(), QIcon::fromTheme("text-x-generic"));
    const QPixmap pm = thumb.pixmap(96, 96);
    m_icon->setIcon(pm.isNull() ? fallback : thumb);
    m_display_name_label->setText(info->displayName());
    m_type_label->setText(info->fileType());

    QLocale locale;
    auto access = QDateTime::fromMSecsSinceEpoch(info->accessTime()*1000);
    auto modify = QDateTime::fromMSecsSinceEpoch(info->modifiedTime()*1000);

    m_time_access_label->setText(access.toString(Qt::SystemLocaleShortDate));
    m_time_modified_label->setText(modify.toString(Qt::SystemLocaleShortDate));
    qDebug() << "OtherPreviewPage::updateInfo ok!";
}

void OtherPreviewPage::cancel() {

}

void OtherPreviewPage::resizeEvent(QResizeEvent* event) {
    int width = event->size().width() - 50;
    width = qMax(width, 96);
    width = qMin(width, 256);
    m_icon->setIconSize(QSize(width, width * 2/3));
}
