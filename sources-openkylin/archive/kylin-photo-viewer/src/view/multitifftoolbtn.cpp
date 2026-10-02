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
#include "multitifftoolbtn.h"
#include <QDebug>
MultiTiffToolBtn::MultiTiffToolBtn(QString btnType, QWidget *parent) : QPushButton(parent)
{
    this->setObjectName("Kylin-photo-viewer_QPushButton_multiTiffToolBtn");
    this->setAccessibleName("Kylin-photo-viewer_QPushButton_multiTiffToolBtn");
    this->setAccessibleDescription("multipage tiff button");

    this->setFixedSize(36, 36);
    this->setBackgroundRole(QPalette::Base);
    m_btnType = btnType;
    this->setIconSize(QSize(36, 36));
    this->setMouseTracking(true);
}

void MultiTiffToolBtn::themeStyle(QString theme)
{
    m_theme = theme;
}

void MultiTiffToolBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOption opt;
    opt.init(this);
    QPixmap pixmap;
    if (m_theme == "ukui-dark" || m_theme == "ukui-black") {
        if (opt.state & QStyle::State_MouseOver) {
            if (this->isDown()) {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_click_D.png");
            } else {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_hover_D.png");
            }
        } else {
            pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_normal_D.png");
        }
    } else {
        if (opt.state & QStyle::State_MouseOver) {
            if (this->isDown()) {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_click_W.png");
            } else {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_hover_W.png");
            }
        } else {
            pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_normal_W.png");
        }
    }
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(opt.rect, 6, 6);
    painter.drawPixmap(opt.rect.adjusted(0, 0, 0, 0), pixmap);
}
