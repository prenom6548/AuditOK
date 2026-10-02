/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "playiconlabel.h"

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

PlayIconLabel::PlayIconLabel(const QString& iconName, const QString& currentStyle, QWidget *parent)
    : QLabel(parent)
    , m_iconName(iconName)
    , m_currentStyle(currentStyle)
    , m_checked(false)
{
    setAlignment(Qt::AlignVCenter);
    // 上边距设为 1 是为了让图标居中，可能是 QT 的 bug
    setContentsMargins(0, 1, 0, 0);

    resetPixmap();
}

void PlayIconLabel::resetPixmap()
{
    QPixmap iconPixmap = QIcon::fromTheme(m_iconName).pixmap(16,16);
    QPixmap pixmap = m_checked ? fillPixmapByColor(iconPixmap, Qt::white)
                               : processPixmapAccordingStyle(iconPixmap, m_currentStyle);
    setPixmap(pixmap);
}

void PlayIconLabel::onToggled(bool checked)
{
    m_checked = checked;
    resetPixmap();
}

void PlayIconLabel::onStyleChanged(QString newStyle)
{
    m_currentStyle = newStyle;
    resetPixmap();
}

void PlayIconLabel::onIconThemeChanged()
{
    resetPixmap();
}

QPixmap PlayIconLabel::processPixmapAccordingStyle(const QPixmap &pixmap, const QString& styleName) const
{
    if (styleName.isEmpty())
        return QPixmap();

    Qt::GlobalColor pixmapColor = Qt::color0;
    if (styleName == "ukui-light" || styleName == "ukui-default") {
        pixmapColor = Qt::black;
    }
    else if (styleName == "ukui-dark") {
        pixmapColor = Qt::white;
    }
    else {
        qWarning() << "Can't process icon according style" << styleName;
    }

    return fillPixmapByColor(pixmap, pixmapColor);
}

QPixmap PlayIconLabel::fillPixmapByColor(const QPixmap& pixmap, Qt::GlobalColor color) const
{
    QPixmap target = pixmap;
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(target.rect(), color);

    return target;
}
