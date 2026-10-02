/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "previewwidget.h"
#include <QProcess>

PreviewWidget::PreviewWidget(QWidget *parent):QWidget(parent)
{

}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(0, 0, 0, 0));
    p.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 10, 10);
}

void PreviewWidget::mousePressEvent(QMouseEvent *e)
{
    if (parentWidget() && !parentWidget()->isEnabled()) {
        e->ignore();
        return ;
    }
    if(e->button() == Qt::LeftButton) {
        previewScreensaver();
    }
}

void PreviewWidget::previewScreensaver()
{
    static QProcess *viewProcess = new QProcess();
    viewProcess->start("ukui-screensaver-command --screensaver");
}
