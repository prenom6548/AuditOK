/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "imagelabel.h"
#include <QPainter>
#include <QPainterPath>

ImageLabel::ImageLabel(QWidget *parent):
    QLabel(parent)
{
    m_labTime   = new QLabel;
    m_labTime->setAccessibleName("kylin-video_QLabel_m_labTime");
    QFont f("Noto Sans CJK SC Regular");
    f.setPointSize(11);
    m_labTime->setFont(f);

    m_boxLayout = new QVBoxLayout(this);
    m_boxLayout->setObjectName("kylin-video_QVBoxLayout_m_boxLayout");

    m_boxLayout->addStretch();
    m_boxLayout->addWidget(m_labTime);
    m_boxLayout->setContentsMargins(0,0,0,0);

    m_labTime->setStyleSheet("color:#ffffff;"
                             "background-color:qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(19, 19, 20, 150), stop:1 rgba(255, 255, 255, 0));"
                             "border-bottom-left-radius:5px;"
                             "border-bottom-right-radius:5px;");
    m_labTime->setAlignment(Qt::AlignCenter);
}

ImageLabel::~ImageLabel()
{
    delete m_labTime;
    delete m_boxLayout;
}

void ImageLabel::setTime(QString time)
{
    m_labTime->setText(time);
}

void ImageLabel::paintEvent(QPaintEvent *e)
{
    if(pixmap()){
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addRoundedRect(QRectF(0, 0, width(), height()), 5, 5);
        painter.setClipPath(path);
        if(width() < pixmap()->width() || height() < pixmap()->height())
            return;
        painter.setBackground(QBrush(QColor(0, 0, 0)));
        painter.drawPixmap((width() - pixmap()->width())/2,
                           (height() - pixmap()->height())/2,
                           pixmap()->width(),
                           pixmap()->height(),
                           *pixmap());
        return;
    }
    return QLabel::paintEvent(e);
}
