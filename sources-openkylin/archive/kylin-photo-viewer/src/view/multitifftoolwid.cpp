/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "multitifftoolwid.h"
#include <QDebug>
MultiTiffToolWid::MultiTiffToolWid(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_multiTiffToolWid");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_multiTiffToolWid");
    this->setAccessibleDescription("multipage tiff widget");

    this->setFixedSize(138, 48);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    m_backgroundColor = QColor(255, 255, 255);
}

void MultiTiffToolWid::themeStyle(QString theme)
{
    m_theme = theme;
}
void MultiTiffToolWid::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
                                                   //    //设置背景颜色
                                                   //    QColor color = this->palette().color(QPalette::Base);
                                                   //    QPalette pal(this->palette());
                                                   //    pal.setColor(QPalette::Background, QColor(color));
                                                   //    this->setPalette(pal);
                                                   //    QBrush brush = QBrush(color);
                                                   //    painter.setBrush(brush);
                                                   //    painter.setOpacity(0.6);
    //    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    QPixmap pixmap;
    if (m_theme == "ukui-dark" || m_theme == "ukui-black") {
        pixmap = QPixmap(":/res/res/tiff/tiffTool_D.png");
    } else {
        pixmap = QPixmap(":/res/res/tiff/tiffTool_W.png");
    }
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(opt.rect, 8, 8);
    painter.drawPixmap(opt.rect.adjusted(0, 0, 0, 0), pixmap);
}
