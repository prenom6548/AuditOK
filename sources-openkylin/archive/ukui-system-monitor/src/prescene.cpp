/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "prescene.h"

#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QApplication>
#include <QDebug>
#include <QStringList>

#define THEME_QT_SCHEMA  "org.ukui.style"
#define WINDOW_RAIDUS    "window-radius"

PreScene::PreScene(QSize size, QWidget *parent) : QWidget(parent), m_size(size)
{
    const QByteArray idd(THEME_QT_SCHEMA);
    qtSettings = new QGSettings(idd,  QByteArray(), this);

    this->setFixedSize(size);
    this->setObjectName("prescene");

    m_vlayout = new QVBoxLayout;
    m_logoLayout = new QHBoxLayout;

    QIcon titleIcon = QIcon::fromTheme("ukui-system-monitor");

    logoLabel = new QLabel(this);
    logoLabel->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(96, 96))));
    m_logoLayout->setAlignment(Qt::AlignCenter);
    m_logoLayout->addStretch();
    m_logoLayout->addWidget(logoLabel);
    m_logoLayout->addStretch();

    m_vlayout->setSpacing(0);
    m_vlayout->setContentsMargins(0, 0, 0, 48);
    m_vlayout->addStretch();
    m_vlayout->addLayout(m_logoLayout);
    m_vlayout->addStretch();

    this->setLayout(m_vlayout);
}

const QPixmap PreScene::loadSvg(const QString &fileName)
{
    int size = 128;
    const auto ratio = qApp->devicePixelRatio();
    size *= ratio;
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);

    QImage img = pixmap.toImage();
    return QPixmap::fromImage(img);
}

PreScene::~PreScene() {

}

void PreScene::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;

    int border = 0;
    if (qtSettings->keys().contains("windowRadius")) {
        QString radius = qtSettings->get(WINDOW_RAIDUS).toString();
        QStringList radiuslist =  radius.split(",");
        if (radiuslist.count() > 2) {
            border = QString(radiuslist.at(0)).toInt();
        }
        qDebug() << Q_FUNC_INFO << "init mCornerWidget combox current" << radius << border;
    }

    rectPath.addRoundedRect(this->rect(), border, border);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();

}
